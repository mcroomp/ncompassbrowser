#include "cross_p.h"

#include "readhtml.h"

#include <atomic>
#include <fstream>
#include <iostream>
#include <thread>
#include <string>
#include <vector>

#include <wininet.h>


class CParserHarness : public CParseHTML
	{
public:
	CParserHarness(LPCSTR url)
		: CParseHTML(url, "text/html", FALSE)
		{
		}

	LOAD_STATE Feed(LPCBYTE data, INT32 length)
		{
		return OnReadData(data, length);
		}

	LOAD_STATE Finish()
		{
		return OnEndOfFile();
		}
	};


class CInternetHandle
	{
public:
	CInternetHandle(HINTERNET handle = NULL) : handle_(handle)
		{
		}

	~CInternetHandle()
		{
		if (handle_)
			InternetCloseHandle(handle_);
		}

	operator HINTERNET() const
		{
		return handle_;
		}

private:
	HINTERNET handle_;
	};


bool FetchUrl(const char *url, DWORD timeout_ms, std::vector<BYTE>& body)
	{
	CInternetHandle session(InternetOpenA("NCompass Version 1A2",
		INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0));
	if (!session)
		return false;

	DWORD retries = 0;
	if (!InternetSetOption(session, INTERNET_OPTION_CONNECT_TIMEOUT,
			&timeout_ms, sizeof(timeout_ms)) ||
		!InternetSetOption(session, INTERNET_OPTION_RECEIVE_TIMEOUT,
			&timeout_ms, sizeof(timeout_ms)) ||
		!InternetSetOption(session, INTERNET_OPTION_SEND_TIMEOUT,
			&timeout_ms, sizeof(timeout_ms)) ||
		!InternetSetOption(session, INTERNET_OPTION_CONNECT_RETRIES,
			&retries, sizeof(retries)))
		return false;

	const char headers[] = "Accept-Encoding: identity\r\n";
	CInternetHandle request(InternetOpenUrlA(session, url, headers,
		static_cast<DWORD>(strlen(headers)),
		INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE |
			INTERNET_FLAG_PRAGMA_NOCACHE, 0));
	if (!request)
		return false;

	DWORD status = 0;
	DWORD status_size = sizeof(status);
	DWORD index = 0;
	if (!HttpQueryInfoA(request,
			HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
			&status, &status_size, &index) ||
		status < 200 || status > 299)
		return false;

	BYTE buffer[65536];
	for (;;)
		{
		DWORD amount = 0;
		if (!InternetReadFile(request, buffer, sizeof(buffer), &amount))
			return false;
		if (!amount)
			break;
		if (body.size() + amount > 16 * 1024 * 1024)
			return false;
		body.insert(body.end(), buffer, buffer + amount);
		}
	return true;
	}


UINT64 HashBytes(UINT64 hash, const void *data, size_t length)
	{
	const BYTE *bytes = static_cast<const BYTE *>(data);
	for (size_t i = 0; i < length; i++)
		{
		hash ^= bytes[i];
		hash *= 1099511628211ULL;
		}
	return hash;
	}


struct ParseFingerprint
	{
	LOAD_STATE state;
	UINT64 hash;
	size_t tags;

	bool operator==(const ParseFingerprint& other) const
		{
		return state == other.state && hash == other.hash &&
			tags == other.tags;
		}
	};


ParseFingerprint ParseBytes(const std::vector<BYTE>& body, LPCSTR url,
	size_t chunk_size)
	{
	CParserHarness parser(url);
	LOAD_STATE state = LOAD_STATE_LOADING;
	for (size_t offset = 0;
		offset < body.size() && state == LOAD_STATE_LOADING;
		offset += chunk_size)
		{
		size_t amount = min(chunk_size, body.size() - offset);
		state = parser.Feed(body.data() + offset,
			static_cast<INT32>(amount));
		}
	if (state == LOAD_STATE_LOADING)
		state = parser.Finish();

	parser.DEBUG_LOCK();
	CString title = parser.GetTitle();
	CBigString plain_text = parser.GetPlainText();
	CString plain;
	plain_text.GetString(plain, 0, plain_text.GetLength());
	UINT64 hash = 1469598103934665603ULL;
	hash = HashBytes(hash, static_cast<LPCSTR>(title), title.GetLength());
	hash = HashBytes(hash, static_cast<LPCSTR>(plain), plain.GetLength());
	INT32 colors[] = {
		parser.GetBackgroundColor(),
		parser.GetTextColor(),
		parser.GetHotlinkColor(),
		parser.GetOldHotlinkColor()
		};
	hash = HashBytes(hash, colors, sizeof(colors));
	CString background_picture = parser.GetBackgroundPicture();
	hash = HashBytes(hash, static_cast<LPCSTR>(background_picture),
		background_picture.GetLength());
	size_t tags = 0;
	POSITION walk = parser.GetFirstTagPos();
	while(walk)
		{
		const CTag *tag = parser.GetNextTag(walk);
		if ((tag->m_type & ~TAG_END) != TAG_TEXT)
			{
			hash = HashBytes(hash, &tag->m_type, sizeof(tag->m_type));
			tags++;
			}
		}
	parser.Unlock();
	return { state, hash, tags };
	}


int StressUrl(const char *url, int iterations, int parallel,
	DWORD timeout_ms)
	{
	std::vector<BYTE> body;
	if (!FetchUrl(url, timeout_ms, body))
		{
		std::cerr << "URL fetch failed with error " << GetLastError()
			<< "\n";
		return 1;
		}

	static const size_t chunk_sizes[] = { 1, 2, 7, 31, 4096 };
	std::vector<ParseFingerprint> results(iterations);
	std::atomic<int> next(0);
	std::vector<std::thread> workers;
	for (int worker = 0; worker < parallel; worker++)
		{
		workers.emplace_back([&]()
			{
			for (;;)
				{
				int iteration = next.fetch_add(1);
				if (iteration >= iterations)
					break;
				results[iteration] = ParseBytes(body, url,
					chunk_sizes[iteration % _countof(chunk_sizes)]);
				}
			});
		}
	for (std::thread& worker : workers)
		worker.join();

	for (int iteration = 0; iteration < iterations; iteration++)
		{
		if (results[iteration].state != LOAD_STATE_COMPLETE ||
			!(results[iteration] == results[0]))
			{
			std::cerr << "Parser divergence at iteration "
				<< iteration << "\n";
			return 1;
			}
		}

	std::cout << "PASS: " << body.size() << " bytes, " << iterations
		<< " parses, " << parallel << " workers, " << results[0].tags
		<< " tags, hash " << results[0].hash << "\n";
	return 0;
	}


void WriteJsonString(std::ostream& output, LPCSTR text, int length)
	{
	static const char hex[] = "0123456789ABCDEF";
	output << '"';
	for (int i = 0; i < length; i++)
		{
		unsigned char c = static_cast<unsigned char>(text[i]);
		switch(c)
			{
			case '"':
				output << "\\\"";
				break;
			case '\\':
				output << "\\\\";
				break;
			case '\b':
				output << "\\b";
				break;
			case '\f':
				output << "\\f";
				break;
			case '\n':
				output << "\\n";
				break;
			case '\r':
				output << "\\r";
				break;
			case '\t':
				output << "\\t";
				break;
			default:
				if (c < 0x20 || c >= 0x80)
					{
					output << "\\u00" << hex[c >> 4] << hex[c & 0x0F];
					}
				else
					{
					output << static_cast<char>(c);
					}
				break;
			}
		}
	output << '"';
	}


void WriteJsonString(std::ostream& output, const CString& text)
	{
	WriteJsonString(output, text, text.GetLength());
	}


LPCSTR GetTagName(int type)
	{
	switch(type & ~TAG_END)
		{
		case TAG_TEXT: return "text";
		case TAG_ANCHOR_NAME: return "anchor-name";
		case TAG_ANCHOR_HREF: return "anchor-href";
		case TAG_PARAGRAPH: return "paragraph";
		case TAG_HEADING: return "heading";
		case TAG_HORZRULE: return "horizontal-rule";
		case TAG_IMAGE: return "image";
		case TAG_NEWLINE: return "newline";
		case TAG_NEWLINE_CLEAR: return "newline-clear";
		case TAG_NEWLINE_CLEAR_EN: return "newline-clear-en";
		case TAG_NEWLINE_CLEAR_PIXEL: return "newline-clear-pixel";
		case TAG_NEWLINE_HARD_BREAK: return "hard-break";
		case TAG_CONDITIONAL_BREAK: return "conditional-break";
		case TAG_QUICKTIME: return "quicktime";
		case TAG_TABLE: return "table";
		case TAG_TABLECAPTION: return "table-caption";
		case TAG_TABLEROW: return "table-row";
		case TAG_TABLECELL: return "table-cell";
		case TAG_BULLET: return "bullet";
		case TAG_CENTER: return "center";
		default: return "unknown";
		}
	}


void WriteTag(std::ostream& output, const CTag *tag,
	const CBigString& plain_text)
	{
	int base_type = tag->m_type & ~TAG_END;
	output << "{\"type\":";
	WriteJsonString(output, GetTagName(tag->m_type),
		static_cast<int>(strlen(GetTagName(tag->m_type))));
	output << ",\"end\":" << ((tag->m_type & TAG_END) ? "true" : "false");

	if (base_type == TAG_TEXT)
		{
		const CTagText *text = static_cast<const CTagText *>(tag);
		CString value;
		plain_text.GetString(value, text->m_startpos, text->m_textlen);
		output << ",\"offset\":" << text->m_startpos
			<< ",\"length\":" << text->m_textlen
			<< ",\"fontFlags\":" << text->m_font_flags
			<< ",\"text\":";
		WriteJsonString(output, value);
		}
	else if ((base_type == TAG_ANCHOR_NAME ||
		base_type == TAG_ANCHOR_HREF) && !(tag->m_type & TAG_END))
		{
		const CTagCString *value = static_cast<const CTagCString *>(tag);
		output << ",\"value\":";
		WriteJsonString(output, value->m_string);
		}
	else if (base_type == TAG_HEADING ||
		base_type == TAG_HORZRULE ||
		base_type == TAG_NEWLINE_CLEAR ||
		base_type == TAG_NEWLINE_CLEAR_EN ||
		base_type == TAG_NEWLINE_CLEAR_PIXEL)
		{
		const CTagINT32 *value = static_cast<const CTagINT32 *>(tag);
		output << ",\"value\":" << value->m_int32;
		}
	else if (base_type == TAG_IMAGE)
		{
		const CTagImage *image = static_cast<const CTagImage *>(tag);
		output << ",\"url\":";
		WriteJsonString(output, image->m_url);
		output << ",\"width\":" << image->m_width
			<< ",\"height\":" << image->m_height
			<< ",\"hspace\":" << image->m_hspace
			<< ",\"vspace\":" << image->m_vspace
			<< ",\"border\":" << image->m_border
			<< ",\"align\":" << image->m_align
			<< ",\"mapped\":" << (image->m_is_mapped ? "true" : "false");
		}
	else if (base_type == TAG_BULLET)
		{
		const CTagBullet *bullet = static_cast<const CTagBullet *>(tag);
		output << ",\"bulletType\":" << bullet->m_bullet_type
			<< ",\"number\":" << bullet->m_bullet_number;
		}

	output << '}';
	}


int main(int argc, char *argv[])
	{
	if (argc >= 3 && argc <= 6 && strcmp(argv[1], "--stress-url") == 0)
		{
		int iterations = argc >= 4 ? atoi(argv[3]) : 20;
		int parallel = argc >= 5 ? atoi(argv[4]) : 4;
		DWORD timeout_ms = argc >= 6 ?
			static_cast<DWORD>(strtoul(argv[5], NULL, 10)) : 1000;
		if (iterations <= 0 || parallel <= 0 || !timeout_ms)
			{
			std::cerr << "iterations, parallel, and timeout-ms must be positive\n";
			return 2;
			}
		if (!AfxWinInit(GetModuleHandle(NULL), NULL, GetCommandLineA(), 0))
			{
			std::cerr << "Could not initialize MFC\n";
			return 2;
			}
		return StressUrl(argv[2], iterations, parallel, timeout_ms);
		}

	if (argc < 2 || argc > 4)
		{
		std::cerr << "Usage: ncompass-parse <html-file> "
			"[chunk-size] [base-url]\n";
		return 2;
		}

	int chunk_size = 4096;
	if (argc >= 3)
		{
		char *end = NULL;
		long value = strtol(argv[2], &end, 10);
		if (!argv[2][0] || *end || value <= 0 || value > INT_MAX)
			{
			std::cerr << "chunk-size must be a positive integer\n";
			return 2;
			}
		chunk_size = static_cast<int>(value);
		}

	std::ifstream input(argv[1], std::ios::binary);
	if (!input)
		{
		std::cerr << "Could not open " << argv[1] << "\n";
		return 2;
		}

	if (!AfxWinInit(GetModuleHandle(NULL), NULL, GetCommandLineA(), 0))
		{
		std::cerr << "Could not initialize MFC\n";
		return 2;
		}

	LPCSTR base_url = argc >= 4 ? argv[3] :
		"http://parser.test/input.html";
	CParserHarness parser(base_url);
	std::vector<BYTE> buffer(chunk_size);
	LOAD_STATE state = LOAD_STATE_LOADING;

	while(input && state == LOAD_STATE_LOADING)
		{
		input.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
		std::streamsize amount = input.gcount();
		if (amount > 0)
			state = parser.Feed(buffer.data(), static_cast<INT32>(amount));
		}

	if (state == LOAD_STATE_LOADING)
		state = parser.Finish();

	parser.DEBUG_LOCK();
	CString title = parser.GetTitle();
	CBigString plain_text = parser.GetPlainText();
	CString plain;
	plain_text.GetString(plain, 0, plain_text.GetLength());
	INT32 background = parser.GetBackgroundColor();
	INT32 text_color = parser.GetTextColor();
	INT32 hotlink_color = parser.GetHotlinkColor();
	INT32 old_hotlink_color = parser.GetOldHotlinkColor();
	CString background_picture = parser.GetBackgroundPicture();

	std::cout << "{\n  \"loadState\":" << state << ",\n"
		<< "  \"title\":";
	WriteJsonString(std::cout, title);
	std::cout << ",\n  \"plainText\":";
	WriteJsonString(std::cout, plain);
	std::cout << ",\n  \"backgroundColor\":" << background
		<< ",\n  \"textColor\":" << text_color
		<< ",\n  \"hotlinkColor\":" << hotlink_color
		<< ",\n  \"oldHotlinkColor\":" << old_hotlink_color
		<< ",\n  \"backgroundPicture\":";
	WriteJsonString(std::cout, background_picture);
	std::cout << ",\n  \"tags\":[";

	bool first = true;
	POSITION walk = parser.GetFirstTagPos();
	while(walk)
		{
		const CTag *tag = parser.GetNextTag(walk);
		if (!first)
			std::cout << ',';
		std::cout << "\n    ";
		WriteTag(std::cout, tag, plain_text);
		first = false;
		}
	parser.Unlock();

	if (!first)
		std::cout << '\n';
	std::cout << "  ]\n}\n";
	return state == LOAD_STATE_COMPLETE ? 0 : 1;
	}

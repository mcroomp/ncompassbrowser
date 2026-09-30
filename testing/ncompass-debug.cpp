#include <windows.h>
#include <dbghelp.h>
#include <tlhelp32.h>
#include <wininet.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


const DWORD TITLE_SETTLE_MS = 100;


struct MonitorOutcome
	{
	bool timed_out;
	bool dump_written;
	bool dialog_captured;
	bool title_matched;
	bool screenshot_written;
	DWORD exit_code;
	std::wstring dump_path;
	std::wstring observed_title;
	};


struct WindowSearch
	{
	DWORD process_id;
	HWND browser_window;
	HWND assertion_window;
	HWND dialog_window;
	};


struct NavigationStep
	{
	std::wstring url;
	std::wstring expected_title;
	};


struct ChildControlSearch
	{
	int id;
	HWND window;
	};


BOOL CALLBACK FindChildControl(HWND window, LPARAM lparam)
	{
	ChildControlSearch *search =
		reinterpret_cast<ChildControlSearch *>(lparam);
	if (GetDlgCtrlID(window) == search->id)
		{
		search->window = window;
		return FALSE;
		}
	return TRUE;
	}


// Drives navigation exactly the way a user does: through the browser's own
// location bar edit control and Open Location command, both original,
// unmodified browser behavior (win32/inc/resource.h). No test-only code
// exists in the browser for this.
const int IDC_LOCATION = 1000;
const int ID_OPEN_LOCATION = 4013;

bool NavigateBrowser(HWND browser_window, const NavigationStep& step)
	{
	ChildControlSearch search = { IDC_LOCATION, NULL };
	EnumChildWindows(browser_window, FindChildControl,
		reinterpret_cast<LPARAM>(&search));
	if (!search.window)
		return false;

	DWORD_PTR result = 0;
	if (!SendMessageTimeoutW(search.window, WM_SETTEXT, 0,
		reinterpret_cast<LPARAM>(step.url.c_str()),
		SMTO_ABORTIFHUNG | SMTO_BLOCK, 200, &result))
		return false;

	return PostMessageW(browser_window, WM_COMMAND,
		MAKEWPARAM(ID_OPEN_LOCATION, 0), 0) != FALSE;
	}


bool TitleMatches(const wchar_t *title, const std::wstring& expected)
	{
	size_t start = 0;
	while (start <= expected.size())
		{
		size_t separator = expected.find(L'|', start);
		std::wstring alternative = expected.substr(start,
			separator == std::wstring::npos ? std::wstring::npos :
				separator - start);
		if (!alternative.empty() && wcsstr(title, alternative.c_str()))
			return true;
		if (separator == std::wstring::npos)
			break;
		start = separator + 1;
		}
	return false;
	}


BOOL CALLBACK FindProcessWindows(HWND window, LPARAM lparam)
	{
	WindowSearch *search = reinterpret_cast<WindowSearch *>(lparam);
	DWORD process_id = 0;
	GetWindowThreadProcessId(window, &process_id);

	if (process_id != search->process_id || !IsWindowVisible(window))
		return TRUE;

	wchar_t title[256];
	GetWindowTextW(window, title, _countof(title));
	bool is_dialog = false;
	if (wcscmp(title, L"Microsoft Visual C++ Runtime Library") == 0)
		{
		search->assertion_window = window;
		is_dialog = true;
		}
	else
		{
		wchar_t class_name[64];
		GetClassNameW(window, class_name, _countof(class_name));
		if (wcscmp(class_name, L"#32770") == 0)
			{
			search->dialog_window = window;
			is_dialog = true;
			}
		}

	if (!is_dialog && !search->browser_window &&
		GetWindow(window, GW_OWNER) == NULL)
		search->browser_window = window;

	return TRUE;
	}


std::wstring GetModulePath()
	{
	wchar_t path[MAX_PATH];
	DWORD length = GetModuleFileNameW(NULL, path, _countof(path));
	if (length == 0 || length == _countof(path))
		return std::wstring();
	return std::wstring(path, length);
	}


std::wstring GetModuleDirectory()
	{
	std::wstring path = GetModulePath();
	size_t separator = path.find_last_of(L"\\/");
	if (separator == std::wstring::npos)
		return std::wstring();
	return path.substr(0, separator);
	}


std::wstring GetDebuggerToolPath(const wchar_t *name)
	{
	wchar_t program_files[MAX_PATH];
	DWORD length = GetEnvironmentVariableW(L"ProgramFiles(x86)",
		program_files, _countof(program_files));
	if (!length || length >= _countof(program_files))
		return std::wstring();

	std::wstring path(program_files);
	path += L"\\Windows Kits\\10\\Debuggers\\x86\\";
	path += name;
	if (GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES)
		return std::wstring();
	return path;
	}


std::wstring QuoteArgument(const std::wstring& argument)
	{
	if (argument.find_first_of(L" \t\"") == std::wstring::npos)
		return argument;

	std::wstring quoted = L"\"";
	size_t backslashes = 0;
	for (wchar_t c : argument)
		{
		if (c == L'\\')
			{
			backslashes++;
			continue;
			}
		if (c == L'"')
			{
			quoted.append(backslashes * 2 + 1, L'\\');
			quoted += c;
			backslashes = 0;
			continue;
			}
		quoted.append(backslashes, L'\\');
		backslashes = 0;
		quoted += c;
		}
	quoted.append(backslashes * 2, L'\\');
	quoted += L'"';
	return quoted;
	}


HANDLE CreateKillOnCloseJob()
	{
	HANDLE job = CreateJobObjectW(NULL, NULL);
	if (!job)
		return NULL;

	JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits = {};
	limits.BasicLimitInformation.LimitFlags =
		JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation,
		&limits, sizeof(limits)))
		{
		CloseHandle(job);
		return NULL;
		}
	return job;
	}


bool SetInternetOption(HINTERNET handle, DWORD option, DWORD value)
	{
	return InternetSetOptionW(handle, option, &value, sizeof(value)) != FALSE;
	}


bool QueryHeader(HINTERNET request, DWORD query, std::wstring& value)
	{
	DWORD size = 0;
	DWORD index = 0;
	HttpQueryInfoW(request, query, NULL, &size, &index);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		return false;

	std::vector<wchar_t> buffer(size / sizeof(wchar_t) + 1);
	index = 0;
	if (!HttpQueryInfoW(request, query, buffer.data(), &size, &index))
		return false;
	value.assign(buffer.data());
	return true;
	}


// Isolates the browser's WinINet request behavior (decoding, redirects,
// HTTPS) from the loader, parser, and UI. Always run under --fetch, which
// supervises this exact executable re-invoked with --fetch-worker so a stuck
// request gets the same job/timeout/dump/stack handling as every other mode.
int Fetch(const wchar_t *url, const wchar_t *output_path,
	DWORD operation_timeout, ULONGLONG max_bytes)
	{
	if (wcscmp(url, L"ncompass-test://hang") == 0)
		{
		Sleep(INFINITE);
		return 1;
		}

	HINTERNET session = NULL;
	HINTERNET connection = NULL;
	HINTERNET request = NULL;
	HANDLE output = INVALID_HANDLE_VALUE;
	int result = 1;
	DWORD started = GetTickCount();
	ULONGLONG total = 0;
	URL_COMPONENTSW components = {};
	components.dwStructSize = sizeof(components);
	components.dwHostNameLength = static_cast<DWORD>(-1);
	components.dwUrlPathLength = static_cast<DWORD>(-1);
	components.dwExtraInfoLength = static_cast<DWORD>(-1);

	if (!InternetCrackUrlW(url, 0, 0, &components))
		{
		std::wcerr << L"InternetCrackUrlW failed: " << GetLastError() << L"\n";
		goto cleanup;
		}

	{
	std::wstring hostname(components.lpszHostName, components.dwHostNameLength);
	std::wstring path(components.lpszUrlPath, components.dwUrlPathLength);
	if (components.dwExtraInfoLength)
		path.append(components.lpszExtraInfo, components.dwExtraInfoLength);
	if (path.empty())
		path = L"/";

	session = InternetOpenW(L"NCompass Version 1A2",
		INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!session)
		{
		std::wcerr << L"InternetOpenW failed: " << GetLastError() << L"\n";
		goto cleanup;
		}

	DWORD retries = 0;
	if (!SetInternetOption(session, INTERNET_OPTION_CONNECT_TIMEOUT,
			operation_timeout) ||
		!SetInternetOption(session, INTERNET_OPTION_RECEIVE_TIMEOUT,
			operation_timeout) ||
		!SetInternetOption(session, INTERNET_OPTION_SEND_TIMEOUT,
			operation_timeout) ||
		!SetInternetOption(session, INTERNET_OPTION_CONNECT_RETRIES, retries))
		{
		std::wcerr << L"InternetSetOptionW failed: " << GetLastError() << L"\n";
		goto cleanup;
		}

	connection = InternetConnectW(session, hostname.c_str(), components.nPort,
		NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	if (!connection)
		{
		std::wcerr << L"InternetConnectW failed: " << GetLastError() << L"\n";
		goto cleanup;
		}

	DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE |
		INTERNET_FLAG_PRAGMA_NOCACHE;
	if (components.nScheme == INTERNET_SCHEME_HTTPS)
		flags |= INTERNET_FLAG_SECURE;
	const wchar_t *accept_types[] = { L"*/*", NULL };
	request = HttpOpenRequestW(connection, L"GET", path.c_str(), NULL, NULL,
		accept_types, flags, 0);
	if (!request)
		{
		std::wcerr << L"HttpOpenRequestW failed: " << GetLastError() << L"\n";
		goto cleanup;
		}

	DWORD decoding = TRUE;
	if (!InternetSetOptionW(request, INTERNET_OPTION_HTTP_DECODING,
			&decoding, sizeof(decoding)) ||
		!HttpAddRequestHeadersW(request,
			L"Accept-Encoding: gzip, deflate\r\n", static_cast<DWORD>(-1),
			HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE))
		{
		std::wcerr << L"Request setup failed: " << GetLastError() << L"\n";
		goto cleanup;
		}

	if (!HttpSendRequestW(request, NULL, 0, NULL, 0))
		{
		std::wcerr << L"HttpSendRequestW failed: " << GetLastError() << L"\n";
		goto cleanup;
		}

	DWORD status = 0;
	DWORD status_size = sizeof(status);
	DWORD index = 0;
	if (!HttpQueryInfoW(request,
			HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
			&status, &status_size, &index))
		{
		std::wcerr << L"Status query failed: " << GetLastError() << L"\n";
		goto cleanup;
		}
	if (status < 200 || status > 299)
		{
		std::wcerr << L"HTTP status " << status << L"\n";
		result = 22;
		goto cleanup;
		}

	output = CreateFileW(output_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (output == INVALID_HANDLE_VALUE)
		{
		std::wcerr << L"Could not create output: " << GetLastError() << L"\n";
		goto cleanup;
		}

	BYTE buffer[65536];
	for (;;)
		{
		DWORD read = 0;
		if (!InternetReadFile(request, buffer, sizeof(buffer), &read))
			{
			std::wcerr << L"InternetReadFile failed: " << GetLastError()
				<< L"\n";
			goto cleanup;
			}
		if (!read)
			break;
		if (total + read > max_bytes)
			{
			std::wcerr << L"Response exceeds byte limit\n";
			result = 27;
			goto cleanup;
			}
		DWORD written = 0;
		if (!WriteFile(output, buffer, read, &written, NULL) ||
			written != read)
			{
			std::wcerr << L"WriteFile failed: " << GetLastError() << L"\n";
			goto cleanup;
			}
		total += read;
		}

	std::wstring content_type;
	QueryHeader(request, HTTP_QUERY_CONTENT_TYPE, content_type);
	DWORD final_size = 0;
	InternetQueryOptionW(request, INTERNET_OPTION_URL, NULL, &final_size);
	std::wstring final_url = url;
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
		std::vector<wchar_t> final_buffer(
			final_size / sizeof(wchar_t) + 1);
		if (InternetQueryOptionW(request, INTERNET_OPTION_URL,
				final_buffer.data(), &final_size))
			final_url.assign(final_buffer.data());
		}

	std::wcout << L"status=" << status << L"\n"
		<< L"bytes=" << total << L"\n"
		<< L"elapsed-ms=" << GetTickCount() - started << L"\n"
		<< L"content-type=" << content_type << L"\n"
		<< L"final-url=" << final_url << L"\n";
	result = 0;
	}

cleanup:
	if (output != INVALID_HANDLE_VALUE)
		CloseHandle(output);
	if (request)
		InternetCloseHandle(request);
	if (connection)
		InternetCloseHandle(connection);
	if (session)
		InternetCloseHandle(session);
	if (result != 0)
		DeleteFileW(output_path);
	return result;
	}


struct SuspendedThread
	{
	DWORD id;
	HANDLE handle;
	};


std::vector<SuspendedThread> SuspendTargetThreads(DWORD process_id)
	{
	std::vector<SuspendedThread> threads;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (snapshot == INVALID_HANDLE_VALUE)
		return threads;

	THREADENTRY32 entry = {};
	entry.dwSize = sizeof(entry);
	if (Thread32First(snapshot, &entry))
		{
		do
			{
			if (entry.th32OwnerProcessID != process_id)
				continue;
			HANDLE thread = OpenThread(THREAD_GET_CONTEXT |
				THREAD_QUERY_INFORMATION | THREAD_SUSPEND_RESUME,
				FALSE, entry.th32ThreadID);
			if (!thread)
				continue;
			if (SuspendThread(thread) == static_cast<DWORD>(-1))
				{
				CloseHandle(thread);
				continue;
				}
			threads.push_back({ entry.th32ThreadID, thread });
			}
		while(Thread32Next(snapshot, &entry));
		}
	CloseHandle(snapshot);
	return threads;
	}


bool WriteLiveStackTrace(HANDLE process,
	const std::vector<SuspendedThread>& threads,
	const std::wstring& stack_path)
	{
	std::wofstream output(std::filesystem::path(stack_path),
		std::ios::trunc);
	if (!output)
		return false;

	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
	if (!SymInitializeW(process, NULL, TRUE))
		return false;

	for (const SuspendedThread& thread : threads)
		{
		CONTEXT context = {};
		context.ContextFlags = CONTEXT_FULL;
		if (!GetThreadContext(thread.handle, &context))
			continue;

		output << L"Thread " << thread.id << L"\n";
		STACKFRAME64 frame = {};
		frame.AddrPC.Offset = context.Eip;
		frame.AddrPC.Mode = AddrModeFlat;
		frame.AddrStack.Offset = context.Esp;
		frame.AddrStack.Mode = AddrModeFlat;
		frame.AddrFrame.Offset = context.Ebp;
		frame.AddrFrame.Mode = AddrModeFlat;

		for (unsigned int depth = 0; depth < 64; depth++)
			{
			if (!StackWalk64(IMAGE_FILE_MACHINE_I386, process,
				thread.handle, &frame, &context, NULL,
				SymFunctionTableAccess64, SymGetModuleBase64, NULL) ||
				!frame.AddrPC.Offset)
				break;

			BYTE symbol_buffer[sizeof(SYMBOL_INFOW) +
				MAX_SYM_NAME * sizeof(wchar_t)] = {};
			PSYMBOL_INFOW symbol =
				reinterpret_cast<PSYMBOL_INFOW>(symbol_buffer);
			symbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
			symbol->MaxNameLen = MAX_SYM_NAME;
			DWORD64 displacement = 0;
			output << L"  0x" << std::hex << frame.AddrPC.Offset;
			if (SymFromAddrW(process, frame.AddrPC.Offset,
				&displacement, symbol))
				output << L" " << symbol->Name << L"+0x"
					<< displacement;
			output << std::dec << L"\n";
			}
		output << L"\n";
		}
	SymCleanup(process);
	return true;
	}


std::wstring MakeDumpBaseName(DWORD process_id)
	{
	SYSTEMTIME time;
	GetLocalTime(&time);

	wchar_t name[128];
	swprintf_s(name, _countof(name),
		L"ncompass-%04u%02u%02u-%02u%02u%02u-%03u-%lu",
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute,
		time.wSecond, time.wMilliseconds, process_id);
	return name;
	}


std::wstring GetOutputDirectory()
	{
	std::wstring directory = GetModuleDirectory() + L"\\dumps";
	if (!CreateDirectoryW(directory.c_str(), NULL) &&
		GetLastError() != ERROR_ALREADY_EXISTS)
		return std::wstring();
	return directory;
	}


bool WriteTextFile(const std::wstring& path, const std::wstring& text)
	{
	HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE)
		return false;

	int byte_count = WideCharToMultiByte(CP_UTF8, 0, text.c_str(),
		static_cast<int>(text.size()), NULL, 0, NULL, NULL);
	if (!byte_count)
		{
		CloseHandle(file);
		return false;
		}

	std::vector<char> bytes(byte_count);
	WideCharToMultiByte(CP_UTF8, 0, text.c_str(),
		static_cast<int>(text.size()), bytes.data(), byte_count, NULL, NULL);

	DWORD written = 0;
	const BYTE byte_order_mark[] = { 0xEF, 0xBB, 0xBF };
	BOOL result = WriteFile(file, byte_order_mark,
		sizeof(byte_order_mark), &written, NULL);
	if (result)
		result = WriteFile(file, bytes.data(),
			static_cast<DWORD>(bytes.size()), &written, NULL);
	CloseHandle(file);
	return result != FALSE;
	}


bool CaptureWindowBitmap(HWND window, const std::wstring& path)
	{
	RECT client;
	if (!GetClientRect(window, &client))
		return false;

	int width = client.right - client.left;
	int height = client.bottom - client.top;
	if (width <= 0 || height <= 0)
		return false;

	HDC window_dc = GetDC(window);
	HDC memory_dc = CreateCompatibleDC(window_dc);
	HBITMAP bitmap = CreateCompatibleBitmap(window_dc, width, height);
	HGDIOBJ old_bitmap = SelectObject(memory_dc, bitmap);

	DWORD_PTR print_result = 0;
	BOOL rendered = SendMessageTimeoutW(window, WM_PRINT,
		reinterpret_cast<WPARAM>(memory_dc),
		PRF_CLIENT | PRF_CHILDREN | PRF_ERASEBKGND | PRF_OWNED,
		SMTO_ABORTIFHUNG | SMTO_BLOCK, 250, &print_result) != 0;
	if (!rendered)
		rendered = BitBlt(memory_dc, 0, 0, width, height,
			window_dc, 0, 0, SRCCOPY);

	BITMAPINFO info = {};
	info.bmiHeader.biSize = sizeof(info.bmiHeader);
	info.bmiHeader.biWidth = width;
	info.bmiHeader.biHeight = height;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 24;
	info.bmiHeader.biCompression = BI_RGB;

	DWORD stride = (width * 3 + 3) & ~3;
	std::vector<BYTE> pixels(stride * height);
	BOOL copied = rendered && GetDIBits(memory_dc, bitmap, 0, height,
		pixels.data(), &info, DIB_RGB_COLORS);

	SelectObject(memory_dc, old_bitmap);
	DeleteObject(bitmap);
	DeleteDC(memory_dc);
	ReleaseDC(window, window_dc);

	if (!copied)
		return false;

	HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE)
		return false;

	BITMAPFILEHEADER file_header = {};
	file_header.bfType = 0x4D42;
	file_header.bfOffBits = sizeof(file_header) + sizeof(info.bmiHeader);
	file_header.bfSize = file_header.bfOffBits +
		static_cast<DWORD>(pixels.size());

	DWORD written = 0;
	BOOL result = WriteFile(file, &file_header, sizeof(file_header),
		&written, NULL);
	if (result)
		result = WriteFile(file, &info.bmiHeader, sizeof(info.bmiHeader),
			&written, NULL);
	if (result)
		result = WriteFile(file, pixels.data(),
			static_cast<DWORD>(pixels.size()), &written, NULL);
	CloseHandle(file);
	return result != FALSE;
	}


BOOL CALLBACK AppendChildWindowText(HWND window, LPARAM lparam)
	{
	std::wstring *text = reinterpret_cast<std::wstring *>(lparam);
	int length = GetWindowTextLengthW(window);
	if (!length)
		return TRUE;

	std::vector<wchar_t> buffer(length + 1);
	GetWindowTextW(window, buffer.data(), static_cast<int>(buffer.size()));
	if (!text->empty())
		*text += L"\r\n";
	*text += buffer.data();
	return TRUE;
	}


std::wstring CaptureDialog(HWND dialog, DWORD process_id)
	{
	std::wstring directory = GetOutputDirectory();
	if (directory.empty())
		return std::wstring();

	wchar_t title[512];
	GetWindowTextW(dialog, title, _countof(title));

	std::wstring text = L"Window: ";
	text += title;
	text += L"\r\n";
	EnumChildWindows(dialog, AppendChildWindowText,
		reinterpret_cast<LPARAM>(&text));
	text += L"\r\n";

	wchar_t suffix[64];
	swprintf_s(suffix, _countof(suffix), L"-dialog-%lu.txt",
		GetTickCount());
	std::wstring path = directory + L"\\" +
		MakeDumpBaseName(process_id) + suffix;
	if (!WriteTextFile(path, text))
		return std::wstring();

	std::wstring bitmap_path = path.substr(0, path.size() - 4) + L".bmp";
	CaptureWindowBitmap(dialog, bitmap_path);
	std::wcout << L"Dialog capture: " << path << L"\n";
	return path;
	}


void DismissDialog(HWND dialog)
	{
	static const int button_ids[] =
		{ IDOK, IDCANCEL, IDNO, IDABORT, IDIGNORE, IDYES };

	for (int i = 0; i < _countof(button_ids); i++)
		{
		HWND button = GetDlgItem(dialog, button_ids[i]);
		if (button && IsWindowEnabled(button))
			{
			PostMessageW(button, BM_CLICK, 0, 0);
			return;
			}
		}

	PostMessageW(dialog, WM_CLOSE, 0, 0);
	}


bool WriteCrashDumpUnbounded(HANDLE process, DWORD process_id, DWORD thread_id,
	const EXCEPTION_RECORD *exception_record, std::wstring& dump_path)
	{
	std::wstring dump_directory = GetOutputDirectory();
	if (dump_directory.empty())
		{
		std::wcerr << L"Could not create dump directory (error "
			<< GetLastError() << L")\n";
		return false;
		}

	std::wstring base_path = dump_directory + L"\\" +
		MakeDumpBaseName(process_id);
	dump_path = base_path + L".dmp";
	HANDLE dump_file = CreateFileW(dump_path.c_str(), GENERIC_WRITE,
		FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (dump_file == INVALID_HANDLE_VALUE)
		{
		std::wcerr << L"Could not create dump file (error "
			<< GetLastError() << L")\n";
		return false;
		}

	MINIDUMP_EXCEPTION_INFORMATION exception_info;
	MINIDUMP_EXCEPTION_INFORMATION *exception_info_pointer = NULL;
	EXCEPTION_RECORD record;
	CONTEXT context;
	EXCEPTION_POINTERS pointers;
	HANDLE thread = NULL;

	if (exception_record && thread_id)
		{
		record = *exception_record;
		ZeroMemory(&context, sizeof(context));
		context.ContextFlags = CONTEXT_ALL;
		thread = OpenThread(THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION,
			FALSE, thread_id);

		if (thread && GetThreadContext(thread, &context))
			{
			pointers.ExceptionRecord = &record;
			pointers.ContextRecord = &context;
			exception_info.ThreadId = thread_id;
			exception_info.ExceptionPointers = &pointers;
			exception_info.ClientPointers = FALSE;
			exception_info_pointer = &exception_info;
			}
		}

	MINIDUMP_TYPE dump_type = static_cast<MINIDUMP_TYPE>(
		MiniDumpWithDataSegs |
		MiniDumpWithHandleData |
		MiniDumpWithIndirectlyReferencedMemory |
		MiniDumpWithUnloadedModules |
		MiniDumpWithFullMemoryInfo |
		MiniDumpWithThreadInfo);

	BOOL result = MiniDumpWriteDump(process, process_id, dump_file,
		dump_type, exception_info_pointer, NULL, NULL);
	DWORD error = result ? ERROR_SUCCESS : GetLastError();

	if (thread)
		CloseHandle(thread);
	CloseHandle(dump_file);

	if (!result)
		{
		DeleteFileW(dump_path.c_str());
		std::wcerr << L"MiniDumpWriteDump failed (error " << error << L")\n";
		return false;
		}

	wchar_t summary[1024];
	if (exception_record)
		{
		swprintf_s(summary, _countof(summary),
			L"Ncompass crash dump\r\n"
			L"Process ID: %lu\r\n"
			L"Thread ID: %lu\r\n"
			L"Exception code: 0x%08lX\r\n"
			L"Exception address: 0x%p\r\n"
			L"Dump: %s\r\n",
			process_id, thread_id, exception_record->ExceptionCode,
			exception_record->ExceptionAddress, dump_path.c_str());
		}
	else
		{
		swprintf_s(summary, _countof(summary),
			L"Ncompass Debug assertion dump\r\n"
			L"Process ID: %lu\r\n"
			L"Dump: %s\r\n",
			process_id, dump_path.c_str());
		}
	WriteTextFile(base_path + L".txt", summary);

	std::wcout << L"Crash dump: " << dump_path << L"\n";
	return true;
	}


bool WriteCrashDump(HANDLE process, DWORD process_id, DWORD thread_id,
	const EXCEPTION_RECORD *exception_record, std::wstring& dump_path)
	{
	return WriteCrashDumpUnbounded(process, process_id, thread_id,
		exception_record, dump_path);
	}


struct MonitorWatchdog
	{
	HANDLE process;
	DWORD process_id;
	DWORD timeout_ms;
	HANDLE cancel_event;
	std::wstring screenshot_path;
	volatile LONG fired;
	bool dump_written;
	bool screenshot_written;
	std::wstring dump_path;
	std::wstring observed_title;
	};


DWORD WINAPI MonitorWatchdogThread(LPVOID parameter)
	{
	MonitorWatchdog *watchdog = static_cast<MonitorWatchdog *>(parameter);
	if (WaitForSingleObject(watchdog->cancel_event,
		watchdog->timeout_ms) != WAIT_TIMEOUT)
		return 0;

	InterlockedExchange(&watchdog->fired, 1);
	std::wcerr << L"Timed out waiting for the process\n";

	std::vector<SuspendedThread> threads =
		SuspendTargetThreads(watchdog->process_id);
	watchdog->dump_written = WriteCrashDump(watchdog->process,
		watchdog->process_id, 0, NULL, watchdog->dump_path);
	std::wstring stack_path = watchdog->dump_path + L".stack.txt";
	if (WriteLiveStackTrace(watchdog->process, threads, stack_path))
		std::wcerr << L"Timeout stack: " << stack_path << L"\n";
	TerminateProcess(watchdog->process, 2);
	for (const SuspendedThread& thread : threads)
		{
		ResumeThread(thread.handle);
		CloseHandle(thread.handle);
		}
	WaitForSingleObject(watchdog->process, 2000);
	return 0;
	}


int MonitorProcess(const std::wstring& application,
	const std::wstring& arguments, const std::wstring& expected_title,
	DWORD timeout_ms, const std::wstring& screenshot_path,
	bool dump_self_test, bool dialog_self_test,
	MonitorOutcome *outcome = NULL,
	const std::vector<NavigationStep> *navigation_steps = NULL)
	{
	if (outcome)
		*outcome = {};

	std::wstring command_line = QuoteArgument(application);
	if (!arguments.empty())
		command_line += L" " + arguments;
	std::vector<wchar_t> command_buffer(command_line.begin(),
		command_line.end());
	command_buffer.push_back(L'\0');

	HANDLE job = CreateKillOnCloseJob();
	if (!job)
		{
		std::wcerr << L"Could not create process job (error "
			<< GetLastError() << L")\n";
		return 2;
		}

	STARTUPINFOW startup_info = {};
	startup_info.cb = sizeof(startup_info);
	PROCESS_INFORMATION process = {};
	if (!CreateProcessW(application.c_str(), command_buffer.data(), NULL, NULL,
		FALSE, DEBUG_ONLY_THIS_PROCESS | CREATE_SUSPENDED, NULL, NULL,
		&startup_info, &process))
		{
		std::wcerr << L"Could not start process (error " << GetLastError()
			<< L")\n";
		CloseHandle(job);
		return 2;
		}
	if (!AssignProcessToJobObject(job, process.hProcess))
		{
		std::wcerr << L"Could not assign process job (error "
			<< GetLastError() << L")\n";
		TerminateProcess(process.hProcess, 2);
		CloseHandle(process.hThread);
		CloseHandle(process.hProcess);
		CloseHandle(job);
		return 2;
		}
	if (ResumeThread(process.hThread) == static_cast<DWORD>(-1))
		{
		std::wcerr << L"Could not resume process (error "
			<< GetLastError() << L")\n";
		TerminateProcess(process.hProcess, 2);
		CloseHandle(process.hThread);
		CloseHandle(process.hProcess);
		CloseHandle(job);
		return 2;
		}

	DWORD started = GetTickCount();
	DWORD title_matched = 0;
	DWORD close_requested = 0;
	size_t navigation_index = 0;
	std::wstring active_expected_title = expected_title;
	bool navigation_pending = false;
	DWORD navigation_posted = 0;
	NavigationStep pending_navigation_step;
	bool process_exited = false;
	bool dump_written = false;
	bool assertion_dumped = false;
	bool dialog_captured = false;
	bool timed_out = false;
	bool screenshot_written = false;
	std::wstring dump_path;
	std::wstring observed_title;
	DWORD exit_code = 0;
	HWND browser_window = NULL;
	std::vector<HWND> handled_dialogs;
	MonitorWatchdog watchdog = {};
	watchdog.process = process.hProcess;
	watchdog.process_id = process.dwProcessId;
	watchdog.timeout_ms = timeout_ms;
	watchdog.screenshot_path = screenshot_path;
	watchdog.cancel_event = CreateEventW(NULL, TRUE, FALSE, NULL);
	HANDLE watchdog_thread = NULL;
	if (timeout_ms && watchdog.cancel_event)
		watchdog_thread = CreateThread(NULL, 0, MonitorWatchdogThread,
			&watchdog, 0, NULL);
	if (timeout_ms && (!watchdog.cancel_event || !watchdog_thread))
		{
		std::wcerr << L"Could not start timeout watchdog (error "
			<< GetLastError() << L")\n";
		TerminateProcess(process.hProcess, 2);
		if (watchdog_thread)
			CloseHandle(watchdog_thread);
		if (watchdog.cancel_event)
			CloseHandle(watchdog.cancel_event);
		CloseHandle(process.hThread);
		CloseHandle(process.hProcess);
		CloseHandle(job);
		return 2;
		}

	while (!process_exited)
		{
		if (close_requested &&
			GetTickCount() - close_requested >= 2000)
			{
			DebugActiveProcessStop(process.dwProcessId);
			TerminateJobObject(job, 0);
			WaitForSingleObject(process.hProcess, 2000);
			process_exited = true;
			break;
			}

		if (InterlockedCompareExchange(&watchdog.fired, 0, 0) &&
			WaitForSingleObject(watchdog_thread, 0) == WAIT_OBJECT_0)
			{
			DebugActiveProcessStop(process.dwProcessId);
			TerminateJobObject(job, 2);
			WaitForSingleObject(process.hProcess, 2000);
			process_exited = true;
			break;
			}

		DEBUG_EVENT event;
		DWORD wait_time = 100;
		DWORD events_processed = 0;
		while (WaitForDebugEvent(&event, wait_time))
			{
			wait_time = 0;
			DWORD continue_status = DBG_CONTINUE;

			switch(event.dwDebugEventCode)
				{
				case EXCEPTION_DEBUG_EVENT:
					if (event.u.Exception.dwFirstChance &&
						event.u.Exception.ExceptionRecord.ExceptionCode ==
							EXCEPTION_BREAKPOINT)
						{
						continue_status = DBG_CONTINUE;
						}
					else if (event.u.Exception.dwFirstChance)
						{
						continue_status = DBG_EXCEPTION_NOT_HANDLED;
						}
					else
						{
						dump_written = WriteCrashDump(process.hProcess,
							process.dwProcessId, event.dwThreadId,
							&event.u.Exception.ExceptionRecord, dump_path);
						continue_status = DBG_EXCEPTION_NOT_HANDLED;
						}
					break;

				case CREATE_PROCESS_DEBUG_EVENT:
					if (event.u.CreateProcessInfo.hFile)
						CloseHandle(event.u.CreateProcessInfo.hFile);
					break;

				case CREATE_THREAD_DEBUG_EVENT:
					if (event.u.CreateThread.hThread)
						CloseHandle(event.u.CreateThread.hThread);
					break;

				case LOAD_DLL_DEBUG_EVENT:
					if (event.u.LoadDll.hFile)
						CloseHandle(event.u.LoadDll.hFile);
					break;

				case EXIT_PROCESS_DEBUG_EVENT:
					exit_code = event.u.ExitProcess.dwExitCode;
					process_exited = true;
					break;
				}

			ContinueDebugEvent(event.dwProcessId, event.dwThreadId,
				continue_status);

			if (process_exited)
				break;
			if (timeout_ms &&
				GetTickCount() - started >= timeout_ms)
				break;
			if (++events_processed >= 64)
				break;
			}

		if (process_exited)
			break;

		WindowSearch search = { process.dwProcessId, NULL, NULL, NULL };
		EnumWindows(FindProcessWindows, reinterpret_cast<LPARAM>(&search));
		if (search.browser_window &&
			(!browser_window || !IsWindow(browser_window)))
			browser_window = search.browser_window;

		if (search.assertion_window && !assertion_dumped)
			{
			CaptureDialog(search.assertion_window, process.dwProcessId);
			assertion_dumped = true;
			dump_written = WriteCrashDump(process.hProcess,
				process.dwProcessId, 0, NULL, dump_path);
			HWND abort_button = GetDlgItem(search.assertion_window, IDABORT);
			if (abort_button)
				PostMessageW(abort_button, BM_CLICK, 0, 0);
			}

		if (search.dialog_window &&
			std::find(handled_dialogs.begin(), handled_dialogs.end(),
				search.dialog_window) == handled_dialogs.end())
			{
			handled_dialogs.push_back(search.dialog_window);
			dialog_captured =
				!CaptureDialog(search.dialog_window,
					process.dwProcessId).empty();
			DismissDialog(search.dialog_window);
			if (dialog_captured && !active_expected_title.empty())
				TerminateProcess(process.hProcess, 3);
			}

		if (!active_expected_title.empty() && browser_window)
			{
			// A location-bar navigation isn't guaranteed to have landed if
			// the UI thread was briefly busy; retry if the title hasn't
			// moved. Re-entering the same URL is idempotent, so a
			// duplicate attempt is harmless.
			if (navigation_pending && !title_matched &&
				GetTickCount() - navigation_posted >= 2000)
				{
				std::wcerr << L"Retrying navigation to "
					<< pending_navigation_step.url << L"\n";
				NavigateBrowser(browser_window, pending_navigation_step);
				navigation_posted = GetTickCount();
				}

			wchar_t title[512];
			GetWindowTextW(browser_window, title, _countof(title));
			observed_title = title;
			if (TitleMatches(title, active_expected_title))
				{
				navigation_pending = false;
				if (!title_matched)
					title_matched = GetTickCount();
				else if (GetTickCount() - title_matched >= TITLE_SETTLE_MS)
					{
					if (navigation_steps &&
						navigation_index < navigation_steps->size())
						{
						const NavigationStep& step =
							(*navigation_steps)[navigation_index++];
						std::wcout << L"Navigate: " << step.url << L"\n";
						if (!NavigateBrowser(browser_window, step))
							std::wcerr << L"Initial navigation attempt "
								L"did not land; will retry\n";
						active_expected_title = step.expected_title;
						pending_navigation_step = step;
						navigation_pending = true;
						navigation_posted = GetTickCount();
						title_matched = 0;
						continue;
						}

					SetEvent(watchdog.cancel_event);
					if (!screenshot_path.empty() && !screenshot_written)
						{
						screenshot_written =
							CaptureWindowBitmap(browser_window,
								screenshot_path);
						if (screenshot_written)
							std::wcout << L"Screenshot: "
								<< screenshot_path << L"\n";
						else
							std::wcerr << L"Could not capture screenshot\n";
						}
					if (!close_requested)
						{
						close_requested = GetTickCount();
						PostMessageW(browser_window, WM_CLOSE, 0, 0);
						}
					}
				}
			}

		}

	if (watchdog_thread)
		{
		SetEvent(watchdog.cancel_event);
		WaitForSingleObject(watchdog_thread, INFINITE);
		CloseHandle(watchdog_thread);
		}
	if (watchdog.cancel_event)
		CloseHandle(watchdog.cancel_event);
	timed_out = InterlockedCompareExchange(&watchdog.fired, 0, 0) != 0;
	dump_written = dump_written || watchdog.dump_written;
	screenshot_written =
		screenshot_written || watchdog.screenshot_written;

	if (outcome)
		{
		outcome->timed_out = timed_out;
		outcome->dump_written = dump_written;
		outcome->dialog_captured = dialog_captured;
		outcome->title_matched = title_matched != 0;
		outcome->screenshot_written = screenshot_written;
		outcome->exit_code = exit_code;
		outcome->dump_path = watchdog.dump_written ?
			watchdog.dump_path : dump_path;
		outcome->observed_title = observed_title;
		}

	CloseHandle(process.hThread);
	CloseHandle(process.hProcess);
	CloseHandle(job);

	if (dump_self_test)
		{
		if (dump_written)
			{
			std::wcout << L"PASS: crash dump self-test\n";
			return 0;
			}
		std::wcerr << L"FAIL: crash dump was not written\n";
		return 1;
		}

	if (dialog_self_test)
		{
		if (dialog_captured)
			{
			std::wcout << L"PASS: dialog capture self-test\n";
			return 0;
			}
		std::wcerr << L"FAIL: dialog was not captured\n";
		return 1;
		}

	if (timed_out)
		return 1;

	if (!screenshot_path.empty() && !screenshot_written)
		return 1;

	if (dump_written)
		return 1;

	if (!active_expected_title.empty() && !title_matched)
		{
		std::wcerr << L"FAIL: expected title was not rendered\n";
		return 1;
		}

	if (title_matched)
		{
		std::wcout << L"PASS: " << active_expected_title << L"\n";
		return 0;
		}

	return static_cast<int>(exit_code);
	}


enum class SiteStatus
	{
	Passed,
	Crash,
	Timeout,
	Dialog,
	Failed
	};


struct SiteEntry
	{
	std::wstring name;
	std::wstring url;
	std::wstring expected_title;
	};


struct SiteAttempt
	{
	std::wstring name;
	SiteStatus status;
	int result;
	DWORD elapsed_ms;
	MonitorOutcome outcome;
	std::filesystem::path screenshot;
	};


std::wstring FromUtf8(const std::string& text)
	{
	if (text.empty())
		return std::wstring();
	int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
		text.data(), static_cast<int>(text.size()), NULL, 0);
	if (!length)
		throw std::runtime_error("Invalid UTF-8 in site manifest");
	std::wstring result(length, L'\0');
	MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(),
		static_cast<int>(text.size()), result.data(), length);
	return result;
	}


std::string ToUtf8(const std::wstring& text)
	{
	if (text.empty())
		return std::string();
	int length = WideCharToMultiByte(CP_UTF8, 0, text.data(),
		static_cast<int>(text.size()), NULL, 0, NULL, NULL);
	if (!length)
		throw std::runtime_error("Could not encode UTF-8");
	std::string result(length, '\0');
	WideCharToMultiByte(CP_UTF8, 0, text.data(),
		static_cast<int>(text.size()), result.data(), length, NULL, NULL);
	return result;
	}


std::string JsonString(const std::wstring& text)
	{
	std::string utf8 = ToUtf8(text);
	std::ostringstream output;
	output << '"';
	for (unsigned char c : utf8)
		{
		switch(c)
			{
			case '"': output << "\\\""; break;
			case '\\': output << "\\\\"; break;
			case '\b': output << "\\b"; break;
			case '\f': output << "\\f"; break;
			case '\n': output << "\\n"; break;
			case '\r': output << "\\r"; break;
			case '\t': output << "\\t"; break;
			default:
				if (c < 0x20)
					{
					char escaped[7];
					sprintf_s(escaped, "\\u%04x", c);
					output << escaped;
					}
				else
					output << static_cast<char>(c);
				break;
			}
		}
	output << '"';
	return output.str();
	}


std::vector<SiteEntry> ReadSiteManifest(const std::filesystem::path& path)
	{
	std::ifstream input(path);
	if (!input)
		throw std::runtime_error("Could not open site manifest");

	std::vector<SiteEntry> sites;
	std::string line;
	unsigned int line_number = 0;
	while (std::getline(input, line))
		{
		line_number++;
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		if (line.empty() || line[0] == '#')
			continue;

		size_t first = line.find('\t');
		size_t second = first == std::string::npos ?
			std::string::npos : line.find('\t', first + 1);
		if (first == std::string::npos || second == std::string::npos ||
			line.find('\t', second + 1) != std::string::npos)
			{
			throw std::runtime_error("Invalid site manifest line " +
				std::to_string(line_number));
			}

		SiteEntry site = {
			FromUtf8(line.substr(0, first)),
			FromUtf8(line.substr(first + 1, second - first - 1)),
			FromUtf8(line.substr(second + 1))
			};
		if (site.name.empty() || site.url.empty() ||
			site.expected_title.empty())
			throw std::runtime_error("Empty site manifest field on line " +
				std::to_string(line_number));
		sites.push_back(std::move(site));
		}
	if (sites.empty())
		throw std::runtime_error("Site manifest is empty");
	return sites;
	}


const wchar_t *SiteStatusName(SiteStatus status)
	{
	switch(status)
		{
		case SiteStatus::Passed: return L"passed";
		case SiteStatus::Crash: return L"crash";
		case SiteStatus::Timeout: return L"timeout";
		case SiteStatus::Dialog: return L"dialog";
		default: return L"failed";
		}
	}


SiteStatus ClassifySite(const MonitorOutcome& outcome, int result)
	{
	if (outcome.dialog_captured)
		return SiteStatus::Dialog;
	if (outcome.timed_out)
		return SiteStatus::Timeout;
	if (outcome.dump_written)
		return SiteStatus::Crash;
	if (result == 0 && outcome.title_matched)
		return SiteStatus::Passed;
	return SiteStatus::Failed;
	}


std::wstring GetGflagsPath()
	{
	std::filesystem::path gflags =
		GetDebuggerToolPath(L"gflags.exe");
	if (gflags.empty())
		return std::wstring();
	if (!std::filesystem::exists(gflags))
		return std::wstring();
	return gflags.wstring();
	}


bool RunTool(const std::wstring& application, const std::wstring& arguments,
	DWORD timeout_ms)
	{
	std::wstring command = QuoteArgument(application) + L" " + arguments;
	std::vector<wchar_t> command_buffer(command.begin(), command.end());
	command_buffer.push_back(L'\0');
	HANDLE job = CreateKillOnCloseJob();
	if (!job)
		return false;
	STARTUPINFOW startup = {};
	startup.cb = sizeof(startup);
	PROCESS_INFORMATION process = {};
	if (!CreateProcessW(application.c_str(), command_buffer.data(), NULL, NULL,
		FALSE, CREATE_NO_WINDOW | CREATE_SUSPENDED, NULL, NULL,
		&startup, &process))
		{
		CloseHandle(job);
		return false;
		}
	if (!AssignProcessToJobObject(job, process.hProcess) ||
		ResumeThread(process.hThread) == static_cast<DWORD>(-1))
		{
		TerminateProcess(process.hProcess, 1);
		CloseHandle(process.hThread);
		CloseHandle(process.hProcess);
		CloseHandle(job);
		return false;
		}
	CloseHandle(process.hThread);
	DWORD wait = WaitForSingleObject(process.hProcess, timeout_ms);
	if (wait == WAIT_TIMEOUT)
		{
		TerminateProcess(process.hProcess, 1);
		WaitForSingleObject(process.hProcess, 1000);
		}
	DWORD exit_code = 1;
	GetExitCodeProcess(process.hProcess, &exit_code);
	CloseHandle(process.hProcess);
	CloseHandle(job);
	return wait == WAIT_OBJECT_0 && exit_code == 0;
	}


class PageHeapScope
	{
public:
	PageHeapScope() : enabled_(false)
		{
		}

	bool Enable()
		{
		path_ = GetGflagsPath();
		if (path_.empty())
			return false;
		enabled_ = true;
		if (!RunTool(path_, L"/p /enable ncompass.exe /full", 5000))
			{
			RunTool(path_, L"/p /disable ncompass.exe", 5000);
			enabled_ = false;
			return false;
			}
		return true;
		}

	bool Disable()
		{
		if (!enabled_)
			return true;
		bool result = RunTool(path_, L"/p /disable ncompass.exe", 5000);
		if (result)
			enabled_ = false;
		return result;
		}

	~PageHeapScope()
		{
		Disable();
		}

private:
	bool enabled_;
	std::wstring path_;
	};


SiteAttempt RunSiteAttempt(const SiteEntry& site,
	const std::filesystem::path& output_directory,
	const std::wstring& attempt_name, DWORD timeout_ms)
	{
	std::filesystem::path site_directory =
		output_directory / site.name / attempt_name;
	std::filesystem::create_directories(site_directory);
	std::filesystem::path screenshot = site_directory / L"render.bmp";
	std::wstring browser = GetModuleDirectory() + L"\\ncompass.exe";

	MonitorOutcome outcome = {};
	DWORD started = GetTickCount();
	int result = MonitorProcess(browser, site.url, site.expected_title,
		timeout_ms, screenshot.wstring(), false, false, &outcome);
	DWORD elapsed = GetTickCount() - started;
	SiteAttempt attempt = {
		attempt_name,
		ClassifySite(outcome, result),
		result,
		elapsed,
		outcome,
		screenshot
		};
	std::wcout << SiteStatusName(attempt.status) << L": " << site.name
		<< L" [" << attempt_name << L"] " << elapsed << L" ms\n";
	if (attempt.status != SiteStatus::Passed &&
		!attempt.outcome.observed_title.empty())
		std::wcout << L"observed title: "
			<< attempt.outcome.observed_title << L"\n";
	return attempt;
	}


void WriteSiteSummary(const std::filesystem::path& output_directory,
	DWORD timeout_ms, const std::vector<SiteEntry>& sites,
	const std::vector<std::vector<SiteAttempt>>& attempts)
	{
	std::ofstream output(output_directory / L"summary.json",
		std::ios::binary | std::ios::trunc);
	if (!output)
		throw std::runtime_error("Could not create site summary");
	output << "{\n  \"timeoutMilliseconds\": " << timeout_ms
		<< ",\n  \"results\": [\n";
	for (size_t i = 0; i < sites.size(); i++)
		{
		if (i)
			output << ",\n";
		output << "    {\n"
			<< "      \"name\": " << JsonString(sites[i].name) << ",\n"
			<< "      \"url\": " << JsonString(sites[i].url) << ",\n"
			<< "      \"expectedTitle\": "
			<< JsonString(sites[i].expected_title) << ",\n"
			<< "      \"attempts\": [\n";
		for (size_t j = 0; j < attempts[i].size(); j++)
			{
			const SiteAttempt& attempt = attempts[i][j];
			if (j)
				output << ",\n";
			output << "        {\"name\": " << JsonString(attempt.name)
				<< ", \"status\": "
				<< JsonString(SiteStatusName(attempt.status))
				<< ", \"result\": " << attempt.result
				<< ", \"elapsedMilliseconds\": " << attempt.elapsed_ms
				<< ", \"dump\": "
				<< (attempt.outcome.dump_path.empty() ? "null" :
					JsonString(attempt.outcome.dump_path))
				<< ", \"observedTitle\": "
				<< JsonString(attempt.outcome.observed_title)
				<< ", \"screenshot\": "
				<< (attempt.outcome.screenshot_written ?
					JsonString(attempt.screenshot.wstring()) : "null")
				<< "}";
			}
		output << "\n      ]\n    }";
		}
	output << "\n  ]\n}\n";
	}


int RunSites(const std::filesystem::path& manifest,
	const std::filesystem::path& output_directory, DWORD timeout_ms,
	const std::wstring& page_heap_mode)
	{
	std::vector<SiteEntry> sites = ReadSiteManifest(manifest);
	std::filesystem::create_directories(output_directory);
	std::vector<std::vector<SiteAttempt>> all_attempts;
	bool failed = false;

	for (const SiteEntry& site : sites)
		{
		std::wcout << L"test: " << site.name << L" " << site.url << L"\n";
		std::vector<SiteAttempt> attempts;
		if (page_heap_mode == L"always")
			{
			PageHeapScope page_heap;
			if (!page_heap.Enable())
				throw std::runtime_error("Could not enable PageHeap");
			attempts.push_back(RunSiteAttempt(site, output_directory,
				L"pageheap", timeout_ms));
			if (!page_heap.Disable())
				throw std::runtime_error("Could not disable PageHeap");
			}
		else
			{
			attempts.push_back(RunSiteAttempt(site, output_directory,
				L"baseline", timeout_ms));
			if (page_heap_mode == L"on-crash" &&
				attempts.front().status == SiteStatus::Crash)
				{
				PageHeapScope page_heap;
				if (!page_heap.Enable())
					throw std::runtime_error("Could not enable PageHeap");
				attempts.push_back(RunSiteAttempt(site, output_directory,
					L"pageheap", timeout_ms));
				if (!page_heap.Disable())
					throw std::runtime_error("Could not disable PageHeap");
				}
			}
		if (attempts.front().status != SiteStatus::Passed)
			failed = true;
		all_attempts.push_back(std::move(attempts));
		}

	WriteSiteSummary(output_directory, timeout_ms, sites, all_attempts);
	std::wcout << L"summary: "
		<< (output_directory / L"summary.json").wstring() << L"\n";
	return failed ? 1 : 0;
	}


int wmain(int argc, wchar_t *argv[])
	{
	if (argc >= 2 && wcscmp(argv[1], L"--wrap") == 0)
		{
		int command_index = 2;
		LPCWSTR working_directory = NULL;
		std::wstring working_directory_storage;
		if (command_index < argc &&
			wcscmp(argv[command_index], L"--cwd") == 0)
			{
			if (command_index + 2 >= argc)
				{
				std::wcerr << L"Usage: ncompass-debug --wrap [--cwd "
					L"directory] [--] command [arguments...]\n";
				return 2;
				}
			working_directory_storage = argv[command_index + 1];
			working_directory = working_directory_storage.c_str();
			command_index += 2;
			}
		if (command_index < argc &&
			wcscmp(argv[command_index], L"--") == 0)
			command_index++;
		if (command_index >= argc)
			{
			std::wcerr << L"Usage: ncompass-debug --wrap [--cwd directory] "
				L"[--] command [arguments...]\n";
			return 2;
			}

		std::wstring command_line;
		for (int i = command_index; i < argc; i++)
			{
			if (!command_line.empty())
				command_line.push_back(L' ');
			command_line += QuoteArgument(argv[i]);
			}

		HANDLE job = CreateKillOnCloseJob();
		if (!job)
			{
			std::wcerr << L"Could not create process job (error "
				<< GetLastError() << L")\n";
			return 1;
			}

		std::vector<wchar_t> mutable_command(
			command_line.begin(), command_line.end());
		mutable_command.push_back(L'\0');

		STARTUPINFOW startup = {};
		startup.cb = sizeof(startup);
		PROCESS_INFORMATION process = {};
		if (!CreateProcessW(NULL, mutable_command.data(), NULL, NULL, FALSE,
			CREATE_SUSPENDED, NULL, working_directory, &startup, &process))
			{
			std::wcerr << L"Could not start process (error "
				<< GetLastError() << L")\n";
			CloseHandle(job);
			return 1;
			}
		if (!AssignProcessToJobObject(job, process.hProcess))
			{
			std::wcerr << L"Could not assign process job (error "
				<< GetLastError() << L")\n";
			TerminateProcess(process.hProcess, 1);
			CloseHandle(process.hThread);
			CloseHandle(process.hProcess);
			CloseHandle(job);
			return 1;
			}
		if (ResumeThread(process.hThread) == static_cast<DWORD>(-1))
			{
			std::wcerr << L"Could not resume process (error "
				<< GetLastError() << L")\n";
			TerminateJobObject(job, 1);
			CloseHandle(process.hThread);
			CloseHandle(process.hProcess);
			CloseHandle(job);
			return 1;
			}

		CloseHandle(process.hThread);
		WaitForSingleObject(process.hProcess, INFINITE);
		DWORD exit_code = 1;
		GetExitCodeProcess(process.hProcess, &exit_code);
		CloseHandle(process.hProcess);
		CloseHandle(job);
		return static_cast<int>(exit_code);
		}

	if (argc == 6 && wcscmp(argv[1], L"--fetch-worker") == 0)
		{
		DWORD operation_timeout = wcstoul(argv[4], NULL, 10);
		ULONGLONG max_bytes = _wcstoui64(argv[5], NULL, 10);
		return Fetch(argv[2], argv[3], operation_timeout, max_bytes);
		}

	if (argc >= 4 && argc <= 6 && wcscmp(argv[1], L"--fetch") == 0)
		{
		DWORD timeout_ms = argc >= 5 ? wcstoul(argv[4], NULL, 10) : 5000;
		ULONGLONG max_bytes = argc >= 6 ? _wcstoui64(argv[5], NULL, 10) :
			16ULL * 1024 * 1024;
		if (!timeout_ms || !max_bytes)
			{
			std::wcerr << L"Timeout and byte limit must be positive\n";
			return 2;
			}
		DWORD operation_timeout = timeout_ms > 1000 ? timeout_ms - 500 :
			timeout_ms;
		std::wstring arguments = L"--fetch-worker " +
			QuoteArgument(argv[2]) + L" " + QuoteArgument(argv[3]) + L" " +
			std::to_wstring(operation_timeout) + L" " +
			std::to_wstring(max_bytes);
		MonitorOutcome outcome = {};
		int result = MonitorProcess(GetModulePath(), arguments,
			std::wstring(), timeout_ms, std::wstring(), false, false,
			&outcome);
		if (outcome.timed_out)
			{
			std::wcerr << L"Fetch exceeded " << timeout_ms << L" ms\n";
			return 124;
			}
		return result;
		}

	if (argc >= 7 && (argc - 3) % 2 == 0 &&
		wcscmp(argv[1], L"--navigate") == 0)
		{
		wchar_t *end = NULL;
		unsigned long timeout_ms = wcstoul(argv[2], &end, 10);
		if (!argv[2][0] || *end || !timeout_ms)
			{
			std::wcerr << L"timeout-ms must be a positive integer\n";
			return 2;
			}

		std::vector<NavigationStep> steps;
		for (int i = 5; i < argc; i += 2)
			steps.push_back({ argv[i], argv[i + 1] });
		std::wstring browser = GetModuleDirectory() + L"\\ncompass.exe";
		return MonitorProcess(browser, QuoteArgument(argv[3]), argv[4],
			timeout_ms, std::wstring(), false, false, NULL, &steps);
		}

	if (argc >= 4 && wcscmp(argv[1], L"--command") == 0)
		{
		wchar_t *end = NULL;
		unsigned long timeout_ms = wcstoul(argv[2], &end, 10);
		if (!argv[2][0] || *end || !timeout_ms)
			{
			std::wcerr << L"timeout-ms must be a positive integer\n";
			return 2;
			}
		std::wstring arguments;
		for (int i = 4; i < argc; i++)
			{
			if (!arguments.empty())
				arguments += L" ";
			arguments += QuoteArgument(argv[i]);
			}
		return MonitorProcess(argv[3], arguments, std::wstring(),
			timeout_ms, std::wstring(), false, false);
		}

	if (argc >= 4 && argc <= 6 && wcscmp(argv[1], L"--sites") == 0)
		{
		wchar_t *end = NULL;
		unsigned long timeout_ms = argc >= 5 ?
			wcstoul(argv[4], &end, 10) : 1000;
		if ((argc >= 5 && (!argv[4][0] || *end)) || !timeout_ms)
			{
			std::wcerr << L"timeout-ms must be a positive integer\n";
			return 2;
			}
		std::wstring page_heap_mode =
			argc >= 6 ? argv[5] : L"on-crash";
		if (page_heap_mode != L"off" &&
			page_heap_mode != L"on-crash" &&
			page_heap_mode != L"always")
			{
			std::wcerr << L"pageheap must be off, on-crash, or always\n";
			return 2;
			}
		try
			{
			return RunSites(argv[2], argv[3], timeout_ms,
				page_heap_mode);
			}
		catch(const std::exception& error)
			{
			std::wcerr << L"Site harness failed: "
				<< FromUtf8(error.what()) << L"\n";
			return 2;
			}
		}

	if (argc == 2 && wcscmp(argv[1], L"--crash-child") == 0)
		{
		volatile int *invalid = NULL;
		*invalid = 1;
		return 1;
		}

	if (argc == 2 && wcscmp(argv[1], L"--dialog-child") == 0)
		{
		MessageBoxW(NULL, L"Automatic dialog capture test message.",
			L"Ncompass Debug Dialog Test", MB_OK | MB_ICONERROR);
		return 0;
		}

	if (argc == 2 && wcscmp(argv[1], L"--hang-child") == 0)
		{
		Sleep(INFINITE);
		return 1;
		}

	if (argc == 2 && wcscmp(argv[1], L"--self-test") == 0)
		return MonitorProcess(GetModulePath(), L"--crash-child",
			std::wstring(), 15000, std::wstring(), true, false);

	if (argc == 2 && wcscmp(argv[1], L"--dialog-self-test") == 0)
		return MonitorProcess(GetModulePath(), L"--dialog-child",
			std::wstring(), 15000, std::wstring(), false, true);

	if (argc == 2 && wcscmp(argv[1], L"--timeout-self-test") == 0)
		{
		DWORD started = GetTickCount();
		int result = MonitorProcess(GetModulePath(), L"--hang-child",
			std::wstring(), 1000, std::wstring(), false, false);
		DWORD elapsed = GetTickCount() - started;
		if (result == 1 && elapsed <= 10000)
			{
			std::wcout << L"PASS: timeout watchdog completed in "
				<< elapsed << L" ms\n";
			return 0;
			}
		std::wcerr << L"FAIL: timeout watchdog result " << result
			<< L" after " << elapsed << L" ms\n";
		return 1;
		}

	if (argc < 2 || argc > 5)
		{
		std::wcerr
			<< L"Usage: ncompass-debug <url> [expected-title] "
				L"[timeout-seconds] [screenshot.bmp]\n"
			<< L"       ncompass-debug --self-test\n"
			<< L"       ncompass-debug --dialog-self-test\n"
			<< L"       ncompass-debug --timeout-self-test\n"
			<< L"       ncompass-debug --command timeout-ms application "
				L"[arguments...]\n"
			<< L"       ncompass-debug --sites manifest.tsv output-dir "
				L"[timeout-ms] [off|on-crash|always]\n"
			<< L"       ncompass-debug --navigate timeout-ms url title "
				L"[url title...]\n"
			<< L"       ncompass-debug --fetch url output-file "
				L"[timeout-ms] [max-bytes]\n"
			<< L"       ncompass-debug --wrap [--cwd directory] [--] "
				L"command [arguments...]\n";
		return 2;
		}

	DWORD timeout_seconds = argc >= 3 ? 1 : 0;
	if (argc >= 4)
		{
		wchar_t *end = NULL;
		unsigned long value = wcstoul(argv[3], &end, 10);
		if (!argv[3][0] || *end || !value)
			{
			std::wcerr << L"timeout-seconds must be a positive integer\n";
			return 2;
			}
		timeout_seconds = value;
		}

	std::wstring browser = GetModuleDirectory() + L"\\ncompass.exe";
	if (GetFileAttributesW(browser.c_str()) == INVALID_FILE_ATTRIBUTES)
		{
		std::wcerr << L"Could not find ncompass.exe beside the helper\n";
		return 2;
		}

	std::wstring expected_title = argc >= 3 ? argv[2] : L"";
	std::wstring screenshot_path = argc == 5 ? argv[4] : L"";
	return MonitorProcess(browser, argv[1], expected_title,
		timeout_seconds * 1000, screenshot_path, false, false);
	}

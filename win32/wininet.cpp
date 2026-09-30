// wininet.cpp : WinINet-based implementation of CProtocolHTTP
//
// Historically this file's contents lived inline in protocol.cpp. Isolating
// the WinINet transport here keeps protocol.cpp itself close to its
// original, historical form; CProtocolHTTP's class declaration remains in
// protocol.h alongside CProtocolFile and CProtocolSMTP, exactly as before.

#include "cross_p.h"
#include "resource.h"

#include "viewhtml.h"

#ifndef _DYNLOAD_H_
#include "dynload.h"
#endif

#ifndef _PROTOCOL_H_
#include "protocol.h"

#include <wininet.h>
#endif

#define BUFFER_SIZE 65536

///////////////////////////////////////////////////////////
// CProtocolHTTP 

CProtocolHTTP::CProtocolHTTP(CDynamicLoad *load_object)
	: CProtocol(load_object )
	{
	m_thread_handle = NULL;
	m_session = NULL;
	m_aborted = FALSE;
	m_thread_done_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
	}

CProtocolHTTP::~CProtocolHTTP()
	{
	ASSERT(m_thread_handle == NULL);
	CloseHandle(m_thread_done_semaphore);
	}

void CProtocolHTTP::BeginLoadThread()
	{
	m_thread_handle = AfxBeginThread( WinInetWorkerThread, (LPVOID)this );
	}

void CProtocolHTTP::AbortLoadThread()
	{
	ASSERT(m_thread_handle);
	
	// do everything possible to get the thread to teminate as quickly as possible

	m_access_lock.DEBUG_LOCK();
	m_aborted = TRUE;
	HINTERNET session = (HINTERNET)m_session;
	m_session = NULL;
	m_access_lock.Unlock();

	if (session)
		InternetCloseHandle(session);

	WaitForSingleObject( m_thread_done_semaphore, INFINITE );
	m_thread_handle = NULL;
	}

void CProtocolHTTP::WaitEndLoadThread()
	{
	ASSERT(m_thread_handle);
	WaitForSingleObject( m_thread_done_semaphore, INFINITE );
	m_thread_handle = NULL;
	}

BOOL CProtocolHTTP::SetSession(LPVOID session)
	{
	m_access_lock.DEBUG_LOCK();
	if (m_aborted)
		{
		m_access_lock.Unlock();
		return FALSE;
		}
	m_session = session;
	m_access_lock.Unlock();
	return TRUE;
	}

void CProtocolHTTP::CloseSession(LPVOID session)
	{
	BOOL close_session = FALSE;

	m_access_lock.DEBUG_LOCK();
	if (m_session == session)
		{
		m_session = NULL;
		close_session = TRUE;
		}
	m_access_lock.Unlock();

	if (close_session)
		InternetCloseHandle((HINTERNET)session);
	}

BOOL CProtocolHTTP::IsAborted()
	{
	m_access_lock.DEBUG_LOCK();
	BOOL aborted = m_aborted;
	m_access_lock.Unlock();
	return aborted;
	}

BOOL ParseHeader(BOOL& first_line, LPBYTE &buffer,INT32& amount,CString& current_line,CMapStringToString& mime_map, INT32& response_code)
	{
	const char *p;

	while(amount > 0)
		{
		switch(*buffer)
			{
			case '\r':
				break;
			case '\n':
				if (current_line.GetLength() == 0)
					{
					buffer++;
					amount--;
					return TRUE;
					}

				p = current_line;

				if (first_line)
					{
					// skip initial spaces
					while(*p && *p == ' ')
						p++;
					
					// skip HTTP/1.0
					while(*p && *p != ' ')
						p++;
					
					// skip spaces before response code
					while(*p && *p == ' ')
						p++;
					
					response_code = atoi(p);
					first_line = FALSE;
					}
				else
					{	
					const char *colon = strchr(p, ':');
					// ignore bad header string that doesn't have a colon
					if (colon)
						{
						int width = colon - p;

						colon++; 
						while(*colon == ' ' || *colon == '\t' )
							colon++;

						CString label = CString( p, width);
						label.MakeLower();

						mime_map [label] = CString(colon);
						}
					}
				current_line.Empty();						
				break;
			default:
				current_line += *buffer;
				break;
			}
		buffer++; amount--;
		}
	return FALSE;
	}

CString GetInternetError()
	{
	DWORD error = GetLastError();
	char buffer[256];

	if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, error, 0, buffer, sizeof(buffer), NULL) == 0)
		{
		CString message;
		message.Format("Internet error %lu", error);
		return message;
		}

	return CString(buffer);
	}

BOOL IsTransientInternetError(DWORD error)
	{
	return error == ERROR_INTERNET_TIMEOUT ||
		error == ERROR_INTERNET_CANNOT_CONNECT ||
		error == ERROR_INTERNET_CONNECTION_ABORTED ||
		error == ERROR_INTERNET_CONNECTION_RESET ||
		error == ERROR_INTERNET_FORCE_RETRY;
	}

BOOL QueryInternetHeader(HINTERNET request, DWORD query, CString& value)
	{
	DWORD size = 0;
	DWORD index = 0;
	HttpQueryInfoW(request, query, NULL, &size, &index);

	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		return FALSE;

	LPWSTR buffer = new WCHAR[size / sizeof(WCHAR)];
	index = 0;
	BOOL result = HttpQueryInfoW(request, query, buffer, &size, &index);

	if (result)
		value = CString(buffer);

	delete [] buffer;
	return result;
	}

UINT WinInetWorkerThread( LPVOID lparam )
	{
	CProtocolHTTP *protocol = (CProtocolHTTP *)lparam;
	CDynamicLoad *dlobject = protocol->m_load_object;
	HINTERNET session = NULL;
	HINTERNET connection = NULL;
	HINTERNET request = NULL;
	CString hostname;
	CString progress;
	CString error_message;
	CString url;
	CString post_headers;
	CString post_data;
	CString agent_name;
	CString header_value;
	CStringW wide_url;
	CStringW wide_hostname;
	CStringW request_path;
	CStringW wide_agent_name;
	CStringW proxy;
	CStringW wide_post_headers;
	CMapStringToString mime_map;
	URL_COMPONENTSW components;
	METHOD_TYPE method;
	LOAD_STATE load_state = LOAD_STATE_LOADING;
	DWORD access_type = INTERNET_OPEN_TYPE_PRECONFIG;
	LPCWSTR proxy_name = NULL;
	LPCWSTR verb = L"GET";
	LPCWSTR accept_types[] = { L"*/*", NULL };
	DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE |
		INTERNET_FLAG_PRAGMA_NOCACHE;
	LPVOID post_buffer = NULL;
	DWORD post_length = 0;
	DWORD status_code = 0;
	DWORD status_size = sizeof(status_code);
	DWORD final_url_size = 0;
	DWORD amount_read = 0;
	DWORD request_attempt = 0;
	DWORD connect_retries = 0;
	DWORD timeout = 0;
	DWORD header_index = 0;
	BOOL http_decoding = TRUE;
	INT32 total_bytes_loaded = 0;
	BYTE buffer[BUFFER_SIZE];
	UINT error_id = IDS_HTTP_CONNECT_FAILED;
	UINT result = 1;

	dlobject->DEBUG_LOCK();
	url = dlobject->GetURL();
	method = dlobject->GetMethodType();
	post_headers = dlobject->GetPostHeaders();
	post_data = dlobject->GetPostData();
	dlobject->Unlock();

	wide_url = url;
	memset(&components, 0, sizeof(components));
	components.dwStructSize = sizeof(components);
	components.dwHostNameLength = (DWORD)-1;
	components.dwUrlPathLength = (DWORD)-1;
	components.dwExtraInfoLength = (DWORD)-1;

	if (!InternetCrackUrlW(wide_url, 0, 0, &components))
		{
		error_message = GetInternetError();
		goto error_exit;
		}

	wide_hostname = CStringW(components.lpszHostName, components.dwHostNameLength);
	request_path = CStringW(components.lpszUrlPath, components.dwUrlPathLength);
	if (components.dwExtraInfoLength)
		request_path += CStringW(components.lpszExtraInfo, components.dwExtraInfoLength);
	if (request_path.IsEmpty())
		request_path = L"/";

	hostname = CString(wide_hostname);
	progress.Format("Contacting host %s.", hostname);
	dlobject->DEBUG_LOCK();
	dlobject->SetProgressMessage(progress);
	dlobject->Unlock();

	agent_name.LoadString(IDS_HTTP_AGENT_NAME);
	wide_agent_name = agent_name;

	if (!theApp.m_HttpProxy.IsEmpty())
		{
		proxy.Format(L"%S:%u", (LPCSTR)theApp.m_HttpProxy,
			theApp.m_HttpProxyPort);
		access_type = INTERNET_OPEN_TYPE_PROXY;
		proxy_name = proxy;
		}

open_session:
	session = InternetOpenW(wide_agent_name, access_type, proxy_name,
		NULL, 0);
	if (!session)
		{
		error_message = GetInternetError();
		goto error_exit;
		}

	if (!protocol->SetSession(session))
		{
		InternetCloseHandle(session);
		session = NULL;
		goto aborted;
		}

	timeout = 10000;
	if (!InternetSetOption(session, INTERNET_OPTION_CONNECT_TIMEOUT,
		&timeout, sizeof(timeout)))
		{
		error_message = GetInternetError();
		goto error_exit;
		}
	timeout = 8000;
	if (!InternetSetOption(session, INTERNET_OPTION_RECEIVE_TIMEOUT,
		&timeout, sizeof(timeout)))
		{
		error_message = GetInternetError();
		goto error_exit;
		}
	if (!InternetSetOption(session, INTERNET_OPTION_CONNECT_RETRIES,
		&connect_retries, sizeof(connect_retries)))
		{
		error_message = GetInternetError();
		goto error_exit;
		}

	request_attempt++;
	error_id = IDS_HTTP_SEND_FAILED;
	connection = InternetConnectW(session, wide_hostname,
		components.nPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	if (!connection)
		{
		error_message = GetInternetError();
		goto error_exit;
		}

	if (method == METHOD_HEAD)
		verb = L"HEAD";
	else if (method == METHOD_POST)
		verb = L"POST";

	if (components.nScheme == INTERNET_SCHEME_HTTPS)
		flags |= INTERNET_FLAG_SECURE;

	request = HttpOpenRequestW(connection, verb, request_path, NULL,
		NULL, accept_types, flags, 0);
	if (!request)
		{
		error_message = GetInternetError();
		goto error_exit;
		}

	if (!InternetSetOption(request, INTERNET_OPTION_HTTP_DECODING,
		&http_decoding, sizeof(http_decoding)))
		{
		error_message = GetInternetError();
		goto error_exit;
		}
	if (!HttpAddRequestHeadersW(request,
		L"Accept-Encoding: gzip, deflate\r\n", (DWORD)-1,
		HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE))
		{
		error_message = GetInternetError();
		goto error_exit;
		}

	progress.Format("Host %s contacted. Sending HTTP command.", hostname);
	dlobject->DEBUG_LOCK();
	dlobject->SetProgressMessage(progress);
	dlobject->Unlock();

	wide_post_headers = post_headers;
	post_buffer = post_data.IsEmpty() ? NULL :
		(LPVOID)(LPCSTR)post_data;
	post_length = post_data.GetLength();

	if (!HttpSendRequestW(request,
		wide_post_headers.IsEmpty() ? NULL :
			(LPCWSTR)wide_post_headers,
		wide_post_headers.IsEmpty() ? 0 : (DWORD)-1,
		post_buffer, post_length))
		{
		DWORD request_error = GetLastError();
		if (method == METHOD_GET && request_attempt < 3 &&
			IsTransientInternetError(request_error))
			{
			TRACE("Internet error %lu for %s; retrying request %lu.\n",
				request_error, (LPCSTR)url, request_attempt + 1);
			InternetCloseHandle(request);
			request = NULL;
			InternetCloseHandle(connection);
			connection = NULL;
			protocol->CloseSession(session);
			session = NULL;
			Sleep(250 * request_attempt);
			goto open_session;
			}
		SetLastError(request_error);
		error_message = GetInternetError();
		goto error_exit;
		}

	header_index = 0;
	if (!HttpQueryInfoW(request,
		HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
		&status_code, &status_size, &header_index))
		{
		error_message = GetInternetError();
		goto error_exit;
		}

	if (method == METHOD_GET && request_attempt < 3 &&
		(status_code == 429 || status_code == 500 || status_code == 502 ||
		status_code == 503 || status_code == 504))
		{
		TRACE("HTTP status %lu for %s; retrying request %lu.\n",
			status_code, (LPCSTR)url, request_attempt + 1);
		InternetCloseHandle(request);
		request = NULL;
		if (connection)
			{
			InternetCloseHandle(connection);
			connection = NULL;
			}
		protocol->CloseSession(session);
		session = NULL;
		Sleep(250 * request_attempt);
		goto open_session;
		}

	InternetQueryOptionW(request, INTERNET_OPTION_URL, NULL, &final_url_size);
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
		LPWSTR final_url = new WCHAR[final_url_size / sizeof(WCHAR)];
		if (InternetQueryOptionW(request, INTERNET_OPTION_URL,
			final_url, &final_url_size))
			{
			CString final_url_string(final_url);
			if (final_url_string != url)
				{
				dlobject->DEBUG_LOCK();
				protocol->SetURL(final_url_string);
				dlobject->Unlock();
				}
			}
		delete [] final_url;
		}

	if (status_code < 200 || status_code > 299)
		{
		switch(status_code)
			{
			case 400:
				error_id = IDS_HTTP_ERROR_400;
				break;
			case 401:
				error_id = IDS_HTTP_ERROR_401;
				break;
			case 403:
				error_id = IDS_HTTP_ERROR_403;
				break;
			case 404:
				error_id = IDS_HTTP_ERROR_404;
				break;
			case 500:
				error_id = IDS_HTTP_ERROR_500;
				break;
			case 501:
				error_id = IDS_HTTP_ERROR_501;
				break;
			case 502:
				error_id = IDS_HTTP_ERROR_502;
				break;
			case 503:
				error_id = IDS_HTTP_ERROR_503;
				break;
			default:
				error_id = IDS_HTTP_ERROR_UNKNOWN;
				error_message.Format("%lu", status_code);
				break;
			}
		goto http_error;
		}

	if (QueryInternetHeader(request, HTTP_QUERY_CONTENT_TYPE, header_value))
		{
		int separator = header_value.Find(';');
		if (separator != -1)
			header_value = header_value.Left(separator);
		header_value.TrimLeft();
		header_value.TrimRight();
		header_value.MakeLower();
		mime_map["content-type"] = header_value;
		}
	else
		{
		mime_map["content-type"] = "unknown";
		}

	if (QueryInternetHeader(request, HTTP_QUERY_CONTENT_LENGTH, header_value))
		mime_map["content-length"] = header_value;

	load_state = protocol->CallOnBeginLoading(mime_map);
	if (load_state == LOAD_STATE_COMPLETE || load_state == LOAD_STATE_ABORTED)
		goto done_loading;

	error_id = IDS_HTTP_RECV_FAILED;

	while(1)
		{
		amount_read = 0;
		if (!InternetReadFile(request, buffer, sizeof(buffer), &amount_read))
			{
			DWORD read_error = GetLastError();
			TRACE("Internet read failed for %s after %ld bytes: %lu.\n",
				(LPCSTR)url, total_bytes_loaded, read_error);
			SetLastError(read_error);
			error_message = GetInternetError();
			goto error_exit;
			}

		if (amount_read == 0)
			break;

		total_bytes_loaded += amount_read;
		progress.Format("%ld bytes received from %s",
			total_bytes_loaded, hostname);
		dlobject->DEBUG_LOCK();
		dlobject->SetProgressMessage(progress);
		dlobject->Unlock();

		load_state = protocol->CallOnLoading(buffer, amount_read);
		if (load_state == LOAD_STATE_COMPLETE ||
			load_state == LOAD_STATE_ABORTED)
			break;
		}

done_loading:
	load_state = protocol->CallOnEndLoading();
	progress.Format("Connection to %s closed.", hostname);
	dlobject->DEBUG_LOCK();
	protocol->SetLoadState(load_state);
	protocol->CallNotify(CHANGEFLAG_DONE);
	dlobject->SetProgressMessage(progress);
	dlobject->Unlock();
	result = 0;
	goto cleanup;

http_error:
	dlobject->DEBUG_LOCK();
	if (error_id == IDS_HTTP_ERROR_UNKNOWN)
		dlobject->SetErrorMessage(error_id, hostname, error_message);
	else
		dlobject->SetErrorMessage(error_id, hostname);
	dlobject->Unlock();
	goto fail_loading;

error_exit:
	if (protocol->IsAborted())
		goto aborted;

	dlobject->DEBUG_LOCK();
	dlobject->SetErrorMessage(error_id, hostname, error_message);
	dlobject->Unlock();

fail_loading:
	dlobject->DEBUG_LOCK();
	protocol->SetLoadState(LOAD_STATE_ABORTED);
	dlobject->Unlock();
	protocol->CallOnEndLoading();
	dlobject->DEBUG_LOCK();
	protocol->CallNotify(CHANGEFLAG_DONE);
	dlobject->Unlock();
	goto cleanup;

aborted:
	result = 1;

	dlobject->DEBUG_LOCK();
	protocol->SetLoadState(LOAD_STATE_ABORTED);
	dlobject->Unlock();

	protocol->CallOnEndLoading();

	dlobject->DEBUG_LOCK();
	protocol->CallNotify( CHANGEFLAG_DONE );
	dlobject->Unlock();

cleanup:
	if (request)
		InternetCloseHandle(request);
	if (connection)
		InternetCloseHandle(connection);
	if (session)
		protocol->CloseSession(session);
	ReleaseSemaphore(protocol->m_thread_done_semaphore, 1, NULL);
	return result;
	}

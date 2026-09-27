#include "cross_p.h"
#include "resource.h"

#include "viewhtml.h"

#ifndef _DYNLOAD_H_
#include "dynload.h"
#endif

#ifndef _PROTOCOL_H_
#include "protocol.h"
#endif

#define WM_HTTP_SOCKET_EVENT 	(WM_USER)
#define WM_HTTP_ABORT			(WM_USER+1)



#define BUFFER_SIZE 4096

CProtocol::CProtocol(CDynamicLoad *load_object)
	{
	m_load_object = load_object;

	m_load_object->DEBUG_LOCK();
	m_url = m_load_object->GetURL();
	m_load_object->Unlock();

	m_thread_handle = NULL;
	}

void CProtocol::SetLoadState( LOAD_STATE l )
	{
	ASSERT_LOCKED(m_load_object);

	m_load_object->m_load_state = l;
	}

void CProtocol::SetURL( LPCSTR url )
	{
	ASSERT_LOCKED(m_load_object);
	
	m_load_object->m_url = url;
	m_url = url;
	}

void CProtocol::OpenProtocolManager()
	{
	VERIFY( CThreadSocket::Initialize() );
    }

void CProtocol::CloseProtocolManager()
	{
	CThreadSocket::Terminate();
	}

LOAD_STATE CProtocol::CallOnEndLoading()
	{
	return m_load_object->OnEndLoading();
	}

LOAD_STATE CProtocol::CallOnBeginLoading( const CMapStringToString& map )
	{
	return m_load_object->OnBeginLoading(map);
	}

LOAD_STATE CProtocol::CallOnLoading(LPCBYTE buffer, INT32 length)
	{
	return m_load_object->OnLoading(buffer, length);
	}

LOAD_STATE CProtocol::CallOnPreLoading()
	{
	return m_load_object->OnPreLoading();
	}
	
void CProtocol::CallNotify(UINT32 flags)
	{
	ASSERT_LOCKED(m_load_object);

	m_load_object->Notify(flags);
	}

CProtocol::~CProtocol()
	{
	ASSERT(m_thread_handle == NULL);
	}

//////////////////////////////
/// File protocol

CString ConvertFilenameToURL( LPCSTR filename)
	{
	CString str( "file:///" );

	char buffer[256];

	GetFullPathName( filename, sizeof(buffer), buffer, NULL);

	char *p = buffer;
	BYTE b;

	while(*p)
		{
		switch(*p)
			{
			case '=':
			case ';':
			case ' ':
			case '/':
			case '#':
			case '?':
			case '%':
				b = (BYTE)*p;

				str += '%';
				str += (char)('0' + (b>>4));
				str += (char)('0' + (b&0xf));
				break;
			case ':':
				str += '|';
				break;			
			case '\\':
				str += '/';
				break;
			default:
				str += *p;
				break;
			}
		p++;
		}
	return str;
	}


CProtocolFile::CProtocolFile(CDynamicLoad *load_object)
	: CProtocol(load_object )
	{
	m_bTerminateThread = FALSE;
	m_thread_handle = NULL;
	m_thread_done_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
	}

CProtocolFile::~CProtocolFile()
	{
	ASSERT(m_thread_handle == NULL);

	CloseHandle(m_thread_done_semaphore);
	}

UINT FileWorkerThread( LPVOID lparam )
	{
	CProtocolFile *protocol = (CProtocolFile *)lparam;
	CDynamicLoad *dlobject = protocol->m_load_object;

	CString filename;
		
	TRY
		{
		dlobject->DEBUG_LOCK();
		CString url = dlobject->GetURL();
	
		CFileException e;
		const char *p = ((const char *)url);
		
		if (dlobject->GetMethodType() != METHOD_GET)
			{
			dlobject->SetErrorMessage(IDS_METHOD_TYPE_BAD, "file");
	error_exit:
			protocol->SetLoadState(LOAD_STATE_ABORTED);
			dlobject->Unlock();
			protocol->CallOnEndLoading();
			dlobject->DEBUG_LOCK();
			protocol->CallNotify( CHANGEFLAG_DONE );
			dlobject->Unlock();
			ReleaseSemaphore(protocol->m_thread_done_semaphore,1,NULL);
			return 1;
			}

		dlobject->Unlock();

		if (memicmp(url, "file:///", 8) != 0)
			{
			dlobject->DEBUG_LOCK();
			dlobject->SetErrorMessage(IDS_BAD_URL, url );
			goto error_exit;
			}

		p+=8;

		if (strchr(p,'|') == NULL)
			filename += '\\';		// if filename does not contain a drive letter, make it absolute

		while(*p)
			{
			if (*p == '%')
				{
				char str[3] = { *(p+1), *(p+2), 0 };
				char *p;
		
				// convert from base 16
				long l = strtol( str, &p, 16);
				if (l != 0)
					filename += (char)l;

				p+=3;
				}
			else if (*p == '|')
				{
				filename += ':';
				p++;
				}
			else if (*p == '/')
				{
				filename += '\\';
				p++;
				}
			else
				{
				filename += *p;
				p++;
				}			
			}


		CString progress;
		progress.Format("Opening file %s", filename);
		
		dlobject->DEBUG_LOCK();
		dlobject->SetProgressMessage(progress);
		dlobject->Unlock();

		CFile file;

		if (file.Open( filename, CFile::modeRead, &e)==0)
			{
			dlobject->DEBUG_LOCK();
			dlobject->SetErrorMessage(IDS_FILE_CANNOT_OPEN, filename);
			goto error_exit;
			}

		// construct the MIME header
	
		CMapStringToString map;

		const char *extension = strrchr( filename, '.');

		CString mime_type;

		if (extension == NULL)
			{
			mime_type = "unknown";
			}
		else if (stricmp(extension, ".gif")==0)
			{
			mime_type = "image/gif";
			}
		else if (stricmp(extension, ".htm") ==0 ||
				 stricmp(extension, ".html") == 0)
			{
			mime_type = "text/html";
			}
		else if (stricmp(extension, ".txt") == 0)
			{
			mime_type = "text/plain";
			}
		else if (stricmp(extension, ".jpg") == 0 ||
				 stricmp(extension, ".jpeg") == 0)
			{
			mime_type = "image/jpeg";
			}
		else
			{
			mime_type = "unknown";
			}

		map[ "content-type" ] = mime_type;

		CString filesize;

		filesize.Format("%ld", file.GetLength() );

		map[ "content-length" ] = filesize;

		LOAD_STATE l = protocol->CallOnBeginLoading( map );
		
		if (l == LOAD_STATE_COMPLETE || l == LOAD_STATE_ABORTED)
			{
			dlobject->DEBUG_LOCK();
			protocol->SetLoadState(l);
			dlobject->Unlock();

			l = protocol->CallOnEndLoading();
			
			dlobject->DEBUG_LOCK();
			protocol->SetLoadState(l);
			protocol->CallNotify( CHANGEFLAG_DONE );
			dlobject->Unlock();
			ReleaseSemaphore(protocol->m_thread_done_semaphore,1,NULL);
			return 0;
			}

		BYTE buffer[ BUFFER_SIZE ];

		UINT total_amount =0;

		while( !protocol->m_bTerminateThread )
			{
			UINT amount = file.Read(buffer, BUFFER_SIZE );

			total_amount += amount;
			progress.Format("%u bytes read from file %s", total_amount, filename);
		
			dlobject->DEBUG_LOCK();
			dlobject->SetProgressMessage(progress);
			dlobject->Unlock();

			l = protocol->CallOnLoading(buffer, amount );
			
			if (l == LOAD_STATE_COMPLETE || l == LOAD_STATE_ABORTED)
				{
				dlobject->DEBUG_LOCK();
				protocol->SetLoadState(l);
				dlobject->Unlock();

				l =  protocol->CallOnEndLoading();

				dlobject->DEBUG_LOCK();
				protocol->SetLoadState(l);
				protocol->CallNotify( CHANGEFLAG_DONE );
				dlobject->Unlock();
				ReleaseSemaphore(protocol->m_thread_done_semaphore,1,NULL);
				return 0;
				}
			if (amount != BUFFER_SIZE)
				{
				l = protocol->CallOnEndLoading();

				dlobject->DEBUG_LOCK();
				protocol->SetLoadState( l );
				protocol->CallNotify( CHANGEFLAG_DONE );
				dlobject->Unlock();
				ReleaseSemaphore(protocol->m_thread_done_semaphore,1,NULL);
				return 0;
				}
			}
		// early termination has been requested, so terminate
		ReleaseSemaphore(protocol->m_thread_done_semaphore,1,NULL);
		return 3;
		}
	CATCH(CFileException, e)
		{
		dlobject->DEBUG_LOCK();
		dlobject->SetErrorMessage(IDS_FILE_EXCEPTION, filename);
		protocol->SetLoadState( LOAD_STATE_ABORTED );
		dlobject->Unlock();

		protocol->CallOnEndLoading();

		dlobject->DEBUG_LOCK();
		protocol->CallNotify( CHANGEFLAG_DONE );
		dlobject->Unlock();
		ReleaseSemaphore(protocol->m_thread_done_semaphore,1,NULL);
		return 1;
		}
	END_CATCH
	}

void CProtocolFile::BeginLoadThread()
	{
	m_thread_handle = AfxBeginThread( FileWorkerThread, (LPVOID)this );
	}

void CProtocolFile::AbortLoadThread()
	{
	m_bTerminateThread = TRUE;
	WaitForSingleObject( m_thread_done_semaphore, INFINITE );
	m_thread_handle = NULL;
	}

void CProtocolFile::WaitEndLoadThread()
	{
	WaitForSingleObject( m_thread_done_semaphore, INFINITE );
	m_thread_handle = NULL;
	}

///////////////////////////////////////////////////////////
// CProtocolHTTP 

CProtocolHTTP::CProtocolHTTP(CDynamicLoad *load_object)
	: CProtocol(load_object )
	{
	m_thread_handle = NULL;
	m_thread_done_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
	}

CProtocolHTTP::~CProtocolHTTP()
	{
	ASSERT(m_thread_handle == NULL);
	CloseHandle(m_thread_done_semaphore);
	}

void CProtocolHTTP::BeginLoadThread()
	{
	m_thread_handle = AfxBeginThread( HTTPWorkerThread, (LPVOID)this );
	}

void CProtocolHTTP::AbortLoadThread()
	{
	ASSERT(m_thread_handle);
	
	// do everything possible to get the thread to teminate as quickly as possible

	m_socket.Abort();

	WaitForSingleObject( m_thread_done_semaphore, INFINITE );
	m_thread_handle = NULL;
	}

void CProtocolHTTP::WaitEndLoadThread()
	{
	ASSERT(m_thread_handle);
	WaitForSingleObject( m_thread_done_semaphore, INFINITE );
	m_thread_handle = NULL;
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

#define HTTP_STATE_READING_HEADER   1
#define HTTP_STATE_READING_DATA 	2

UINT HTTPWorkerThread( LPVOID lparam )
	{
	CProtocolHTTP *protocol = (CProtocolHTTP *)lparam;
	CDynamicLoad *dlobject = protocol->m_load_object;

	CString e;
	
	BOOL first_line = TRUE;

	dlobject->DEBUG_LOCK();
	CString url = dlobject->GetURL();
	dlobject->Unlock();


	CString hostname, getrequest, hostdesc;
	int portnumber;
	
	// parse the url into the hostname and the request string

	const char *p = url;

	ASSERT(memicmp(p, "http://", 7)==0);
	
	p+=7;		// skip "http://"


	int len;
	hostname = CString(p, len = strcspn(p, ":/"));

	portnumber = 80;
	p+=len;
	if (*p == ':')			// parse port number
		{
		portnumber = atoi(p+1);
		}

	p = strstr(url, "//");
	
	ASSERT(p);

	p = strchr(p+2, '/');
	if (p == NULL)	// if there is no directory tacked on the end of the url, use /
		p = "/";

	hostdesc = hostname;
	getrequest = p;

	if (!theApp.m_HttpProxy.IsEmpty())
		{
		// For proxy requests, just send the URL straight to the proxy server

		hostdesc = hostname + " via " + theApp.m_HttpProxy;
		hostname = theApp.m_HttpProxy;
		portnumber = theApp.m_HttpProxyPort;
		getrequest = url;
		}

	CString progress;
	progress.Format("Contacting host %s.", hostdesc);

	dlobject->DEBUG_LOCK();
	dlobject->SetProgressMessage(progress);
	dlobject->Unlock();

	SOCKET_RESULT r = protocol->m_socket.Connect(hostname, portnumber);
	if (r == SOCKET_RESULT_ERROR)
		{
		dlobject->DEBUG_LOCK();
		dlobject->SetErrorMessage(IDS_HTTP_CONNECT_FAILED, hostname, protocol->m_socket.GetErrorMessage() );
		dlobject->Unlock();
		}
	if (r != SOCKET_RESULT_OK)
		{
error_exit:
		dlobject->DEBUG_LOCK();
		protocol->SetLoadState(LOAD_STATE_ABORTED);
		dlobject->Unlock();

		protocol->CallOnEndLoading();
		dlobject->DEBUG_LOCK();
		protocol->CallNotify( CHANGEFLAG_DONE );
		dlobject->Unlock();

		ReleaseSemaphore(protocol->m_thread_done_semaphore,1,NULL);
		return 1;
		}
	
	progress.Format("Host %s contacted. Sending HTTP command.", hostname);

	dlobject->DEBUG_LOCK();
	dlobject->SetProgressMessage(progress);
	dlobject->Unlock();
			
	CString agent_name;
	agent_name.LoadString( IDS_HTTP_AGENT_NAME);
					
	CString getstring = "GET " + getrequest + " " + "HTTP/1.0\n\r" + "Accept: */*\n\r" + 
			"User-Agent: " + agent_name + "\n\r" + "\n\r";
	
	r = protocol->m_socket.Write((const char *)getstring, getstring.GetLength());
	if (r == SOCKET_RESULT_ERROR)
		{
		dlobject->DEBUG_LOCK();
		dlobject->SetErrorMessage(IDS_HTTP_SEND_FAILED, hostname, protocol->m_socket.GetErrorMessage() );
		dlobject->Unlock();
		}
	if (r != SOCKET_RESULT_OK)
		goto error_exit;
		
	progress.Format("Host %s contacted. Waiting for reply.", hostname);

	dlobject->DEBUG_LOCK();
	dlobject->SetProgressMessage(progress);
	dlobject->Unlock();

	BYTE receive_buffer[BUFFER_SIZE];
	LPBYTE buffer;
	INT32 amount;
	INT response_code;
	INT32 total_bytes_loaded = 0;

	CString current_line;
	CMapStringToString mime_map;
	int  socket_status = HTTP_STATE_READING_HEADER;
		
	while(1)
		{
		// loop receiving data
		LOAD_STATE l;

		INT32 amount_read;
		r = protocol->m_socket.Read(receive_buffer, BUFFER_SIZE, amount_read);
		if (r == SOCKET_RESULT_ERROR)
			{
			dlobject->DEBUG_LOCK();
			dlobject->SetErrorMessage(IDS_HTTP_RECV_FAILED, hostname, protocol->m_socket.GetErrorMessage() );
			dlobject->Unlock();
			}
		if (r != SOCKET_RESULT_OK)
			goto error_exit;

		if (amount_read == 0)
			{
			protocol->m_socket.Close();

			l = protocol->CallOnEndLoading();

			progress.Format("Connection to %s closed.", hostname);
			
			dlobject->DEBUG_LOCK();
			protocol->SetLoadState( l );
			protocol->CallNotify( CHANGEFLAG_DONE );
			dlobject->SetProgressMessage(progress);
			dlobject->Unlock();

			ReleaseSemaphore(protocol->m_thread_done_semaphore,1,NULL);
			return 0;
			// socket connection has been closed
			}

		total_bytes_loaded += amount_read;
		amount = amount_read;
		buffer = receive_buffer;

		progress.Format("%ld bytes received from %s", total_bytes_loaded, hostname);
		
		dlobject->DEBUG_LOCK();
		dlobject->SetProgressMessage(progress);
		dlobject->Unlock();

		if (socket_status == HTTP_STATE_READING_HEADER)
			{
			if (ParseHeader(first_line, buffer, amount, current_line, mime_map, response_code))
				{
				if (response_code >= 200 && response_code <= 299)
					{
					// everything went okay, no special handling is needed
					}
				else if (response_code >= 300 && response_code <= 399)
					{
					// we got a redirect
					url = mime_map[ "location" ];

					protocol->m_socket.Close();

					dlobject->DEBUG_LOCK();
					protocol->SetURL( url );
					protocol->CallNotify( CHANGEFLAG_URL_REDIRECT );
					dlobject->Unlock();
					ReleaseSemaphore(protocol->m_thread_done_semaphore,1,NULL);
					return 100;
					}
				else if (response_code >= 400 && response_code <= 599)
					{
					dlobject->DEBUG_LOCK();

					switch(response_code)
						{
						case 400:
							dlobject->SetErrorMessage(IDS_HTTP_ERROR_400, hostname);
							break;
						case 401:
							dlobject->SetErrorMessage(IDS_HTTP_ERROR_401, hostname);
							break;
						case 403:
							dlobject->SetErrorMessage(IDS_HTTP_ERROR_403, hostname);
							break;
						case 404:
							dlobject->SetErrorMessage(IDS_HTTP_ERROR_404, hostname);
							break;
						case 500:
							dlobject->SetErrorMessage(IDS_HTTP_ERROR_500, hostname);
							break;
						case 501:
							dlobject->SetErrorMessage(IDS_HTTP_ERROR_501, hostname);
							break;
						case 502:
							dlobject->SetErrorMessage(IDS_HTTP_ERROR_502, hostname);
							break;
						case 503:
							dlobject->SetErrorMessage(IDS_HTTP_ERROR_503, hostname);
							break;
						default:
								{
								CString f;
								f.Format("%d", (int)response_code);
								dlobject->SetErrorMessage(IDS_HTTP_ERROR_UNKNOWN, hostname, f);
								}
							break;
						}
					dlobject->Unlock();
		
					goto error_exit;
					}
			
				l = protocol->CallOnBeginLoading(mime_map);
				if (l == LOAD_STATE_COMPLETE || l == LOAD_STATE_ABORTED)
					goto done_complete;
					
				socket_status = HTTP_STATE_READING_DATA;
				}
			}

		if (socket_status == HTTP_STATE_READING_DATA)
			{
			l = protocol->CallOnLoading(buffer, amount );
			
			if (l == LOAD_STATE_COMPLETE || l == LOAD_STATE_ABORTED)
				{
done_complete:
				dlobject->DEBUG_LOCK();
				protocol->SetLoadState(l);
				dlobject->Unlock();
		
				l =  protocol->CallOnEndLoading();

				progress.Format("Connection to %s closed.", hostname);

				dlobject->DEBUG_LOCK();
				protocol->SetLoadState(l);
				protocol->CallNotify( CHANGEFLAG_DONE );
				dlobject->SetProgressMessage(progress);
				dlobject->Unlock();

				ReleaseSemaphore(protocol->m_thread_done_semaphore,1,NULL);
				return 0;
				}
			}
		}
	}
	

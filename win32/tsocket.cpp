#include "cross_p.h"
#include "resource.h"

#include "tsocket.h"

#define WM_SOCKET_EVENT 	(WM_USER)
#define WM_SOCKET_ABORT			(WM_USER+1)

const char g_window_class[] = "NullSocketWindowClass";

LRESULT CALLBACK NullWindowProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam)
	{
	return DefWindowProc(hWnd, message, wparam, lparam);
	}

BOOL CThreadSocket::Initialize()
	{
	WNDCLASS wndclass;

	memset(&wndclass, 0, sizeof(wndclass));

	wndclass.lpfnWndProc = NullWindowProc;
    wndclass.hInstance = AfxGetInstanceHandle();
	wndclass.lpszClassName = g_window_class;

	VERIFY( ::RegisterClass(&wndclass) );
	return TRUE;
    }

void CThreadSocket::Terminate()
	{
	::UnregisterClass(g_window_class, AfxGetInstanceHandle());
	}

CThreadSocket::CThreadSocket()
	{
	m_interrupt = FALSE;
	m_message_window = NULL;
	m_socket = NULL;
	}

CThreadSocket::~CThreadSocket()
	{
	ASSERT(m_socket == NULL);
	ASSERT(m_message_window == NULL);
	}

void CThreadSocket::Abort()
	{
	m_access_lock.DEBUG_LOCK();

	m_interrupt = TRUE;

	if (m_message_window)
		{
		PostMessage(m_message_window, WM_SOCKET_ABORT, 0, 0);
		}

	if (m_socket)
		{
		closesocket(m_socket);
		m_socket = NULL;
		}

	m_access_lock.Unlock();
	}

CString CThreadSocket::GetErrorMessage()
	{
	m_access_lock.DEBUG_LOCK();
	CString s = m_error_message;
	m_access_lock.Unlock();
	return s;
	}

SOCKET_RESULT CThreadSocket::Connect(LPCSTR hostname, INT port_number)
	{
	// create response window
	MSG window_message;

	HWND message_window = CreateWindow(g_window_class, NULL, WS_POPUP, 0,0,0,0,NULL,NULL,
		AfxGetInstanceHandle(), NULL);
	ASSERT(message_window);

	m_access_lock.DEBUG_LOCK();
	if (m_interrupt)
		{
		m_access_lock.Unlock();
		DestroyWindow(message_window);
		return SOCKET_RESULT_ABORTED;
		}
	m_message_window = message_window;
	m_access_lock.Unlock();

	ULONG ulIPAddress = inet_addr(hostname);
	char hostbuffer[MAXGETHOSTSTRUCT ];
	HANDLE task_handle;

	if (ulIPAddress == INADDR_NONE)
		task_handle = WSAAsyncGetHostByName( message_window, WM_SOCKET_EVENT,hostname,  
			hostbuffer, MAXGETHOSTSTRUCT);
	else
		task_handle = WSAAsyncGetHostByAddr( message_window, WM_SOCKET_EVENT, (char *)&ulIPAddress,4, PF_INET,
			hostbuffer, MAXGETHOSTSTRUCT);

	if (task_handle == NULL)
		{
		int last_error = WSAGetLastError();
		DestroyWindow(message_window);

		CString message;

		switch(last_error)	
			{
			case WSANOTINITIALISED:
				message.LoadString(IDS_WSANOTINITIALISED);
				break;
			case WSAENETDOWN:
				message.LoadString(IDS_WSAENETDOWN);
				break;
			case WSAEINPROGRESS:
				message.LoadString(IDS_WSAEINPROGRESS);
				break;
			case WSAEWOULDBLOCK:
				message.LoadString(IDS_WSAEWOULDBLOCK);
				break;
			default:
				message.Format("Unknown error %d", last_error);
				break;
			}
		m_access_lock.DEBUG_LOCK();
		m_error_message = message;
		m_access_lock.Unlock();
		return SOCKET_RESULT_ERROR;
		}

	while(1)
		{
		VERIFY( GetMessage(&window_message, NULL, 0,0 ) );

		if (window_message.message == WM_SOCKET_ABORT)
			{
			WSACancelAsyncRequest(task_handle);
			m_access_lock.DEBUG_LOCK();
			m_message_window = NULL;
			m_access_lock.Unlock();
			DestroyWindow(m_message_window);
			return SOCKET_RESULT_ABORTED;
			}
		else if (window_message.message == WM_SOCKET_EVENT)
			{
			m_access_lock.DEBUG_LOCK();
			m_message_window = NULL;
			m_access_lock.Unlock();
			DestroyWindow(message_window);

			int error_code = WSAGETASYNCERROR(window_message.lParam);
			if (error_code == 0)
				break;

			CString message;

			switch(error_code)	
				{
				case WSAENETDOWN:
					message.LoadString(IDS_WSAENETDOWN);
					break;
				case WSAHOST_NOT_FOUND:
					message.LoadString(IDS_DNS_WSAHOST_NOT_FOUND);
					break;
				case WSAENOBUFS:
					message.LoadString(IDS_DNS_WSAENOBUFS);
					break;
				case WSATRY_AGAIN:
					message.LoadString(IDS_DNS_WSATRY_AGAIN);
					break;
				case WSANO_RECOVERY:
					message.LoadString(IDS_DNS_WSANO_RECOVERY);
					break;
				case WSANO_DATA:
					message.LoadString(IDS_DNS_WSANO_DATA);
					break;
				default:
					message.Format("Unknown DNS error %d", error_code);
					break;
				}

			m_access_lock.DEBUG_LOCK();
			m_error_message = message;
			m_access_lock.Unlock();
			return SOCKET_RESULT_ERROR;
			}
		}

	SOCKET hSocket;
	
	SOCKADDR_IN sinLocal;
	SOCKADDR_IN sinRemote;

	sinLocal.sin_family = AF_INET;
	sinLocal.sin_port = 0;
	sinLocal.sin_addr.s_addr = htonl(INADDR_ANY);

	sinRemote.sin_family = AF_INET;
	sinRemote.sin_port = htons(port_number);
	sinRemote.sin_addr.s_addr = *(ULONG *)(((struct hostent FAR *)(hostbuffer))->h_addr);

	m_access_lock.DEBUG_LOCK();
	if (m_interrupt)
		{
		m_access_lock.Unlock();
		return SOCKET_RESULT_ABORTED;
		}

	hSocket = socket(PF_INET, SOCK_STREAM, 0);
	
	if (hSocket == INVALID_SOCKET)
		{
		int error_code = WSAGetLastError();
		
		CString e;
		e.Format("%d", (int)error_code);

		m_error_message.Format("Create socket failed (error %d)", error_code);
		m_access_lock.Unlock();
		return SOCKET_RESULT_ERROR;
		}

	if (bind(hSocket, (LPSOCKADDR)&sinLocal, sizeof(SOCKADDR_IN)) == SOCKET_ERROR )
		{
		int error_code = WSAGetLastError();
		
		CString e;
		e.Format("%d", (int)error_code);

		m_error_message.Format("Bind socket failed (error %d)", error_code);
		m_access_lock.Unlock();
		return SOCKET_RESULT_ERROR;
		}

	m_socket = hSocket;
	m_access_lock.Unlock();

	if (connect(hSocket, (LPSOCKADDR)&sinRemote, sizeof(SOCKADDR_IN)) == SOCKET_ERROR )
		{
		m_access_lock.DEBUG_LOCK();
		if (m_interrupt)
			{
			ASSERT(!m_socket);
			m_access_lock.Unlock();
			return SOCKET_RESULT_ABORTED;
			}
		m_access_lock.Unlock();
		
		int error_code = WSAGetLastError();
		CString message;

		switch(error_code)
		 	{
			case WSANOTINITIALISED:
				message.LoadString(IDS_WSANOTINITIALISED);
				break;
			case WSAENETDOWN:
				message.LoadString(IDS_WSAENETDOWN);
				break;
			case WSAEINPROGRESS:
				message.LoadString(IDS_WSAEINPROGRESS);
				break;
			case WSAENOTSOCK:
				message.LoadString(IDS_WSAENOTSOCK);
				break;
			case WSAEWOULDBLOCK:
				message.LoadString(IDS_WSAEWOULDBLOCK);
				break;
			case WSAEADDRINUSE:
				message.LoadString(IDS_CON_WSAEADDRINUSE);
				break;
			case WSAEADDRNOTAVAIL:
				message.LoadString(IDS_CON_WSAEADDRNOTAVAIL);
				break;
			case WSAEAFNOSUPPORT:
				message.LoadString(IDS_CON_WSAEAFNOSUPPORT);
				break;
			case WSAECONNREFUSED:
				message.LoadString(IDS_CON_WSAECONNREFUSED);
				break;
			case WSAEFAULT:
				message.LoadString(IDS_CON_WSAEFAULT);
				break;
			case WSAEINVAL:
				message.LoadString(IDS_CON_WSAEINVAL);
				break;
			case WSAEISCONN:
				message.LoadString(IDS_CON_WSAEISCONN);
				break;
			case WSAEMFILE:
				message.LoadString(IDS_CON_WSAEMFILE);
				break;
			case WSAENETUNREACH:
				message.LoadString(IDS_CON_WSAENETUNREACH);
				break;
			case WSAENOBUFS:
				message.LoadString(IDS_CON_WSAENOBUFS);
				break;
			case WSAETIMEDOUT:
				message.LoadString(IDS_CON_WSAETIMEDOUT);
				break;
			default:
				message.Format("Unknown connect error %d", error_code);
				break;
			}
		
		m_access_lock.DEBUG_LOCK();
		if (m_interrupt)
			{
			m_access_lock.Unlock();
			return SOCKET_RESULT_ABORTED;
			}
		m_socket = NULL;
		m_error_message = message;
		m_access_lock.Unlock();
		
		closesocket(hSocket);
		return SOCKET_RESULT_ERROR;
		}

	return SOCKET_RESULT_OK;
	}

SOCKET_RESULT CThreadSocket::Write(LPCVOID buffer, INT32 amount)
	{
	m_access_lock.DEBUG_LOCK();
	SOCKET s = m_socket;
	if (s == NULL)
		{
		m_access_lock.Unlock();
		return SOCKET_RESULT_ABORTED;
		}
	m_access_lock.Unlock();

	if (send(s, (const char *)buffer, amount, 0) == SOCKET_ERROR)
		{
		m_access_lock.DEBUG_LOCK();
		if (m_interrupt)
			{
			ASSERT(!m_socket);
			m_access_lock.Unlock();
			return SOCKET_RESULT_ABORTED;
			}
		
		int error_code = WSAGetLastError();

		SOCKET hSocket = m_socket;
		m_socket = NULL;
		m_access_lock.Unlock();

		closesocket(hSocket);

		CString message;
		
		switch(error_code)
		 	{
			case WSANOTINITIALISED:
				message.LoadString(IDS_WSANOTINITIALISED);
				break;
			case WSAENETDOWN:
				message.LoadString(IDS_WSAENETDOWN);
				break;
			case WSAEINPROGRESS:
				message.LoadString(IDS_WSAEINPROGRESS);
				break;
			case WSAENOTSOCK:
				message.LoadString(IDS_WSAENOTSOCK);
				break;
			case WSAEWOULDBLOCK:
				message.LoadString(IDS_WSAEWOULDBLOCK);
				break;
			case WSAEACCES:
				message.LoadString(IDS_SEND_WSAEACCES);
				break;
			case WSAEFAULT:
				message.LoadString(IDS_SEND_WSAEFAULT);
				break;
			case WSAENETRESET:
				message.LoadString(IDS_SEND_WSAENETRESET);
				break;
			case WSAENOBUFS:
				message.LoadString(IDS_SEND_WSAENOBUFS);
				break;
			case WSAENOTCONN:
				message.LoadString(IDS_SEND_WSAENOTCONN);
				break;
			case WSAEOPNOTSUPP:
				message.LoadString(IDS_SEND_WSAEOPNOTSUPP);
				break;
			case WSAESHUTDOWN:
				message.LoadString(IDS_SEND_WSAESHUTDOWN);
				break;
			case WSAEMSGSIZE:
				message.LoadString(IDS_SEND_WSAEMSGSIZE);
				break;
			case WSAEINVAL:
				message.LoadString(IDS_SEND_WSAEINVAL);
				break;
			case WSAECONNABORTED:
				message.LoadString(IDS_SEND_WSAECONNABORTED);
				break;
			case WSAECONNRESET:
				message.LoadString(IDS_SEND_WSAECONNRESET);
				break;
			default:
				message.Format("Unknown send error %d", error_code);
				break;
			}
		
		m_access_lock.DEBUG_LOCK();
		m_error_message = message;
		m_access_lock.Unlock();
		
		return SOCKET_RESULT_ERROR;
		}
	else
		{
		return SOCKET_RESULT_OK;
		}
	}

SOCKET_RESULT CThreadSocket::Read(LPVOID buffer, INT32 buffer_size, INT32& amount_read)
	{
	m_access_lock.DEBUG_LOCK();
	SOCKET s = m_socket;
	if (s == NULL)
		{
		m_access_lock.Unlock();
		return SOCKET_RESULT_ABORTED;
		}
	m_access_lock.Unlock();

	int r = recv(s, (char *)buffer, buffer_size, 0);

	if (r == SOCKET_ERROR)
		{
		m_access_lock.DEBUG_LOCK();
		if (m_interrupt)
			{
			ASSERT(!m_socket);
			m_access_lock.Unlock();
			return SOCKET_RESULT_ABORTED;
			}
		
		int error_code = WSAGetLastError();

		SOCKET hSocket = m_socket;
		m_socket = NULL;
		m_access_lock.Unlock();

		CString message;
		
		closesocket(hSocket);
		
		switch(error_code)
		 	{
			case WSANOTINITIALISED:
				message.LoadString(IDS_WSANOTINITIALISED);
				break;
			case WSAENETDOWN:
				message.LoadString(IDS_WSAENETDOWN);
				break;
			case WSAEINPROGRESS:
				message.LoadString(IDS_WSAEINPROGRESS);
				break;
			case WSAENOTSOCK:
				message.LoadString(IDS_WSAENOTSOCK);
				break;
			case WSAEWOULDBLOCK:
				message.LoadString(IDS_WSAEWOULDBLOCK);
				break;
			case WSAENOTCONN:
				message.LoadString(IDS_SEND_WSAENOTCONN);
				break;
			case WSAESHUTDOWN:
				message.LoadString(IDS_SEND_WSAESHUTDOWN);
				break;
			case WSAEOPNOTSUPP:
				message.LoadString(IDS_SEND_WSAEOPNOTSUPP);
				break;
			case WSAEMSGSIZE:
				message.LoadString(IDS_RECV_WSAEMSGSIZE);
				break;
			case WSAECONNABORTED:
				message.LoadString(IDS_SEND_WSAECONNABORTED);
				break;
			case WSAECONNRESET:
				message.LoadString(IDS_SEND_WSAECONNRESET);
				break;
			case WSAEINVAL:
				message.LoadString(IDS_SEND_WSAEINVAL);
				break;
			default:
				message.Format("Unknown receive error %d", error_code);
				break;
			}
		
		m_access_lock.DEBUG_LOCK();
		m_error_message = message;
		m_access_lock.Unlock();
		
		return SOCKET_RESULT_ERROR;
		}
	else
		{
		amount_read = r;
		return SOCKET_RESULT_OK;
		}
	}

SOCKET_RESULT CThreadSocket::Close()
	{
	m_access_lock.DEBUG_LOCK();
	SOCKET s = m_socket;
	m_socket = NULL;
	m_access_lock.Unlock();
		
	if (s)
		{
		closesocket(s);
		}

	return SOCKET_RESULT_OK;
	}


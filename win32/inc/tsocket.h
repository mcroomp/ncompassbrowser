#define _TSOCKET_H_

#ifndef _MUTEX_H_
#include "mutex.h"
#endif

typedef enum
	{
	SOCKET_RESULT_OK,
	SOCKET_RESULT_ABORTED,
	SOCKET_RESULT_ERROR
	} SOCKET_RESULT;

class CThreadSocket
	{
public:
	CThreadSocket();
	~CThreadSocket();

	SOCKET_RESULT Connect(LPCSTR host, INT port_number);
	SOCKET_RESULT Write(LPCVOID buffer, INT32 amount);
	SOCKET_RESULT Read(LPVOID buffer, INT32 buffer_size, INT32& amount_read);
	SOCKET_RESULT Close();

	CString GetLastErrorMessage();

	void Abort();

static BOOL Initialize();
static void Terminate();

	CString GetErrorMessage();
	
private:
	CAccessLock m_access_lock;

	BOOL m_interrupt;
	HWND m_message_window;
	SOCKET m_socket;
	CString m_error_message;
	};

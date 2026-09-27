#define _PROTOCOL_H_

#ifndef _DYNLOAD_H_
#include "dynload.h"
#endif

#ifndef _TSOCKET_H_
#include "tsocket.h"
#endif

class CProtocol
	{
public:
	CProtocol(CDynamicLoad *load_object);
	virtual ~CProtocol();

	virtual void BeginLoadThread() = 0;
	virtual void AbortLoadThread() = 0;
	virtual void WaitEndLoadThread() = 0;

static void OpenProtocolManager();
static void CloseProtocolManager();

protected:
	void SetLoadState( LOAD_STATE l );

	LOAD_STATE CallOnEndLoading();
	LOAD_STATE CallOnPreLoading();
	LOAD_STATE CallOnBeginLoading( const CMapStringToString& map );
	LOAD_STATE CallOnLoading(LPCBYTE buffer, INT32 length);
	void CallNotify(UINT32 flags);
	void SetURL(LPCSTR url);

	CString m_url;
	CDynamicLoad *m_load_object;
	CWinThread *m_thread_handle;
	};

CString ConvertFilenameToURL( LPCSTR filename);

class CProtocolFile : public CProtocol
	{
public:
	CProtocolFile(CDynamicLoad *load_object);
	~CProtocolFile();

	virtual void BeginLoadThread();
	virtual void AbortLoadThread();
	virtual void WaitEndLoadThread();
private:
	volatile BOOL m_bTerminateThread;

	HANDLE m_thread_done_semaphore;
	
	friend UINT FileWorkerThread( LPVOID lparam );
	};

class CProtocolHTTP : public CProtocol
	{
public:
	CProtocolHTTP(CDynamicLoad *load_object);
	~CProtocolHTTP();

	virtual void BeginLoadThread();
	virtual void AbortLoadThread();
	virtual void WaitEndLoadThread();
private:
	BOOL SetSession(LPVOID session);
	void CloseSession(LPVOID session);
	BOOL IsAborted();

	CAccessLock m_access_lock;
	LPVOID m_session;
	BOOL m_aborted;
	HANDLE m_thread_done_semaphore;

	friend UINT HTTPWorkerThread( LPVOID lparam );
	friend UINT WinHTTPWorkerThread( LPVOID lparam );
	};

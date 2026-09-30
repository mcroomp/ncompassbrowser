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
	friend UINT WinInetWorkerThread( LPVOID lparam );
	};

// Used for a URL scheme the loader doesn't recognize, and for a load that is
// cancelled before a real protocol was ever created (still waiting for a
// free connection slot). Rather than the dynamic loader calling OnEndLoading
// directly, this still runs it on a genuine worker thread, matching every
// other protocol so the loader's thread-affinity contract never has a
// special case.
class CProtocolUnknown : public CProtocol
	{
public:
	CProtocolUnknown(CDynamicLoad *load_object);
	~CProtocolUnknown();

	virtual void BeginLoadThread();
	virtual void AbortLoadThread();
	virtual void WaitEndLoadThread();
private:
	HANDLE m_thread_done_semaphore;

	friend UINT UnknownProtocolWorkerThread( LPVOID lparam );
	};

#define _MUTEX_H_

#ifdef _DEBUG
#define ASSERT_LOCKED(object) ( (object)->AssertLocked() )
#else
#define ASSERT_LOCKED(object)
#endif

#ifdef _DEBUG
#define DEBUG_LOCK() Lock( __FILE__, __LINE__ )
#else
#define DEBUG_LOCK() Lock()
#endif

// Records which thread owns the UI (document/view/parser/format) side of the
// browser, and dies immediately (via the same ASSERT->non-continuable
// exception path as ASSERT_LOCKED) if a function meant for one side of the
// thread model is reached from the other. Call SetUIThreadID() once, from
// CViewhtmlApp::InitInstance, before any dynamic loading can occur.

#ifdef _DEBUG
void SetUIThreadID();
void AssertUIThread();
void AssertWorkerThread();
#define ASSERT_UI_THREAD() AssertUIThread()
#define ASSERT_WORKER_THREAD() AssertWorkerThread()
#else
#define ASSERT_UI_THREAD()
#define ASSERT_WORKER_THREAD()
#endif

class CAccessLock
	{
public:
	CAccessLock();
	~CAccessLock();

#ifdef _DEBUG
	void Lock(LPCSTR source_file, INT line_number);
#else
	void Lock();
#endif

	void Unlock();
	
#ifdef _DEBUG
	void AssertLocked() const;
#endif

private:
	HANDLE m_semaphore;
	LONG m_interlock;

#ifdef _DEBUG
	CWinThread *m_thread_owner;
	BOOL m_is_locked;
	LPCSTR m_source_file;
	INT m_line_number;
#endif
	};

#ifndef _DEBUG

inline void CAccessLock::Lock()
	{
	if (InterlockedIncrement(&m_interlock) > 0)
		{
		WaitForSingleObject(m_semaphore, INFINITE);
		}
	}
	
inline void CAccessLock::Unlock()
	{
	if (InterlockedDecrement(&m_interlock) >= 0)
		{
		ReleaseSemaphore(m_semaphore,1,NULL);
		Sleep(0);		// prevent the waiting object from starving to death
		}
	}

#endif

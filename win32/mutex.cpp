#include "cross_p.h"

#include "mutex.h"

///////////////////////////////////////////////
// CReadWriteMutex implementation
///////////////////////////////////////////////

CAccessLock::CAccessLock()
	{
#ifdef _DEBUG
	m_is_locked = FALSE;
	m_line_number = 0;
	m_source_file = NULL;
	m_thread_owner = NULL;
#endif
	m_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
	m_interlock = -1;
	}

CAccessLock::~CAccessLock()
	{
	CloseHandle(m_semaphore);
	}

#ifdef _DEBUG

void CAccessLock::Lock(LPCSTR source_file, INT line_number)
	{
	if (InterlockedIncrement(&m_interlock) > 0)
		{
		CWinThread *t = m_thread_owner;
		if (t)
			ASSERT( t->m_nThreadID != AfxGetThread()->m_nThreadID);		// make sure we don't deadlock ourselves

		WaitForSingleObject(m_semaphore, INFINITE);
		}

	ASSERT(!m_is_locked);

	m_is_locked = TRUE;
	m_source_file = source_file;
	m_line_number = line_number;
	m_thread_owner = AfxGetThread();
	}

void CAccessLock::Unlock()
	{
	ASSERT(m_is_locked);

	m_is_locked = FALSE;
	m_source_file = NULL;
	m_line_number = 0;

	if (InterlockedDecrement(&m_interlock) >= 0)
		{
		ReleaseSemaphore(m_semaphore,1,NULL);
		Sleep(0);		// prevent the waiting object from starving to death
		}
	}
	
void CAccessLock::AssertLocked() const
	{
	ASSERT(m_is_locked);
	}

#endif

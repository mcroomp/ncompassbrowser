#include "cross_p.h"
#include "resource.h"

#ifdef _WINDOWS
#include "viewhtml.h"
#endif

#ifndef _DYNLOAD_H_
#include "dynload.h"
#endif

#ifndef _PROTOCOL_H_
#include "protocol.h"
#endif

#ifndef _SMTP_H_
#include "smtp.h"
#endif

#define WM_DO_NOTIFY (WM_USER)

IMPLEMENT_DYNAMIC( CDynamicLoad, CObject );

static CPtrList g_new_list;
static CPtrList g_loading_list;
static CPtrList g_done_list;
static CPtrList g_done_notify_list;
static POSITION g_current_walk = NULL;


CNotifyObject::CNotifyObject()
	{
	}

CNotifyObject::~CNotifyObject()
	{
	POSITION walk = GetFirstDynamicLoadPosition();
	while(walk)
		{
		CDynamicLoad *dlobject = GetNextDynamicLoad(walk);

		dlobject->AbortLoading();
		delete dlobject;
		}
	}

CDynamicLoad::CDynamicLoad( CNotifyObject *notify, LPCSTR url, METHOD_TYPE method, LPCSTR post_headers, LPCSTR post_data)
	{
	m_notify_object = notify;
	m_url = url;
	m_method = method;
	if (post_headers)
		m_post_headers = post_headers;
	if (post_data)
		m_post_data = post_data;
	m_current_list = LIST_NONE;
	m_list_position = NULL;
	m_load_state = LOAD_STATE_NOT_LOADED;

	m_notify_flags = 0;
	m_notify_object_list_pos = m_notify_object->m_dynamic_objects.AddTail(this);
	}

CDynamicLoad::~CDynamicLoad()
	{
	m_notify_object->m_dynamic_objects.RemoveAt(m_notify_object_list_pos);
	m_notify_object = NULL;

	if (m_current_list == LIST_NONE)
		{
		// in case StartLoading never got called
		return;
		}
	
	ASSERT(m_current_list == LIST_DONE || m_current_list == LIST_DONE_NOTIFY);

	if (m_current_list == LIST_DONE)
		{
		ASSERT( g_done_list.Find(this) == m_list_position);
		g_done_list.RemoveAt( m_list_position );
		}
	else if (m_current_list == LIST_DONE_NOTIFY)
		{
		ASSERT( g_done_notify_list.Find(this) == m_list_position);
		g_done_notify_list.RemoveAt( m_list_position );
		}
	}


LOAD_STATE CDynamicLoad::GetLoadState() const
	{
	ASSERT_LOCKED(this);

	return m_load_state;
	}

CString CDynamicLoad::GetURL() const
	{
	ASSERT_LOCKED(this);

	return m_url;
	}

METHOD_TYPE CDynamicLoad::GetMethodType() const
	{
	ASSERT_LOCKED(this);

	return m_method;
	}

CString CDynamicLoad::GetPostHeaders() const
	{
	ASSERT_LOCKED(this);

	return m_post_headers;
	}

CString CDynamicLoad::GetPostData() const
	{
	ASSERT_LOCKED(this);

	return m_post_data;
	}
		

void CDynamicLoad::SetProgressMessage( LPCSTR message )
	{
	ASSERT_LOCKED(this);

	m_progress_message = message;
	}

void CDynamicLoad::SetErrorMessage( UINT message_id, LPCSTR param1, LPCSTR param2)
	{
	ASSERT_LOCKED(this);

	CString main_str;

	main_str.LoadString(message_id);

	m_error_message.Format( main_str, param1, param2 );
	}

CString CDynamicLoad::GetErrorMessage() const
	{
	ASSERT_LOCKED(this);

	return m_error_message;
	}

void CDynamicLoad::InvokeLoadingThread()
	{
	if (strnicmp(m_url, "file:", 5) == 0)
		{
		m_current_list = LIST_LOADING;
		m_list_position = g_loading_list.AddTail( this );

		m_load_state = LOAD_STATE_LOADING;
		g_current_walk = NULL; // restart walking the notify list
		
		m_loading_protocol = new CProtocolFile(this);
		m_loading_protocol->BeginLoadThread();
		}
	else if (strnicmp(m_url, "http:", 5) == 0 ||
			 strnicmp(m_url, "https:", 6) == 0)
		{
		m_current_list = LIST_LOADING;
		m_list_position = g_loading_list.AddTail( this );

		m_load_state = LOAD_STATE_LOADING;
		g_current_walk = NULL; // restart walking the notify list
		
		m_loading_protocol = new CProtocolHTTP(this);
		m_loading_protocol->BeginLoadThread();
		} 
	else if (strnicmp(m_url, "mailto:", 7) == 0)
		{
		m_current_list = LIST_LOADING;
		m_list_position = g_loading_list.AddTail( this );

		m_load_state = LOAD_STATE_LOADING;
		g_current_walk = NULL; // restart walking the notify list
		
		m_loading_protocol = new CProtocolSMTP(this);
		m_loading_protocol->BeginLoadThread();
		}
	else
		{
		m_current_list = LIST_LOADING;
		m_list_position = g_loading_list.AddTail( this );

		DEBUG_LOCK();
		SetErrorMessage(IDS_URL_TYPE_UNKNOWN, m_url );
		m_load_state = LOAD_STATE_ABORTED;
		Unlock();

		g_current_walk = NULL; // restart walking the notify list

		// no real protocol applies to an unrecognized URL scheme, but a
		// worker thread still calls OnEndLoading and signals CHANGEFLAG_DONE
		// exactly like every other protocol, so DoNotifies can reap this
		// object identically regardless of scheme
		m_loading_protocol = new CProtocolUnknown(this);
		m_loading_protocol->BeginLoadThread();
		}
	}

void CDynamicLoad::StartLoading()
	{
	ASSERT(m_current_list == LIST_NONE);

	LOAD_STATE l = OnPreLoading();
	if (l == LOAD_STATE_COMPLETE || l == LOAD_STATE_ABORTED)
		{
		m_current_list = LIST_DONE_NOTIFY;
		m_list_position = g_done_notify_list.AddTail(this);

		m_load_state = l;
		return;
		}
	
	if (g_loading_list.GetCount() == theApp.m_max_connections )
		{
		// If there are too many things loading already, add to the new list rather than
		//  starting the load immediately

		m_current_list = LIST_NEW;
		m_list_position = g_new_list.AddTail(this);

		m_load_state = LOAD_STATE_NOT_LOADED;
		return;
		}
	
	InvokeLoadingThread();

#ifdef _WINDOWS
	theApp.SignalNotify();
#endif
	}

void CDynamicLoad::AbortLoading()
	{
	if (m_current_list == LIST_NONE || m_current_list == LIST_DONE ||
		m_current_list == LIST_DONE_NOTIFY)
		return;

	if (m_current_list == LIST_NEW)
		{
		ASSERT( g_new_list.Find(this) == m_list_position);
		g_new_list.RemoveAt( m_list_position );

		DEBUG_LOCK();
		m_load_state = LOAD_STATE_ABORTED;
		Unlock();

		// never had a real protocol (still queued waiting for a free
		// connection slot), but OnEndLoading still runs on a worker thread,
		// exactly like every other cancellation path
		CProtocol *abort_protocol = new CProtocolUnknown(this);
		abort_protocol->BeginLoadThread();
		abort_protocol->AbortLoadThread();
		delete abort_protocol;

		m_current_list = LIST_DONE_NOTIFY;
		m_list_position = g_done_notify_list.AddTail(this);
		}
	else if (m_current_list == LIST_LOADING)
		{
		m_loading_protocol->AbortLoadThread();
			// AbortLoadThread() blocks until the worker thread itself has
			// called OnEndLoading() and Notify(CHANGEFLAG_DONE), exactly as
			// it would for a normal completion, so neither is called here
		delete m_loading_protocol;
		m_loading_protocol = NULL;


		ASSERT( g_loading_list.Find(this) == m_list_position);
		g_loading_list.RemoveAt( m_list_position );

		m_current_list = LIST_DONE_NOTIFY;
		m_list_position = g_done_notify_list.AddTail(this);

		g_current_walk = NULL; // restart walking the notify list

		// check to see if anyone is waiting to load, and let them load
		while(g_loading_list.GetCount() < theApp.m_max_connections && g_new_list.GetCount() > 0)
			{
			CDynamicLoad *newitem = (CDynamicLoad *)g_new_list.RemoveTail();
			newitem->InvokeLoadingThread();
			}
		}

	// item has already finished so there's no point in waiting for it to complete
#ifdef _WINDOWS
	theApp.SignalNotify();
#endif
	}
		
void CDynamicLoad::Notify(UINT32 flags)
	{
	ASSERT_LOCKED(this);

	m_notify_flags |= flags;
#ifdef _WINDOWS
	theApp.SignalNotify();
#endif
	}

CString CDynamicLoad::DoNotifies()
	{
	ASSERT_UI_THREAD();

	CString message;


	g_current_walk = g_loading_list.GetHeadPosition();
	while(g_current_walk)
		{
		CDynamicLoad *dlobject =
			(CDynamicLoad *)g_loading_list.GetNext(g_current_walk);

		dlobject->DEBUG_LOCK();

		if (!dlobject->m_progress_message.IsEmpty() )
			{
			message = dlobject->m_progress_message;
			dlobject->m_progress_message.Empty();
			}

		UINT32 flags = dlobject->m_notify_flags;
		dlobject->m_notify_flags = 0;
		dlobject->Unlock();

		// if we are finished loading, wait for the protocol thread to finish, delete the
		// protocol and move the object to the done list.

		if (flags & CHANGEFLAG_URL_REDIRECT)
			{
			dlobject->m_loading_protocol->WaitEndLoadThread();
			delete dlobject->m_loading_protocol;
			dlobject->m_loading_protocol = NULL;


			ASSERT( g_loading_list.Find(dlobject) == dlobject->m_list_position);
			g_loading_list.RemoveAt( dlobject->m_list_position );
			dlobject->m_current_list = LIST_NONE;

			LOAD_STATE l = dlobject->OnPreLoading();
			
			dlobject->DEBUG_LOCK();
			dlobject->m_load_state = l;
			dlobject->Unlock();

			if (l == LOAD_STATE_ABORTED || l == LOAD_STATE_COMPLETE)
				goto done_loading;
	
			dlobject->InvokeLoadingThread();
#ifdef _WINDOWS
			theApp.SignalNotify();
#endif

			}

		if (flags & CHANGEFLAG_DONE )
			{
			dlobject->m_loading_protocol->WaitEndLoadThread();
			delete dlobject->m_loading_protocol;
			dlobject->m_loading_protocol = NULL;


			ASSERT( g_loading_list.Find(dlobject) == dlobject->m_list_position);
			g_loading_list.RemoveAt( dlobject->m_list_position );
done_loading:
			dlobject->m_current_list = LIST_DONE;
			dlobject->m_list_position = g_done_list.AddTail( dlobject );
			
			// check to see if anyone is waiting to load, and let them load
			while(g_loading_list.GetCount() < theApp.m_max_connections && g_new_list.GetCount() > 0)
				{
				CDynamicLoad *newitem = (CDynamicLoad *)g_new_list.RemoveTail();
				newitem->InvokeLoadingThread();
#ifdef _WINDOWS
				theApp.SignalNotify();
#endif
				}
			}
		
		if (flags)
			{
			// safe to access m_notify_object since it never changes during the life of the object
			dlobject->m_notify_object->OnNotify(flags, dlobject );
			}
		}

	// Take everything from the aborted list, do a notify, and send it immediately to the done list
	g_current_walk = NULL;
	while(!g_done_notify_list.IsEmpty())
		{
		CDynamicLoad *dlobject =
			(CDynamicLoad *)g_done_notify_list.GetHead();

		dlobject->DEBUG_LOCK();

	   	if (!dlobject->m_progress_message.IsEmpty() )
			{
			message = dlobject->m_progress_message;
			dlobject->m_progress_message.Empty();
			}

		UINT32 flags = dlobject->m_notify_flags;
		dlobject->m_notify_flags = 0;
		dlobject->Unlock();

		ASSERT(g_done_notify_list.Find(dlobject) == dlobject->m_list_position);
		g_done_notify_list.RemoveAt(dlobject->m_list_position);

		dlobject->m_current_list = LIST_DONE;
		dlobject->m_list_position = g_done_list.AddTail(dlobject);

		if (flags)
			{
			// safe to access m_notify_object since it never changes during the life of the object
			dlobject->m_notify_object->OnNotify(flags, dlobject );
			}
		}


	return message;
	}

#ifdef _DEBUG

void CDynamicLoad::Lock(LPCSTR file, INT line_number)
	{
	m_access_lock.Lock(file, line_number);
	}

#else

void CDynamicLoad::Lock()
	{
	m_access_lock.Lock();
	}

#endif

void CDynamicLoad::Unlock()
	{
	m_access_lock.Unlock();
	}

#ifdef _DEBUG

void CDynamicLoad::AssertLocked() const
	{
	m_access_lock.AssertLocked();
	}

#endif

// default implementations for objects that don't care about mime types and the kind of stuff
LOAD_STATE CDynamicLoad::OnPreLoading()
	{
	ASSERT_UI_THREAD();

	return LOAD_STATE_LOADING;
	}

LOAD_STATE CDynamicLoad::OnBeginLoading( const CMapStringToString& mime_header )
	{
	ASSERT_WORKER_THREAD();

	return LOAD_STATE_LOADING;
	}
	
void CDynamicLoad::ClearCacheEntry()
	{
	// do nothing default implementation
	}

void CDynamicLoad::InitializeDynamicLoader()
	{
	CProtocol::OpenProtocolManager();
	}

void CDynamicLoad::CloseDynamicLoader()
	{
	CProtocol::CloseProtocolManager();
	}

BOOL CDynamicLoad::IsAnyDynamicLoading()
	{
	return g_loading_list.GetCount() > 0;
	}

CString CombineURL( LPCSTR old_url, LPCSTR new_url)
	{
	LPCSTR p,p2;

	CString old_sitename;		// http://sitename
	CString old_dir;			// /dir1/dir2/file.html

	p = strstr(old_url, "//");
	if (!p)
		goto give_up;

	p2 = strchr(p+2, '/');
	if (!p2)
		{
		old_sitename = old_url;
		old_dir = "/";
		}
	else
		{
		old_sitename = CString(old_url, p2 - old_url);
		old_dir = p2;
		}

	if (new_url[0] == '/' &&
		new_url[1] != '/')
		{
		// absolute pathname on same server

		return old_sitename + new_url;
		}
	else if ( new_url[0] != '/' && strstr(new_url, "//") == 0 &&
		strchr(new_url, ':') == 0)
		{
		// relative pathname to current directory

		const char *new_url_p = new_url;
		const char *begin_old_dir = old_dir;

		const char *p = strrchr(begin_old_dir, '/');

		// parse the down one level relative URL (../)
		while( p >= begin_old_dir && memcmp(new_url_p, "../", 3) == 0)
			{
			p--;

			while( p >= begin_old_dir)
				{
				if (*p == '/')
					break;
				p--;
				}

			new_url_p += 3;
			}

		if (p < begin_old_dir)
			goto give_up;

		return old_sitename + CString( begin_old_dir, p - begin_old_dir + 1) + CString( new_url_p );
		}
	else
		{
give_up:
		// just use the new URL
		return CString(new_url);
		}
	}

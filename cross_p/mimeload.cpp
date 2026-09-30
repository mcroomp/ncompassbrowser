#include "cross_p.h"

#ifndef _DYNLOAD_H_
#include "dynload.h"
#endif

#ifndef _MIMELOAD_H_
#include "mimeload.h"
#endif

#ifndef _READGIF_H_
#include "readgif.h"
#endif

#ifndef _READJPEG_H_
#include "readjpeg.h"
#endif

#ifndef _READHTML_H_
#include "readhtml.h"
#endif

#ifndef _DMEMFILE_H_
#include "dmemfile.h"
#endif

#ifndef _SMTP_H_
#include "smtp.h"
#endif

IMPLEMENT_DYNAMIC( CMimeDynamicLoad, CDynamicLoad );

#define MAX_CACHE_SIZE 50

typedef struct
	{
	CAccessLock m_access_lock;
	CPtrList m_current_mime_list;
	CPtrList m_cache_mime_list;

	} MIMELOAD_GLOBALS;

static MIMELOAD_GLOBALS *globals;

void CMimeDynamicLoad::OpenMimeLoader()
	{
	globals = new MIMELOAD_GLOBALS;
	}

void CMimeDynamicLoad::CloseMimeLoader()
	{
	ASSERT( globals->m_current_mime_list.GetCount() == 0);

	POSITION walk = globals->m_cache_mime_list.GetHeadPosition();
	while(walk)
		{
		CMimeObject *mime_object = (CMimeObject *)globals->m_cache_mime_list.GetNext(walk);
		delete mime_object;
		}

	globals->m_cache_mime_list.RemoveAll();

	delete globals;
	globals = NULL;
	}

void CMimeDynamicLoad::ClearCacheEntry()
	{
	globals->m_access_lock.DEBUG_LOCK();
			
	DEBUG_LOCK();
	
	ASSERT( GetLoadState() != LOAD_STATE_LOADING);

	if (m_mime_object)
		{
		m_mime_object->DEBUG_LOCK();

		if (m_mime_object->m_parent_list.GetCount() == 1)
			{
			globals->m_current_mime_list.RemoveAt( m_mime_object->m_list_position );
			
			// if we are the only dynload that references this mime object, the kill it
			m_mime_object->m_parent_list.RemoveAll();
			delete m_mime_object;
			m_mime_object = NULL;
			}
		else
			{
			// otherwise just remove the dynload fromt the parent list
			m_mime_object->m_parent_list.RemoveAt( m_mime_object->m_parent_list.Find(this) );
			m_mime_object->Unlock();
			}
		}

	m_mime_object = NULL;
	Unlock();
	globals->m_access_lock.Unlock();
	}

CMimeObject * CMimeDynamicLoad::GetMimeObject()
	{
	ASSERT_LOCKED(this);

	return m_mime_object;
	}

CString CMimeDynamicLoad::GetMimeType()
	{
	ASSERT_LOCKED(this);

	return m_mime_type;
	}

CMimeDynamicLoad::CMimeDynamicLoad(CNotifyObject *notify, LPCSTR url, METHOD_TYPE method, LPCSTR post_headers, LPCSTR post_data)
	: CDynamicLoad(notify, url, method, post_headers, post_data)
	{
	m_mime_object = NULL;
	m_mime_type.Empty();
	}

CMimeDynamicLoad::~CMimeDynamicLoad()
	{
	DEBUG_LOCK();
	CString url = GetURL();
	Unlock();

	if (m_mime_object)
		{
		globals->m_access_lock.DEBUG_LOCK();
		m_mime_object->DEBUG_LOCK();
		if (m_mime_object->m_parent_list.GetCount() == 1)
			{
			// if there is only one reference count to the object, then move it to the cache list
			m_mime_object->m_parent_list.RemoveAll();
			globals->m_current_mime_list.RemoveAt( m_mime_object->m_list_position );

			if (m_mime_object->m_load_state == LOAD_STATE_COMPLETE)
				{
				// cache only completed objects
				m_mime_object->m_list_position = globals->m_cache_mime_list.AddHead(m_mime_object);
		
				if (globals->m_cache_mime_list.GetCount() > MAX_CACHE_SIZE)
					{
					// if the cache is big than than the maximum, delete the last entry in the cache
					//  to make room for us

					CMimeObject *victim = (CMimeObject *)globals->m_cache_mime_list.GetTail();
					globals->m_cache_mime_list.RemoveTail();
					delete victim;
					}
				}
			else
				{
				// delete all partially loaded objects
				delete m_mime_object;
				}
			}
		else
			{
			// otherwise just remove us from the parent list
			m_mime_object->m_parent_list.RemoveAt(
				m_mime_object->m_parent_list.Find(this) );
			}
		m_mime_object->Unlock();
		globals->m_access_lock.Unlock();
		}
	}

LOAD_STATE CMimeDynamicLoad::OnPreLoading()
	{
	ASSERT_UI_THREAD();

	globals->m_access_lock.DEBUG_LOCK();
	DEBUG_LOCK();
	CString url = GetURL();
	
	// walk through currently existing items to see if any of them match our URL
	POSITION walk = globals->m_current_mime_list.GetHeadPosition();
	while(walk)
		{
		CMimeObject *mime_object = (CMimeObject *)globals->m_current_mime_list.GetNext(walk);

		if (mime_object->m_url == url)
			{
			mime_object->DEBUG_LOCK();
			mime_object->m_parent_list.AddTail(this);
			m_mime_object = mime_object;
			mime_object->Unlock();
			Notify(CHANGEFLAG_CACHED);
			Unlock();
			globals->m_access_lock.Unlock();
			return LOAD_STATE_COMPLETE;
			}
		}

	// walk through cached items to see if any match our URL
	walk = globals->m_cache_mime_list.GetHeadPosition();
	while(walk)
		{
		CMimeObject *mime_object = (CMimeObject *)globals->m_cache_mime_list.GetNext(walk);

		if (mime_object->m_url == url)
			{
			// If we find one, remove it from the cache and put it into the current list

			mime_object->DEBUG_LOCK();
			mime_object->m_parent_list.AddTail(this);
			m_mime_object = mime_object;
			globals->m_cache_mime_list.RemoveAt( m_mime_object->m_list_position );
			m_mime_object->m_list_position = globals->m_current_mime_list.AddHead( m_mime_object );

			mime_object->Unlock();
			Notify(CHANGEFLAG_CACHED);
			Unlock();
			globals->m_access_lock.Unlock();
			return LOAD_STATE_COMPLETE;
			}
		}
	
	Unlock();
	globals->m_access_lock.Unlock();
	
	// otherwise continue loading
	return LOAD_STATE_LOADING;
	}

LOAD_STATE CMimeDynamicLoad::OnBeginLoading( const CMapStringToString& header )
	{
	ASSERT_WORKER_THREAD();

	CString mime_type;
	CMimeObject *mime_object;

	DEBUG_LOCK();
	CString url = GetURL();
	Unlock();

	if (!header.Lookup("content-type", mime_type))
		mime_type = "unknown";
	
	PICTURE_FORMAT_INFO format_info;

	format_info.m_bits_per_pixel = 8;
	format_info.m_use_standard_palette = TRUE;
	
	globals->m_access_lock.DEBUG_LOCK();

	if (mime_type == "text/html")
		{
		mime_object = DEBUG_NEW CParseHTML(url, mime_type, FALSE);
		}
	else if (mime_type == "text/plain")
		{
		mime_object = DEBUG_NEW CParseHTML(url, mime_type, TRUE);
		}
	else if (mime_type == "image/gif")
		{
		mime_object = DEBUG_NEW CGifPicture(url, mime_type, format_info);
		}
	else if (mime_type == "image/jpeg")
		{
		mime_object = DEBUG_NEW CJpegPicture(url, mime_type, format_info);
		}
	else if (mime_type == "internal/smtp-post" || 
			mime_type == "internal/smtp-launch" )
		{
		mime_object = DEBUG_NEW CMimeSMTP(url, mime_type);
		}
	else
		{
		mime_object = DEBUG_NEW CMimeLoadMemFile(url, mime_type);
		}

	mime_object->m_parent_list.AddTail(this);
	
	mime_object->m_list_position = globals->m_current_mime_list.AddHead(mime_object);

	DEBUG_LOCK();
	m_mime_object = mime_object;
	m_mime_type = mime_type;
	Notify(CHANGEFLAG_MIMEHEADER_READ);
	Unlock();

	globals->m_access_lock.Unlock();
	
	return LOAD_STATE_LOADING;
	}

LOAD_STATE CMimeDynamicLoad::OnLoading(LPCBYTE buffer, INT32 buffer_size)
	{
	ASSERT_WORKER_THREAD();
	ASSERT(m_mime_object);

	DEBUG_LOCK();
	m_mime_object->m_load_state = GetLoadState();
	Unlock();

	LOAD_STATE l = m_mime_object->OnReadData(buffer, buffer_size);

	m_mime_object->DEBUG_LOCK();
	m_mime_object->m_load_state = l;
	m_mime_object->Unlock();

	return m_mime_object->m_load_state;
	}

LOAD_STATE CMimeDynamicLoad::OnEndLoading()
	{
	ASSERT_WORKER_THREAD();

	if (!m_mime_object)
		{
		return LOAD_STATE_ABORTED;
		}

	DEBUG_LOCK();
	m_mime_object->m_load_state = GetLoadState();
	Unlock();

	m_mime_object->m_load_state = m_mime_object->OnEndOfFile();
	
	return m_mime_object->m_load_state;	
	}


// During constructor the parent object is locked
CMimeObject::CMimeObject(MIME_OBJECT_TYPE object_type, LPCSTR url, LPCSTR mime_type)
	: m_mime_type(mime_type), m_url(url), m_mime_object_type(object_type)
	{
	m_load_state = LOAD_STATE_LOADING;
	}

CMimeObject::~CMimeObject()
	{
	}

void CMimeObject::Notify( INT32 message )
	{
	// Send the notify message to all parents

	POSITION walk = m_parent_list.GetHeadPosition();
	while(walk)
		{
		CMimeDynamicLoad *mime = (CMimeDynamicLoad *)m_parent_list.GetNext(walk);

		mime->DEBUG_LOCK();
		mime->Notify(message);
		mime->Unlock();
		}
	}


POSITION CMimeObject::GetFirstTagPos() const
	{
	ASSERT_LOCKED(this);

	return NULL;
	}

const CTag * CMimeObject::GetNextTag( POSITION& walk) const
	{
	ASSERT_LOCKED(this);

	walk = NULL;
	return NULL;
	}
	
CBigString CMimeObject::GetPlainText() const
	{
	ASSERT_LOCKED(this);

	return CBigString();
	}

POSITION CMimeObject::FindTagPos( const CTag *tag) const
	{
	ASSERT_LOCKED(this);

	return NULL;
	}

CString CMimeObject::GetTitle() const
	{
	ASSERT_LOCKED(this);

	return CString();
	}

INT32 CMimeObject::GetBackgroundColor() const
	{
	ASSERT_LOCKED(this);

	return -1;
	}
INT32 CMimeObject::GetTextColor() const
	{
	ASSERT_LOCKED(this);

	return -1;
	}

INT32 CMimeObject::GetHotlinkColor() const
	{
	ASSERT_LOCKED(this);

	return -1;
	}

INT32 CMimeObject::GetOldHotlinkColor() const
	{
	ASSERT_LOCKED(this);

	return -1;
	}

CString CMimeObject::GetBackgroundPicture() const
	{
	ASSERT_LOCKED(this);

	return CString();
	}
	
BOOL CMimeObject::UsesInternalViewer() const
	{
	return TRUE;
	}

void CMimeObject::LaunchViewer()
	{
	// don't know what kind of viewer to launch
	ASSERT(FALSE);
	}

	

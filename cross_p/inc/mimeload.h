#define _MIMELOAD_H_

#ifndef _DYNLOAD_H_
#include "dynload.h"
#endif

#ifndef _BIGSTR_H_
#include "bigstr.h"
#endif

#define CHANGEFLAG_MIMEHEADER_READ 0x00000001
#define CHANGEFLAG_CACHED 		   0x00000002

typedef enum
	{
	MIME_OBJECT_PICTURE,
	MIME_OBJECT_HTML,
	MIME_OBJECT_MEMFILE,
	MIME_OBJECT_POST_SMTP,
	MIME_OBJECT_LAUNCH_SMTP
	} MIME_OBJECT_TYPE;

class CMimeObject;
class CTag;

class CMimeDynamicLoad : public CDynamicLoad
	{
public:
	CMimeDynamicLoad( CNotifyObject *notify, LPCSTR url, METHOD_TYPE method = METHOD_GET, LPCSTR post_headers = NULL, LPCSTR post_data = NULL);
	~CMimeDynamicLoad();

static void OpenMimeLoader();
static void CloseMimeLoader();

// The object must be locked before using these
	CMimeObject * GetMimeObject();
	CString GetMimeType();

protected:
	virtual LOAD_STATE OnPreLoading();	
	virtual LOAD_STATE OnBeginLoading( const CMapStringToString& header );
	virtual LOAD_STATE OnLoading(LPCBYTE buffer, INT32 buffer_size);
	virtual LOAD_STATE OnEndLoading();	

	virtual void ClearCacheEntry();
private:
	CMimeObject *m_mime_object;
	CString m_mime_type;		// the MIME type of this loaded object

	DECLARE_DYNAMIC( CMimeDynamicLoad );

	friend class CMimeObject;
	};

class CMimeObject : public CObject
	{
protected:
	CMimeObject( MIME_OBJECT_TYPE mime_object_type, LPCSTR url, LPCSTR mime_type );
	virtual ~CMimeObject();

	void Notify( INT32 message );

	virtual LOAD_STATE OnReadData(LPCBYTE buffer, INT32 buffer_size) = 0;
	virtual LOAD_STATE OnEndOfFile() = 0;

	friend class CMimeDynamicLoad;

public:

#ifdef _DEBUG
	void Lock(LPCSTR file, INT line_number)
		{
		m_access_lock.Lock(file, line_number);
		}
#else
	void Lock()
		{
		m_access_lock.Lock();
		}
#endif

	void Unlock()
		{ m_access_lock.Unlock(); };

#ifdef _DEBUG
	void AssertLocked() const
		{ m_access_lock.AssertLocked(); }
#endif

// to make things easier for the formatter, every mime object must return some kind of format
// list even if it is blank (default implementation)

	virtual void LaunchViewer();
		// Launches an external viewer to view the contents of this object. The view should
		// not change the internal state of the object. This function may be called multiple
		// times on the same object.

public:
	// These functions can be called without locking, as their contents never change
	virtual BOOL UsesInternalViewer() const;

	inline MIME_OBJECT_TYPE GetObjectType() const
		{ return m_mime_object_type; }

	inline CString GetMimeType() const
		{ return m_mime_type; }

	inline CString GetURL() const
		{ return m_url; }

		// This function should return TRUE if this object can be meaningfully displayed by
		// the format engine. If it returns FALSE, the overridable LaunchViewer function will
		// be called. The default implementation returns TRUE.

	// The following functions must be called while the object is locked
	virtual POSITION FindTagPos( const CTag *tag) const;
	virtual CBigString GetPlainText() const;
	virtual POSITION GetFirstTagPos() const;
	virtual const CTag * GetNextTag(POSITION &walk) const;
	virtual CString GetTitle() const;
	virtual INT32 GetBackgroundColor() const;
	virtual INT32 GetTextColor() const;
	virtual INT32 GetHotlinkColor() const;
	virtual INT32 GetOldHotlinkColor() const;
	virtual CString GetBackgroundPicture() const;

	inline LOAD_STATE GetLoadState() const
		{ 
		ASSERT_LOCKED(this);
		return m_load_state;	
		}

private:
	LOAD_STATE m_load_state;
	CAccessLock m_access_lock;
	CPtrList m_parent_list;
	POSITION m_list_position;		
		// the position of the item either in the current mime list or in the cached mime list
	
	const MIME_OBJECT_TYPE m_mime_object_type;
	const CString m_url;
	const CString m_mime_type;

	friend class CMimeDynamicLoad;
	};

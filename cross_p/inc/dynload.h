#define _DYNLOAD_H_

#ifndef _MUTEX_H_
#include "mutex.h"
#endif

// Top 4 bits are reserved for CDynamicLoad
#define CHANGEFLAG_DONE 			0x80000000
#define CHANGEFLAG_URL_REDIRECT		0x40000000

typedef enum
	{
	METHOD_GET,
	METHOD_HEAD,
	METHOD_POST
	} METHOD_TYPE;

typedef enum
	{
	LOAD_STATE_NOT_LOADED,
	LOAD_STATE_LOADING,
	LOAD_STATE_ABORTED,
	LOAD_STATE_COMPLETE
	} LOAD_STATE;

class CProtocol;
class CDynamicLoad;

class CNotifyObject
	{
public:
	CNotifyObject();
	virtual ~CNotifyObject();

	POSITION GetFirstDynamicLoadPosition() const
		{ return m_dynamic_objects.GetHeadPosition(); }
	CDynamicLoad *GetNextDynamicLoad(POSITION& walk) const
		{ return (CDynamicLoad *)m_dynamic_objects.GetNext(walk); }

// required override
	virtual void OnNotify(UINT32 notify_flags, CDynamicLoad *dlobject ) = 0;
		// This function is called by DoNotifies and allows the notify object to react to 
		// a change of the dynamic load object

private:
	CPtrList m_dynamic_objects;

	friend class CDynamicLoad;
	};

class CDynamicLoad : public CObject
	{
public:
	// public constructor and destructor
	CDynamicLoad( CNotifyObject *notify, LPCSTR url, METHOD_TYPE method = METHOD_GET, LPCSTR post_headers = NULL, LPCSTR post_data = NULL);
	virtual ~CDynamicLoad();

// The following functions can be called by the UI thread at any time.

	void StartLoading();
		// should be called immediately after construction to begin the loading process in the background
	void AbortLoading();
		// terminates background loading for the object if it hasn't finished loading yet
	virtual void ClearCacheEntry();
		// removes an item 
	void SetProgressMessage( LPCSTR message );

static void InitializeDynamicLoader();
		// call this function before calling any other functions

static CString DoNotifies();
		// The function will call the notify objects when changes occur. Always call this from the UI thread.
		// Returns a progress message

static void CloseDynamicLoader();
		// Call this to clean up after the dynamic loader

static void AbortAllDynamicLoading();
		// Aborts all objects that are currently dynamically loading

static BOOL IsAnyDynamicLoading();
		// returns TRUE if any objects are dynamically loading

// The following functions lock the object to keep it in a consistent state during a read or write

public:

#ifdef _DEBUG
	void Lock(LPCSTR file, INT line_number);
#else
	void Lock();
#endif

	void Unlock();

#ifdef _DEBUG
	void AssertLocked() const;
		// Asserts if the object is locked.
#endif

// The following functions return information about the object. In order to maintain a consistent
// state, the object must be locked for these functions to work reliably.

public:
	LOAD_STATE GetLoadState() const;
		// returns the state of the loaded object
	CString GetURL() const;
		// returns the current URL
	void SetErrorMessage( UINT message_id, LPCSTR param1 = NULL, LPCSTR param2 = NULL);
		// Sets the error message to the specified message ID with the given parameters substituted
		// for %s just like printf
	CString GetErrorMessage() const;
		// Returns the current error message. Empty if there are no errors

	// The following functions return information on the request type
	METHOD_TYPE GetMethodType() const;
	CString GetPostHeaders() const;
	CString GetPostData() const;

protected:
	/* These are functions that can be called by the required overridables */

	void Notify( UINT32 notify_flags );

protected:
	/* required overridables. These functions may be called from within another thread, so in order
	   to keep the object in a consistent state, the dynamic loader will lock the m_access_mutex before
	   calling the overrideables and unlock it afterwords. */

	virtual LOAD_STATE OnPreLoading();
	/* called before the actual network connection is established. Allows the dynamic load 
	   object to immediately return LOAD_STATE_COMPLETE or LOAD_STATE_ABORTED before any work is done

	   This function may be called multiple times if a redirect happens.
	*/ 

	virtual LOAD_STATE OnBeginLoading( const CMapStringToString& mime_header );
	/* called after the network connection has been established and the mime header has been
	   read. By returning LOAD_STATE_COMPLETE or LOAD_STATE_ABORTED, the object can abort the
	   downloading of the actual data of the URL. 
	*/

	virtual LOAD_STATE OnLoading( LPCBYTE buffer, INT32 amount ) = 0;
	/* called whenever data arrives from the network. This function may be called many times
	   before the object has finished loading */

	virtual LOAD_STATE OnEndLoading() = 0;
	/* this function is aways called when the dynamic loader decides that no more loading is
	   possible for this object. Several conditions can cause this:

	   1) One of the OnPreLoading, OnBeginLoading, OnLoading returns LOAD_STATE_COMPLETE or
	      LOAD_STATE_ABORTED, in which case m_load_state is set to the respective return 
		  value and OnEndLoading is called.

	   2) The network connection is broken, or there is an error reading a file, in which case
	      m_load_state is set to LOAD_STATE_ABORTED and OnEndLoading is called.

	   3) StopLoading is called, in which case m_load_state is set to LOAD_STATE_ABORTED and 
	      OnEndLoading is called.
	 */
	   
private:
	void InvokeLoadingThread();

	typedef enum	
		{
		LIST_NONE,
		LIST_NEW,
		LIST_LOADING,
		LIST_DONE,
		LIST_DONE_NOTIFY
		} LIST_TYPE;

	CNotifyObject *m_notify_object;		// never changes during lifetime of object

	CProtocol *m_loading_protocol;
	CAccessLock m_access_lock;

	LIST_TYPE m_current_list;
	POSITION m_list_position, m_notify_object_list_pos;

	METHOD_TYPE m_method;
	CString m_url;
	CString m_post_headers, m_post_data;
	
	CString m_error_message, m_progress_message;

	LOAD_STATE m_load_state;

	UINT32 m_notify_flags;

	friend class CProtocol;

	DECLARE_DYNAMIC( CDynamicLoad )
	};

CString CombineURL( LPCSTR url1, LPCSTR url2);
	// Combines an absolute and a relative url into one new URL

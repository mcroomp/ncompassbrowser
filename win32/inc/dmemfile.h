#define _DMEMFILE_H_

#ifndef _DYNLOAD_H_
#include "dynload.h"
#endif

#ifndef _MIMELOAD_H_
#include "mimeload.h"
#endif

#ifndef _BIGSTR_H_
#include "bigstr.h"
#endif

#define CHANGEFLAG_FILE_GROW 0x00000001

class CDynLoadMemFile : public CDynamicLoad
	{
public:
	CDynLoadMemFile(CNotifyObject *notify, LPCSTR url, METHOD_TYPE method = METHOD_GET, LPCSTR post_headers = NULL, LPCSTR post_data = NULL);

	INT32 GetFileSize();
	void ReadData(LPBYTE buffer, INT32 start_offset, INT32 amount);

	// these can be called ONLY when the file has finished loading
	CFile * GetFile();
	void ReleaseFile();

	BOOL IsMimeHeaderLoaded();
	void GetMimeHeader( CMapStringToString& mime_header );
		
protected:
// overrides
	virtual LOAD_STATE OnBeginLoading( const CMapStringToString & map);
	virtual LOAD_STATE OnLoading(LPCBYTE buffer, INT32 buffer_size);
	virtual LOAD_STATE OnEndLoading();

private:
	CMemFile m_memfile;
	CString m_mime_type;
	BOOL m_locked;

	BOOL m_mime_header_loaded;
	CMapStringToString m_mime_header;

	CAccessLock m_access_lock;

	DECLARE_DYNAMIC( CDynLoadMemFile );
	};

class CMimeLoadMemFile : public CMimeObject
	{
public:
	CMimeLoadMemFile(LPCSTR url, LPCSTR mime_type);

// locks required
	INT32 GetFileSize() const;
	void ReadData(LPBYTE buffer, INT32 start_offset, INT32 amount) const;
	
protected:
// overrides
	virtual LOAD_STATE OnReadData(LPCBYTE buffer, INT32 buffer_size);
	virtual LOAD_STATE OnEndOfFile();

	virtual void LaunchViewer();
	virtual BOOL UsesInternalViewer() const;
	
private:
	CBigString m_data;
	};

#define _DDSKFILE_H_

#ifndef _DYNLOAD_H_
#include "dynload.h"
#endif

#define CHANGEFLAG_FILE_GROW 0x00000001

class CDynLoadDiskFile : public CDynamicLoad
{
public:
	CDynLoadDiskFile(CNotifyObject *notify, LPCSTR url, LPCTSTR target_filename,
		METHOD_TYPE method = METHOD_GET, LPCSTR post_headers = NULL, LPCSTR post_data = NULL);

// lock are required to call these functions
	BOOL IsMimeHeaderLoaded() const;
	void GetMimeHeader( CMapStringToString& mime_header ) const;

protected:
// overrides
	virtual LOAD_STATE OnBeginLoading( const CMapStringToString & map);
	virtual LOAD_STATE OnLoading(LPCBYTE buffer, INT32 buffer_size);
	virtual LOAD_STATE OnEndLoading();

private:
	CFile* 					m_diskFile;
	CString					m_fileName;
	CString 				m_mime_type;
	BOOL 					m_locked;

	BOOL					m_mime_header_loaded;
	CMapStringToString 		m_mime_header;
};

CString			ExtractFileNameFromURL(CString url);

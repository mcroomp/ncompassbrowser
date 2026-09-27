#include "cross_p.h"
#include "ddskfile.h"

#ifndef _VIEWHTML_H_
#include "viewhtml.h"
#endif


CString	ExtractFileNameFromURL(CString url)
{
	CString 	fileName;
 	int			index;

	index = url.ReverseFind('/');
	if (index == -1)
	{
		index  = url.ReverseFind('\\');
		if (index == -1)
		{
			index = url.ReverseFind(':');
			if (index == -1)
				return url;
		}
	}
	if (index == url.GetLength())
	{
		url = url.Left(index - 1);
		fileName = ExtractFileNameFromURL(url);
		return fileName;
	}
	fileName = url.Mid(index + 1);
	return fileName; 		
}

CDynLoadDiskFile::CDynLoadDiskFile(CNotifyObject *notify, LPCSTR url, LPCTSTR fileName, METHOD_TYPE method, LPCSTR post_headers, LPCSTR post_data )
	: CDynamicLoad( notify, url, method, post_headers, post_data )
	{
	m_locked = FALSE;
	m_mime_header_loaded = FALSE;

	if (fileName == NULL)
		{
		m_fileName = ExtractFileNameFromURL(CString(url));
		m_fileName = ((CViewhtmlApp *) AfxGetApp())->m_tempStorageDir + m_fileName;
		}
	else
		{
		m_fileName = CString(fileName);
		}

	m_diskFile = new CFile(m_fileName, CFile::modeCreate | CFile::modeWrite);
	}


LOAD_STATE CDynLoadDiskFile::OnBeginLoading( const CMapStringToString& mime_type )
	{
	DEBUG_LOCK();

	POSITION walk = mime_type.GetStartPosition();
	while(walk)
	{
		CString str1, str2;

		mime_type.GetNextAssoc(walk, str1, str2);
		m_mime_header.SetAt(str1, str2);
	}
	m_mime_header_loaded = TRUE;
	
	Unlock();

	return LOAD_STATE_LOADING;
	}	

LOAD_STATE CDynLoadDiskFile::OnLoading(LPCBYTE buffer, INT32 buffer_size)
	{
	DEBUG_LOCK();
	m_diskFile->SeekToEnd();
	m_diskFile->Write(buffer, static_cast<UINT>(buffer_size));
	Notify( CHANGEFLAG_FILE_GROW );
	Unlock();

	return LOAD_STATE_LOADING;
	}

LOAD_STATE CDynLoadDiskFile::OnEndLoading()
	{
	DEBUG_LOCK();
	Notify( CHANGEFLAG_DONE );
	LOAD_STATE l = GetLoadState();
	Unlock();

	if (m_diskFile != NULL)
		{
		delete m_diskFile;
	 	m_diskFile = NULL;
		}

	if ( l == LOAD_STATE_LOADING || l == LOAD_STATE_COMPLETE)
		return LOAD_STATE_COMPLETE;
	else
		return LOAD_STATE_ABORTED;
	}

BOOL CDynLoadDiskFile::IsMimeHeaderLoaded() const
	{
	ASSERT_LOCKED(this);

	return m_mime_header_loaded;
	}
	
void CDynLoadDiskFile::GetMimeHeader( CMapStringToString& mime_header ) const
	{
	ASSERT_LOCKED(this);
	
	ASSERT(m_mime_header_loaded == TRUE);

	POSITION walk = m_mime_header.GetStartPosition();
	while(walk)
		{
		CString str1, str2;

		m_mime_header.GetNextAssoc(walk, str1, str2);
		mime_header.SetAt(str1, str2);
		}
	}
	

#include "cross_p.h"
#include "resource.h"

#include "dmemfile.h"

IMPLEMENT_DYNAMIC( CDynLoadMemFile, CDynamicLoad );

CDynLoadMemFile::CDynLoadMemFile(CNotifyObject *notify, LPCSTR url, METHOD_TYPE method, LPCSTR post_headers, LPCSTR post_data)
	: CDynamicLoad( notify, url, method, post_headers, post_data), m_memfile(1024)
	{
	m_locked = FALSE;
	m_mime_header_loaded = FALSE;
	}

LOAD_STATE CDynLoadMemFile::OnBeginLoading( const CMapStringToString& mime_type )
	{
	ASSERT_WORKER_THREAD();

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

LOAD_STATE CDynLoadMemFile::OnLoading(LPCBYTE buffer, INT32 buffer_size)
	{
	ASSERT_WORKER_THREAD();

	DEBUG_LOCK();

	m_memfile.SeekToEnd();
	m_memfile.Write(buffer, buffer_size);
	
	Notify( CHANGEFLAG_FILE_GROW );
	Unlock();

	return LOAD_STATE_LOADING;
	}

LOAD_STATE CDynLoadMemFile::OnEndLoading()
	{
	ASSERT_WORKER_THREAD();

	DEBUG_LOCK();
	Notify( CHANGEFLAG_DONE );
	LOAD_STATE l = GetLoadState();
	Unlock();

	if ( l == LOAD_STATE_LOADING || l == LOAD_STATE_COMPLETE )
		return LOAD_STATE_COMPLETE;
	else
		return LOAD_STATE_ABORTED;
	}

CFile * CDynLoadMemFile::GetFile()
	{
#ifdef _DEBUG
	DEBUG_LOCK();
	LOAD_STATE l = GetLoadState();
	Unlock();
	ASSERT( l == LOAD_STATE_COMPLETE || l == LOAD_STATE_ABORTED);
	ASSERT(!m_locked);
#endif
	m_locked = TRUE;
	return &m_memfile;
	}

void CDynLoadMemFile::ReleaseFile()
	{
	ASSERT(m_locked);
	m_locked = FALSE;
	}

INT32 CDynLoadMemFile::GetFileSize()
	{
	DEBUG_LOCK();
	INT32 filesize = m_memfile.GetLength();
	Unlock();

	return filesize;
	}

void CDynLoadMemFile::ReadData(LPBYTE buffer, INT32 start_offset, INT32 amount)
	{
	DEBUG_LOCK();
	m_memfile.Seek( start_offset, CFile::begin );
	m_memfile.Read( buffer, amount );
	Unlock();	
	}

BOOL CDynLoadMemFile::IsMimeHeaderLoaded()
	{
	m_access_lock.DEBUG_LOCK();
	BOOL mime_header_loaded = m_mime_header_loaded;
	m_access_lock.Unlock();
	return mime_header_loaded;
	}

void CDynLoadMemFile::GetMimeHeader( CMapStringToString& mime_header )
	{
	m_access_lock.DEBUG_LOCK();

	ASSERT(m_mime_header_loaded == TRUE);

	POSITION walk = m_mime_header.GetStartPosition();
	while(walk)
		{
		CString str1, str2;

		m_mime_header.GetNextAssoc(walk, str1, str2);
		mime_header.SetAt(str1, str2);
		}

	m_access_lock.Unlock();
	}					
/* loader used to load MIME types */


CMimeLoadMemFile::CMimeLoadMemFile(LPCSTR url, LPCSTR mime_type)
	: CMimeObject( MIME_OBJECT_MEMFILE, url, mime_type )
	{
	}

LOAD_STATE CMimeLoadMemFile::OnReadData(LPCBYTE buffer, INT32 buffer_size)
	{
	ASSERT_WORKER_THREAD();

	DEBUG_LOCK();
	m_data.AppendData(buffer, buffer_size );
	Notify( CHANGEFLAG_FILE_GROW );
	Unlock();

	return LOAD_STATE_LOADING;
	}

LOAD_STATE CMimeLoadMemFile::OnEndOfFile()
	{
	ASSERT_WORKER_THREAD();

	DEBUG_LOCK();
	LOAD_STATE l = GetLoadState();
	Unlock();

	if ( l == LOAD_STATE_LOADING || l == LOAD_STATE_COMPLETE )
		return LOAD_STATE_COMPLETE;
	else
		return LOAD_STATE_ABORTED;
	}

INT32 CMimeLoadMemFile::GetFileSize() const
	{
	ASSERT_LOCKED(this);

	return m_data.GetLength();		
	}

void CMimeLoadMemFile::ReadData(LPBYTE buffer, INT32 start_offset, INT32 amount) const
	{
	ASSERT_LOCKED(this);

	m_data.GetData(buffer, start_offset, amount );
	}

BOOL CMimeLoadMemFile::UsesInternalViewer() const
	{
	return FALSE;
	}

void CMimeLoadMemFile::LaunchViewer()
	{
	CDialog save_dialog( IDD_SAVE_URL );

	if (save_dialog.DoModal() == IDOK)
		{
		CString s = GetURL();
		
		LPCSTR slash = strrchr(s, '/');
		if (!slash)
			slash = s;
		else
			slash++;

		CString newName = slash;

		if (!AfxGetApp()->DoPromptFileName(newName, AFX_IDS_SAVEFILE,
		 	OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST, FALSE, NULL))
			return; // save cancelled

		ASSERT( GetLoadState() == LOAD_STATE_COMPLETE );
		
		INT32 filesize = GetFileSize();
		 
		CFile output;
		CFileException file_exception;

		if (output.Open( newName, CFile::modeCreate	| CFile::modeWrite, &file_exception ) == 0)
			{
			CString error_message;

			error_message.Format("The file \"%s\" could not be opened for writing.", (LPCSTR)newName);
			AfxMessageBox(error_message);
			return;
			}

#define BUFFER_SIZE 4096

		BYTE buffer[BUFFER_SIZE];
		INT32 amount_left = filesize;
		INT32 current_position = 0;

		TRY
			{
			while( amount_left > 0)
				{
				INT32 buf_size;

				if (amount_left > BUFFER_SIZE)
					buf_size = BUFFER_SIZE;
				else
					buf_size = amount_left;
			
				m_data.GetData(buffer, current_position, buf_size );
				output.Write(buffer, buf_size);

				amount_left -= buf_size;
				current_position += buf_size;
				}
			}
		CATCH( CFileException, e )
			{
			AfxMessageBox("An error occured while writing to the disk. The file may not have been saved correctly.");
			return;
			}
		END_CATCH
		}
	}

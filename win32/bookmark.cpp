#include "cross_p.h"
#include "resource.h"

#ifndef _BOOKMARK_H_
#include "bookmark.h"
#endif

CBookmarkLibrary::CBookmarkLibrary()
	{
	}

CBookmarkLibrary::~CBookmarkLibrary()
	{
	}

CBookmarkLibrary& CBookmarkLibrary::operator = (const CBookmarkLibrary& source )
	{
	m_title_array.RemoveAll();
	m_url_array.RemoveAll();

	INT i,ilen = source.m_title_array.GetSize();
	for(i=0;i<ilen;i++)
		{
		m_title_array.Add( source.m_title_array[i] );
		m_url_array.Add( source.m_url_array[i] );
		}
	return *this;
	}

void CBookmarkLibrary::AddBookmark( LPCSTR title, LPCSTR url )
	{
	m_title_array.Add( title );
	m_url_array.Add( url );
	}

void CBookmarkLibrary::DeleteBookmark( INT index )
	{
	m_title_array.RemoveAt( index );
	m_url_array.RemoveAt( index );
	}

void CBookmarkLibrary::ChangeBookmark( INT index, LPCSTR new_title, LPCSTR new_url )
	{
	m_title_array.SetAt( index, new_title );
	m_url_array.SetAt( index, new_url );
	}

void CBookmarkLibrary::MoveBookmarkUp( INT index )
	{
	CString title = m_title_array[index];
	CString url = m_url_array[index];

	m_title_array.RemoveAt( index );
	m_url_array.RemoveAt( index );

	m_title_array.InsertAt( index-1, title );
	m_url_array.InsertAt( index-1, url );
	}

void CBookmarkLibrary::MoveBookmarkDown( INT index )
	{
	CString title = m_title_array[index];
	CString url = m_url_array[index];

	m_title_array.RemoveAt( index );
	m_url_array.RemoveAt( index );

	m_title_array.InsertAt( index+1, title );
	m_url_array.InsertAt( index+1, url );
	}

INT CBookmarkLibrary::GetBookmarkCount() const
	{
	return m_title_array.GetSize();
	}

CString CBookmarkLibrary::GetBookmarkURL(INT index)
	{
	return m_url_array[index];
	}

CString CBookmarkLibrary::GetBookmarkTitle(INT index)
	{
	return m_title_array[index];
	}

void CBookmarkLibrary::StoreBookmarks(LPCTSTR filename)
	{
	TRY 
		{
		CFile file( filename, CFile::modeCreate | CFile::modeWrite );
		CArchive archive( &file, CArchive::store );

		DWORD i, ilen = m_title_array.GetSize();

		archive << ilen;

		for(i=0;i<ilen;i++)
			{
			archive << m_url_array[i];
			archive << m_title_array[i];
			}
		archive.Close();
		}
	CATCH( CException, e )
		{
		CString err;
		err.Format("The bookmark file %s could not be written to.", filename );
		AfxMessageBox(err);
		}
	END_CATCH
	}

void CBookmarkLibrary::LoadBookmarks(LPCTSTR filename)
	{
	TRY 
		{
		CFile file;
		CFileException exception;

		if (file.Open( filename, CFile::modeRead, &exception ) == 0)
			return;		// do nothing on an not found error

		CArchive archive( &file, CArchive::load );

		DWORD i, ilen;

		m_url_array.RemoveAll();
		m_title_array.RemoveAll();

		archive >> ilen;

		for(i=0;i<ilen;i++)
			{
			CString str;
			archive >> str;
			m_url_array.Add(str);
			archive >> str;
			m_title_array.Add(str);
			}
		archive.Close();
		}
	CATCH( CException, e )
		{
		CString err;
		err.Format("The bookmark file %s could not be loaded.", filename );
		AfxMessageBox(err);
		}
	END_CATCH
	}

/////////////////////////////////////////////////////////////////////////////
// CDialogEditBookmark dialog


CDialogEditBookmark::CDialogEditBookmark(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogEditBookmark::IDD, pParent)
{
	m_new_url = FALSE;
	//{{AFX_DATA_INIT(CDialogEditBookmark)
	m_edit_title = _T("");
	m_edit_url = _T("");
	//}}AFX_DATA_INIT
}


void CDialogEditBookmark::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogEditBookmark)
	DDX_Text(pDX, IDC_EDIT_TITLE, m_edit_title);
	DDX_Text(pDX, IDC_EDIT_URL, m_edit_url);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogEditBookmark, CDialog)
	//{{AFX_MSG_MAP(CDialogEditBookmark)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDialogEditBookmark message handlers


/////////////////////////////////////////////////////////////////////////////
// CDialogViewBookmarks dialog


CDialogViewBookmarks::CDialogViewBookmarks(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogViewBookmarks::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogViewBookmarks)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDialogViewBookmarks::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogViewBookmarks)
	DDX_Control(pDX, IDC_LIST_URL, m_current_url);
	DDX_Control(pDX, IDC_BOOKMARK_LIST, m_list_control);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogViewBookmarks, CDialog)
	//{{AFX_MSG_MAP(CDialogViewBookmarks)
	ON_LBN_SELCHANGE(IDC_BOOKMARK_LIST, OnSelchangeBookmarkList)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_ITEM, OnButtonDeleteItem)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_ITEM, OnButtonEditItem)
	ON_BN_CLICKED(IDC_BUTTON_MOVE_DOWN, OnButtonMoveDown)
	ON_BN_CLICKED(IDC_BUTTON_MOVE_UP, OnButtonMoveUp)
	ON_BN_CLICKED(IDC_BUTTON_NEW_ITEM, OnButtonNewItem)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_URL, OnButtonOpenUrl)
	ON_LBN_DBLCLK(IDC_BOOKMARK_LIST, OnDblclkBookmarkList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDialogViewBookmarks message handlers

BOOL CDialogViewBookmarks::OnInitDialog() 
	{
	CDialog::OnInitDialog();
	
	INT i,ilen = m_bookmark_library.GetBookmarkCount();
	for(i=0;i<ilen;i++)
		{
		m_list_control.AddString( m_bookmark_library.GetBookmarkTitle(i));
		}
	if (ilen > 0)
		{
		m_list_control.SetCurSel(0);
		OnSelchangeBookmarkList();
		}

	UpdateUI();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
	}


void CDialogViewBookmarks::OnSelchangeBookmarkList() 
	{
	INT sel = m_list_control.GetCurSel();
	if (sel == LB_ERR)
		return;

	UpdateUI();
	}

void CDialogViewBookmarks::OnButtonDeleteItem() 
{
	INT sel = m_list_control.GetCurSel();
	if (sel == LB_ERR)
		return;

	m_list_control.DeleteString( sel );
	m_bookmark_library.DeleteBookmark( sel );

	UpdateUI();
}


void CDialogViewBookmarks::OnButtonEditItem() 
	{
	INT sel = m_list_control.GetCurSel();
	if (sel == LB_ERR)
		return;

	CDialogEditBookmark dialog;

	dialog.m_edit_title = m_bookmark_library.GetBookmarkTitle(sel);
	dialog.m_edit_url = m_bookmark_library.GetBookmarkURL(sel);

	if (dialog.DoModal() != IDOK)
		return;
	
	m_bookmark_library.ChangeBookmark(sel, dialog.m_edit_title, dialog.m_edit_url );

	m_list_control.DeleteString( sel );
	m_list_control.InsertString( sel, dialog.m_edit_title);
	m_list_control.SetCurSel( sel );

	UpdateUI();
	}

void CDialogViewBookmarks::OnButtonMoveDown() 
	{
	INT sel = m_list_control.GetCurSel();
	if (sel == LB_ERR)
		return;

	m_list_control.DeleteString( sel );
	m_list_control.InsertString( sel+1, m_bookmark_library.GetBookmarkTitle(sel));
	m_list_control.SetCurSel( sel+1 );
	
	m_bookmark_library.MoveBookmarkDown(sel);

	UpdateUI();
	}

void CDialogViewBookmarks::OnButtonMoveUp() 
	{
	INT sel = m_list_control.GetCurSel();
	if (sel == LB_ERR)
		return;

	m_list_control.DeleteString( sel );
	m_list_control.InsertString( sel-1, m_bookmark_library.GetBookmarkTitle(sel) );
	m_list_control.SetCurSel( sel-1 );
	
	m_bookmark_library.MoveBookmarkUp(sel);

	UpdateUI();
	}

void CDialogViewBookmarks::OnButtonNewItem() 
	{
	CDialogEditBookmark dialog;

	dialog.m_edit_title = m_input_title;
	dialog.m_edit_url = m_input_url;
	dialog.m_new_url = TRUE;
	
	if (dialog.DoModal() != IDOK)
		return;

	m_bookmark_library.AddBookmark(dialog.m_edit_title, dialog.m_edit_url );
	m_list_control.AddString( dialog.m_edit_title );

	UpdateUI();
	}

void CDialogViewBookmarks::UpdateUI()
	{
	INT sel = m_list_control.GetCurSel();
	if (sel == LB_ERR)
		{
		GetDlgItem( IDC_BUTTON_MOVE_UP )->EnableWindow(FALSE);
		GetDlgItem( IDC_BUTTON_MOVE_DOWN )->EnableWindow(FALSE);
		GetDlgItem( IDC_BUTTON_EDIT_ITEM )->EnableWindow(FALSE);
		GetDlgItem( IDC_BUTTON_DELETE_ITEM )->EnableWindow(FALSE);
		GetDlgItem( IDC_BUTTON_OPEN_URL)->EnableWindow(FALSE);
		SetDlgItemText( IDC_LIST_URL, "");
		return;
		}

	GetDlgItem( IDC_BUTTON_MOVE_UP )->EnableWindow( sel > 0);
	GetDlgItem( IDC_BUTTON_MOVE_DOWN )->EnableWindow( sel < m_bookmark_library.GetBookmarkCount()-1 );
	GetDlgItem( IDC_BUTTON_EDIT_ITEM )->EnableWindow(TRUE);
	GetDlgItem( IDC_BUTTON_DELETE_ITEM )->EnableWindow(TRUE);
	GetDlgItem( IDC_BUTTON_OPEN_URL)->EnableWindow(TRUE);
	SetDlgItemText( IDC_LIST_URL, m_bookmark_library.GetBookmarkURL(sel));
	}


void CDialogViewBookmarks::OnButtonOpenUrl() 
{
	INT sel = m_list_control.GetCurSel();
	if (sel == LB_ERR)
		return;

	m_output_url = m_bookmark_library.GetBookmarkURL(sel);
	EndDialog( IDC_BUTTON_OPEN_URL );
}

BOOL CDialogEditBookmark::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if (m_new_url)
		SetWindowText("New Bookmark");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDialogViewBookmarks::OnDblclkBookmarkList() 
{
	OnButtonOpenUrl();
}

// viewhtml.cpp : Defines the class behaviors for the application.
//

#include "cross_p.h"
#include "afxpriv.h"		// For CRecentFileList

#include <direct.h>
#ifdef _DEBUG
#include <crtdbg.h>
#endif

#include "viewhtml.h"
#include "mainfrm.h"
#include "viewhdoc.h"
#include "viewhvw.h"

#include "dopenurl.h"

#ifndef _DLGPREF_H_
#include "dlgpref.h"
#endif

#ifndef _DYNLOAD_H_
#include "dynload.h"
#endif

#ifndef _PROTOCOL_H_
#include "protocol.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define BOOKMARK_FILENAME "bookmark.dat"

#ifdef _DEBUG
static int __cdecl CrashOnAssertion(int report_type, char *, int *)
	{
	if (report_type == _CRT_ASSERT)
		RaiseException(0xE0421000, EXCEPTION_NONCONTINUABLE, 0, NULL);
	return FALSE;
	}
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewhtmlApp

BEGIN_MESSAGE_MAP(CViewhtmlApp, CWinApp)
	//{{AFX_MSG_MAP(CViewhtmlApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_OPEN_URL, OnFileOpenUrl)
	ON_COMMAND(ID_GO_BACK, OnGoBack)
	ON_UPDATE_COMMAND_UI(ID_GO_BACK, OnUpdateGoBack)
	ON_UPDATE_COMMAND_UI(ID_GO_FORWARD, OnUpdateGoForward)
	ON_COMMAND(ID_GO_FORWARD, OnGoForward)
	ON_COMMAND(ID_GO_HOME, OnGoHome)
	ON_UPDATE_COMMAND_UI(ID_GO_HOME, OnUpdateGoHome)
	ON_COMMAND(ID_ADD_BOOKMARK, OnAddBookmark)
	ON_UPDATE_COMMAND_UI(ID_ADD_BOOKMARK, OnUpdateAddBookmark)
	ON_COMMAND(ID_EDIT_BOOKMARKS, OnEditBookmarks)
	ON_UPDATE_COMMAND_UI(ID_BOOKMARK00, OnUpdateBookmarks)
	ON_COMMAND(ID_OPTIONS_PREFERENCES, OnOptionsPreferences)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	ON_COMMAND_EX_RANGE(ID_BOOKMARK00, ID_BOOKMARK19, OnSelectBookmark)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewhtmlApp construction

CViewhtmlApp::CViewhtmlApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CViewhtmlApp object

CViewhtmlApp theApp;

// This identifier was generated to be statistically unique for your app.
// You may change it if you prefer to choose a specific identifier.
static const CLSID BASED_CODE clsid =
{ 0x3b463ef0, 0xafee, 0x11ce, { 0x8f, 0x64, 0x0, 0x60, 0x8c, 0x53, 0xd2, 0x63 } };


/////////////////////////////////////////////////////////////////////////////
// CViewhtmlApp initialization

BOOL CViewhtmlApp::InitInstance()
{
#ifdef _DEBUG
	_CrtSetReportHook(CrashOnAssertion);
	SetUIThreadID();
#endif

	SetRegistryKey("ExCITE");
	// Read initial registry entries
	ReadRegistryInfo();

	m_notify_event = CreateEvent(NULL, FALSE, TRUE, NULL);

	m_viewclass = AfxRegisterWndClass( CS_HREDRAW|CS_VREDRAW );
		
	char buffer[256];
	m_programDir = getcwd(buffer, 256);
	if (m_programDir[ m_programDir.GetLength()-1] != '\\')
		m_programDir += '\\';

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	// default to 4 simultaious connections
	m_max_connections = 4;
	
#ifdef _DEBUG
	afxDump.SetDepth(2);
#endif

	// initially there is one disabled entry

	m_last_menu_size = 1;

	m_bookmarks.LoadBookmarks(theApp.m_programDir + BOOKMARK_FILENAME );


	CDynamicLoad::InitializeDynamicLoader();
	CMimeDynamicLoad::OpenMimeLoader();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	Enable3dControls();

	LoadStdProfileSettings(0);  // Load standard INI file options (including MRU)

	//m_pRecentFileList = new CURLRecentFileList(0, szFileSection, szFileEntry,
//		0);

	//m_pRecentFileList->ReadList();

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	AfxSocketInit();

	
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = DEBUG_NEW CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CViewhtmlDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CViewhtmlView));
	pDocTemplate->SetContainerInfo(IDR_CNTR_INPLACE);
	AddDocTemplate(pDocTemplate);

	// Connect the COleTemplateServer to the document template.
	//  The COleTemplateServer creates new documents on behalf
	//  of requesting OLE containers by using information
	//  specified in the document template.
	m_server.ConnectTemplate(clsid, pDocTemplate, TRUE);
		// Note: SDI applications register server objects only if /Embedding
		//   or /Automation is present on the command line.

	// Parse the command line to see if launched as OLE server
	if (RunEmbedded() || RunAutomated())
	{
		// Register all OLE server (factories) as running.  This enables the
		//  OLE libraries to create objects from other applications.
		COleTemplateServer::RegisterAll();

		// Application was run with /Embedding or /Automation.  Don't show the
		//  main window in this case.
		return TRUE;
	}

	// When a server application is launched stand-alone, it is a good idea
	//  to update the system registry in case it has been damaged.
	m_server.UpdateRegistry(OAT_DISPATCH_OBJECT);
	COleObjectFactory::UpdateRegistryAll();

	// create a new (empty) document
	OnFileNew();

	// m_viewhtml_doc and m_viewhtml_view are automatically initialized
	
	if (m_lpCmdLine[0] != '\0')
		{
		OpenDocumentFile(m_lpCmdLine);
		}
	else
		{
		if (m_bLoadHomePageOnStartup)
			OpenDocumentFile(m_HomePage);
		}

	return TRUE;
}

void CViewhtmlApp::ReadRegistryInfo()
{
	// directory info
	m_ocxStorageDir = GetProfileString("Directories", "Ocx Storage", "C:\\temp\\");
	m_tempStorageDir = GetProfileString("Directories", "Temp Storage", "C:\\temp\\");

	// user info
	m_UserName = GetProfileString("UserInfo", "UserName", "");
	m_UserEmail = GetProfileString("UserInfo", "UserEmail", "");
	m_HomePage = GetProfileString("UserInfo", "HomePage", "http://oberon.educ.sfu.ca/NCompass/intro.htm");
	m_bLoadHomePageOnStartup = GetProfileInt("UserInfo", "LoadHomePage", TRUE);

	// network info
	m_SmtpServer = GetProfileString("Network", "SmtpServer", "");
	m_HttpProxy = GetProfileString("Network", "HttpProxy", "");
	m_HttpProxyPort = GetProfileInt("Network", "HttpProxyInt", 80);
	m_max_connections = GetProfileInt("Network", "MaxConnections", 4);

}

void CViewhtmlApp::WriteRegistryInfo()
	{
	// directory info
	WriteProfileString("Directories", "Ocx Storage", m_ocxStorageDir);
	WriteProfileString("Directories", "Temp Storage", m_tempStorageDir);

	// user info
	WriteProfileString("UserInfo", "UserName", m_UserName);
	WriteProfileString("UserInfo", "UserEmail", m_UserEmail);
	WriteProfileString("UserInfo", "HomePage", m_HomePage);
	WriteProfileInt("UserInfo", "LoadHomePage", m_bLoadHomePageOnStartup);

	// network info
	WriteProfileString("Network", "SmtpServer", m_SmtpServer);
	WriteProfileString("Network", "HttpProxy", m_HttpProxy);
	WriteProfileInt("Network", "HttpProxyPort", m_HttpProxyPort);
	WriteProfileInt("Network", "MaxConnections", m_max_connections);
	}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CViewhtmlApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CViewhtmlApp commands

int CViewhtmlApp::ExitInstance() 
	{
	CMimeDynamicLoad::CloseMimeLoader();
	CDynamicLoad::CloseDynamicLoader();

	CloseHandle(m_notify_event);

	return CWinApp::ExitInstance();
	}

int ErrorExit(const char *string)
	{
	TRACE1("Fatal Error: %s", string);
	ASSERT(FALSE);
	return 0;
	}

void CViewhtmlApp::SignalNotify()
	{
	SetEvent(m_notify_event);
	}

CString CViewhtmlApp::GetCurrentURL()
	{
	CString  url;

	if (m_viewhtml_doc->GetMimeObject())
		url = m_viewhtml_doc->GetMimeObject()->GetURL();
	
	return url;
	}
	
void CViewhtmlApp::OnFileOpen() 
	{
	// prompt the user (with all document templates)
	CString newName;
	if (!DoPromptFileName(newName, AFX_IDS_OPENFILE,
	  OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, TRUE, NULL))
		return; // open cancelled

	CString oldurl = GetCurrentURL();

	CString url = ConvertFilenameToURL(newName);
	OpenDocumentFile( url );
	}

void CViewhtmlApp::OnFileOpenUrl() 
	{
	CDialogOpenURL dialog;

	if (dialog.DoModal()==IDOK)
		{
		OpenDocumentFile( dialog.m_url );
		}
	}

/* overriden in order to stop it from trying to parse URLs */

CDocument* CViewhtmlApp::OpenDocumentFile(LPCTSTR lpszFileName)
	{
	CString url = lpszFileName;

	if (strcmp(lpszFileName, "back" )==0)
		{
		if (m_back_array.GetSize() == 0)
			return NULL;

		INT index = m_back_array.GetUpperBound();
		url = m_back_array.GetAt( index );

		m_viewhtml_doc->m_add_to_back_array = FALSE;
		
		m_back_array.RemoveAt(index,1);
		m_forward_array.Add( GetCurrentURL() );
		}
	else if (strcmp(lpszFileName, "forward" )==0)
		{
		if (m_forward_array.GetSize() == 0)
			return NULL;

		INT index = m_forward_array.GetUpperBound();
		url = m_forward_array.GetAt( index );

		m_viewhtml_doc->m_add_to_back_array = FALSE;
		
		m_forward_array.RemoveAt(index,1);
		m_back_array.Add( GetCurrentURL() );
		}
	else
		{
		CString oldurl = GetCurrentURL();
		m_viewhtml_doc->m_add_to_back_array = TRUE;
		
		url = lpszFileName;
		}

	m_viewhtml_doc->OpenURL( url );
	return m_viewhtml_doc;
	}	


void CViewhtmlApp::OnGoBack() 
	{
	OpenDocumentFile("back");
	}

void CViewhtmlApp::OnUpdateGoBack(CCmdUI* pCmdUI) 
	{
	pCmdUI->Enable( m_back_array.GetSize() > 0);	
	}

void CViewhtmlApp::OnGoForward() 
	{
	OpenDocumentFile("forward");
	}

void CViewhtmlApp::OnUpdateGoForward(CCmdUI* pCmdUI) 
	{
	pCmdUI->Enable( m_forward_array.GetSize() > 0);	
	}


void CViewhtmlApp::OnGoHome() 
	{
	OpenDocumentFile(m_HomePage);
	}

void CViewhtmlApp::OnUpdateGoHome(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

void CViewhtmlApp::OnAddBookmark() 
	{
	CString url;
	
	if (m_viewhtml_doc->GetMimeObject())
		url = m_viewhtml_doc->GetMimeObject()->GetURL();

	m_bookmarks.AddBookmark( m_viewhtml_doc->GetTitle(), url );
	m_bookmarks.StoreBookmarks( theApp.m_programDir + BOOKMARK_FILENAME );
	}

void CViewhtmlApp::OnUpdateAddBookmark(CCmdUI* pCmdUI) 
	{
	pCmdUI->Enable( !GetCurrentURL().IsEmpty() );
	}

void CViewhtmlApp::OnEditBookmarks() 
	{
	CDialogViewBookmarks dialog;

	if (m_viewhtml_doc->GetMimeObject())
		{
		dialog.m_input_url = m_viewhtml_doc->GetMimeObject()->GetURL();
		dialog.m_input_title = m_viewhtml_doc->GetTitle();
		}
	
	dialog.m_bookmark_library = m_bookmarks;
	
	switch(dialog.DoModal())
		{
		case IDC_BUTTON_OPEN_URL:
			OpenDocumentFile( dialog.m_output_url);
			// fallthrough
		case IDOK:
			m_bookmarks = dialog.m_bookmark_library;
			m_bookmarks.StoreBookmarks( theApp.m_programDir + BOOKMARK_FILENAME );
			break;
		}
	}

BOOL CViewhtmlApp::OnSelectBookmark(UINT nID) 
	{
	if( nID >= ID_BOOKMARK00 && nID <= ID_BOOKMARK19)
		{
		OpenDocumentFile( m_bookmarks.GetBookmarkURL( nID - ID_BOOKMARK00 ) );
		}
	return TRUE;
	}

void CViewhtmlApp::OnUpdateBookmarks(CCmdUI* pCmdUI) 
	{
	if (m_bookmarks.GetBookmarkCount() == 0)
	{
		// no MRU files
		pCmdUI->Enable(FALSE);
		return;
	}

	if (pCmdUI->m_pMenu == NULL)
		return;

	// delete previous commands
	for (int iMRU = 0; iMRU < m_last_menu_size; iMRU++)
		pCmdUI->m_pMenu->DeleteMenu(pCmdUI->m_nID + iMRU, MF_BYCOMMAND);

	m_last_menu_size = m_bookmarks.GetBookmarkCount();

	if (m_last_menu_size > 0)
		{
		for (int iMRU = 0; iMRU < m_last_menu_size; iMRU++)
			{
			pCmdUI->m_pMenu->InsertMenu(pCmdUI->m_nIndex+ iMRU,
				MF_STRING | MF_BYPOSITION, pCmdUI->m_nID+ iMRU,
				m_bookmarks.GetBookmarkTitle(iMRU));
			}
		}
	else
		{
		pCmdUI->m_pMenu->InsertMenu(pCmdUI->m_nIndex,
			MF_STRING | MF_BYPOSITION, pCmdUI->m_nID,
			"Bookmarks");
		pCmdUI->Enable(FALSE);
		m_last_menu_size = 1;
		}
	}



void CViewhtmlApp::OnOptionsPreferences() 
	{
	CDialogPrefNetworkAndCache network_and_cache;
	CDialogPrefOleControls ole_controls;
	CDialogPrefUserInfo userinfo;

	CDialogPref pref("Preferences");

		// user info
	userinfo.m_UserName = m_UserName;
	userinfo.m_UserEmail = m_UserEmail;
	userinfo.m_HomePage = m_HomePage;
	userinfo.m_GotoHomePage = m_bLoadHomePageOnStartup;

		// network info
	network_and_cache.m_CacheDirectory = m_tempStorageDir;
	network_and_cache.m_SmtpServer = m_SmtpServer;
	network_and_cache.m_HttpProxy = m_HttpProxy;
	network_and_cache.m_HttpProxyPort = m_HttpProxyPort;
	network_and_cache.m_MaxConnections = m_max_connections;

		// OLE controls
	ole_controls.m_OcxDirectory = m_ocxStorageDir;
		
	pref.AddPage(&network_and_cache);
	pref.AddPage(&ole_controls);
	pref.AddPage(&userinfo);

	if (pref.DoModal()==IDOK)
		{
		// save preferences given by user

			// user info
		m_UserName = userinfo.m_UserName;
		m_UserEmail = userinfo.m_UserEmail;
		m_HomePage = userinfo.m_HomePage;
		m_bLoadHomePageOnStartup = userinfo.m_GotoHomePage;

			// network info
		m_tempStorageDir = network_and_cache.m_CacheDirectory;
		if (m_tempStorageDir[ m_ocxStorageDir.GetLength()-1] != '\\')
			m_tempStorageDir += '\\';
		m_SmtpServer = network_and_cache.m_SmtpServer;
		m_HttpProxy = network_and_cache.m_HttpProxy;
		m_HttpProxyPort = network_and_cache.m_HttpProxyPort;
		m_max_connections = network_and_cache.m_MaxConnections;

			// ole info
		m_ocxStorageDir = ole_controls.m_OcxDirectory;
		if (m_ocxStorageDir[ m_ocxStorageDir.GetLength()-1] != '\\')
			m_ocxStorageDir += '\\';
		
		WriteRegistryInfo();
		}
	}

int CViewhtmlApp::Run() 
	{
	// for tracking the idle time state
	BOOL bIdle = TRUE;
	LONG lIdleCount = 0;
	MSG pendingMessage;
	
	ASSERT_VALID(this);

	// message loop lasts until we get a WM_QUIT message
	// upon which we shall return from the function
	while (TRUE) 
	  	{
	    // block-local variable
		DWORD result ;

		// wait for any message sent or posted to this queue
		// or for one of the passed handles to become signaled
		result = MsgWaitForMultipleObjects(1, &m_notify_event, FALSE, INFINITE, QS_ALLINPUT);

		// result tells us the type of event we have:
		// a message or a signaled handle

		// if there are one or more messages in the queue ...
		if (result == (WAIT_OBJECT_0 + 1)) 
			{
			do
				{
				// pump message, but quit on WM_QUIT
				if (!PumpMessage())
					return ExitInstance();

				// reset "no idle" state after pumping "normal" message
				if (IsIdleMessage(AfxGetCurrentMessage()))
					{
					bIdle = TRUE;
					lIdleCount = 0;
					}
				} 
			while (::PeekMessage(&pendingMessage, NULL, NULL, NULL, PM_NOREMOVE));

			// timeout, do idles
			while (bIdle &&
				!::PeekMessage(&pendingMessage, NULL, NULL, NULL, PM_NOREMOVE))
				{
				// call OnIdle while in bIdle state
				if (!OnIdle(lIdleCount++))
					bIdle = FALSE; // assume "no idle" state
				}
			} // end of PeekMessage while loop
		else if (result == WAIT_OBJECT_0)
			{
         	// one of the handles became signaled
			CString m = CDynamicLoad::DoNotifies();

			if (!m.IsEmpty() )
				{
				((CFrameWnd *)m_pMainWnd)->SetMessageText( m );
				}
      		} 
		} 
	}

// viewhtml.h : main header file for the VIEWHTML application
//

#define _VIEWHTML_H_

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"

#ifndef _BOOKMARK_H_
#include "bookmark.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewhtmlApp:
// See viewhtml.cpp for the implementation of this class
//

#define WM_DO_NOTIFY (WM_USER)

class CViewhtmlView;
class CViewhtmlDoc;

class CViewhtmlApp : public CWinApp
{
public:
	CViewhtmlApp();

	void SignalNotify();

	CString GetCurrentURL();

	CString m_viewclass;
	CStringArray m_back_array;			// list of URLs to go forward and backwards
	CStringArray m_forward_array;

	CBookmarkLibrary m_bookmarks;
	INT m_last_menu_size;

	// Environment variables 

		// User Info
	CString m_UserName;
	CString m_UserEmail;
	CString	m_HomePage;
	BOOL m_bLoadHomePageOnStartup;

		// Network Info
	CString m_SmtpServer;
	CString m_HttpProxy;
	INT32 m_HttpProxyPort;
	INT32 m_max_connections;			// maximum number of simulations connections

		// Directories
	CString	m_ocxStorageDir;
	CString m_tempStorageDir;

	CString m_programDir;				// The directory in which the program resides

	void	ReadRegistryInfo();
	void 	WriteRegistryInfo();

	// Since the document and view never change, we can store them as global variables
	CViewhtmlDoc *m_viewhtml_doc;
	CViewhtmlView *m_viewhtml_view;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewhtmlApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
	virtual int Run();
	//}}AFX_VIRTUAL

// Implementation
	COleTemplateServer m_server;
		// Server object for document creation

	//{{AFX_MSG(CViewhtmlApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
	afx_msg void OnFileOpenUrl();
	afx_msg void OnGoBack();
	afx_msg void OnUpdateGoBack(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGoForward(CCmdUI* pCmdUI);
	afx_msg void OnGoForward();
	afx_msg void OnGoHome();
	afx_msg void OnUpdateGoHome(CCmdUI* pCmdUI);
	afx_msg void OnAddBookmark();
	afx_msg void OnUpdateAddBookmark(CCmdUI* pCmdUI);
	afx_msg void OnEditBookmarks();
	afx_msg void OnUpdateBookmarks(CCmdUI* pCmdUI);
	afx_msg void OnOptionsPreferences();
	//}}AFX_MSG

	afx_msg BOOL OnSelectBookmark(UINT nID);

	DECLARE_MESSAGE_MAP()

private:
	HANDLE m_notify_event;
};

extern CViewhtmlApp theApp;	// if anyone wants to reference us directly

/////////////////////////////////////////////////////////////////////////////

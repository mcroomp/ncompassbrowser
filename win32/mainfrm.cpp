// mainfrm.cpp : implementation of the CMainFrame class
//

#include "cross_p.h"
#include "viewhtml.h"

#ifndef _BITBLT_H_
#include "bitblt.h"
#endif

#include "mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_QUERYNEWPALETTE()
	ON_WM_PALETTECHANGED()
	ON_WM_LBUTTONDOWN()
	ON_COMMAND(ID_VIEW_LOCATION_BAR, OnViewLocationBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LOCATION_BAR, OnUpdateViewLocationBar)
	ON_COMMAND(ID_OPEN_LOCATION, OnOpenLocation)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// arrays of IDs used to initialize control bars
	
// toolbar buttons - IDs are command buttons
static UINT BASED_CODE buttons[] =
{
	// same order as in the bitmap 'toolbar.bmp'
	ID_GO_BACK,
	ID_GO_FORWARD,
	ID_GO_HOME,
		ID_SEPARATOR,
	ID_RELOAD,
	ID_FILE_OPEN_URL,
		ID_SEPARATOR,
	ID_ADD_BOOKMARK,
	ID_EDIT_BOOKMARKS,
		ID_SEPARATOR,
	ID_STOP_LOADING,
};

static UINT BASED_CODE indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadBitmap(IDR_MAINFRAME) ||
		!m_wndToolBar.SetButtons(buttons,
		  sizeof(buttons)/sizeof(UINT)))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.SetSizes( CSize(70,38), CSize(64,32) );

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndLocationBar.Create(this, IDD_LOCATION_BAR, CBRS_TOP, IDW_LOCATION_BAR);
	m_wndLocationBar.EnableDocking( CBRS_ALIGN_TOP|CBRS_ALIGN_BOTTOM );
	
	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);
	DockControlBar(&m_wndLocationBar);

	// TODO: Remove this if you don't want tool tips
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::SetLocationText( LPCSTR text )
	{
	m_wndLocationBar.SetDlgItemText( IDC_LOCATION, text );
	}

BOOL CMainFrame::OnQueryNewPalette() 
	{
	CDC *pDC = GetDC();

	pDC->SelectPalette( GetHalftonePalette(), FALSE);
      
   	BOOL f = pDC->RealizePalette();
      		
	ReleaseDC(pDC);

  	if (f)
    	InvalidateRect(NULL,TRUE);

	return CFrameWnd::OnQueryNewPalette();
	}

void CMainFrame::OnPaletteChanged(CWnd* pFocusWnd) 
	{
	CFrameWnd::OnPaletteChanged(pFocusWnd);
	
	if (pFocusWnd->m_hWnd != m_hWnd)
		OnQueryNewPalette();
	}


void CMainFrame::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	CFrameWnd::OnLButtonDown(nFlags, point);
}

void CMainFrame::OnViewLocationBar() 
	{
	CControlBar* pBar = GetControlBar(IDW_LOCATION_BAR);
	if (pBar != NULL)
		{
		ShowControlBar(pBar, (pBar->GetStyle() & WS_VISIBLE) == 0, FALSE);
		}
	}


void CMainFrame::OnUpdateViewLocationBar(CCmdUI* pCmdUI) 
	{
	CControlBar* pBar = GetControlBar(IDW_LOCATION_BAR);
	if (pBar != NULL)
		{
		pCmdUI->SetCheck((pBar->GetStyle() & WS_VISIBLE) != 0);
		}
	}
/////////////////////////////////////////////////////////////////////////////
// CLocationDialogBar dialog

BEGIN_MESSAGE_MAP(CLocationDialogBar, CDialogBar)
	//{{AFX_MSG_MAP(CLocationDialogBar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLocationDialogBar message handlers

void CMainFrame::OnOpenLocation() 
	{
	char url[256];

	m_wndLocationBar.GetDlgItemText( IDC_LOCATION, url, sizeof(url) );
	theApp.OpenDocumentFile( url );
	}

void CMainFrame::OnEditCopy() 
{
	CWnd *w = m_wndLocationBar.GetDlgItem(IDC_LOCATION);
	if (w->m_hWnd == ::GetFocus())
		{
		w->SendMessage(WM_COPY);
		}
}

void CMainFrame::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_wndLocationBar.GetDlgItem(IDC_LOCATION)->m_hWnd == ::GetFocus() );
}

void CMainFrame::OnEditCut() 
{
	CWnd *w = m_wndLocationBar.GetDlgItem(IDC_LOCATION);
	if (w->m_hWnd == ::GetFocus())
		{
		w->SendMessage(WM_CUT);
		}
}

void CMainFrame::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_wndLocationBar.GetDlgItem(IDC_LOCATION)->m_hWnd == ::GetFocus() );
}

void CMainFrame::OnEditPaste() 
	{
	CWnd *w = m_wndLocationBar.GetDlgItem(IDC_LOCATION);
	if (w->m_hWnd == ::GetFocus() )
		{
		w->SendMessage(WM_PASTE);
		}
	}

void CMainFrame::OnUpdateEditPaste(CCmdUI* pCmdUI) 
	{
	pCmdUI->Enable((m_wndLocationBar.GetDlgItem(IDC_LOCATION)->m_hWnd == ::GetFocus())
		&& IsClipboardFormatAvailable(CF_TEXT) );
	}

void CMainFrame::OnEditUndo() 
	{
	CWnd *w = m_wndLocationBar.GetDlgItem(IDC_LOCATION);
	if (w->m_hWnd == ::GetFocus())
		{
		w->SendMessage(WM_UNDO);
		}
	}

void CMainFrame::OnUpdateEditUndo(CCmdUI* pCmdUI) 
	{
	pCmdUI->Enable( m_wndLocationBar.GetDlgItem(IDC_LOCATION)->m_hWnd == ::GetFocus() );
	}

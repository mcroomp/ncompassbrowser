// viewhvw.h : interface of the CViewhtmlView class
//
/////////////////////////////////////////////////////////////////////////////

#define _VIEWHVW_H_

#ifndef _FMTHTML_H_
#include "fmthtml.h"
#endif

class CViewhtmlCntrItem;

class CViewhtmlView : public CScrollView
{
protected: // create from serialization only
	CViewhtmlView();
	DECLARE_DYNCREATE(CViewhtmlView)

// Attributes
public:
	void ClearView();
	void ScrollToAnchor( LPCSTR anchor_name );

	CViewhtmlDoc* GetDocument();
	// m_pSelection holds the selection to the current CViewhtmlCntrItem.
	// For many applications, such a member variable isn't adequate to
	//  represent a selection, such as a multiple selection or a selection
	//  of objects that are not CViewhtmlCntrItem objects.  This selection
	//  mechanism is provided just to help you get started.

	// TODO: replace this selection mechanism with one appropriate to your app.
	CViewhtmlCntrItem* m_pSelection;
	
	// Some flags

	BOOL 	m_nTrackerOn;

// Operations
public:
	void OnDynamicNotify( INT32 changeflags, CDynamicLoad *dlobject);
	// called by document class when objects are loaded

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewhtmlView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual BOOL IsSelected(const CObject* pDocItem) const;// Container support
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	// OLE Control Support
	void 	UpdateOLEControlList();

	virtual ~CViewhtmlView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CFormatHTML 	*m_format_html;
	CFormatHTML 	*m_printing_html;
	long 			m_last_window_width;
	POSITION 		m_last_start_position;

	HBITMAP 		m_double_buffer_bmp;
	LPBYTE 			m_double_buffer_ptr;
	CSize 			m_double_buffer_view_size;
	HPALETTE 		m_default_palette;

	CHotlink 		*m_last_hotlink_shown;

	HCURSOR		m_arrow_cursor, m_hand_cursor, m_wait_cursor;

	COLORREF 	m_back_color, m_text_color, m_hotlink_color, m_old_hotlink_color;
	CString 	m_background_picture_url;

	void ClearBackground(CDC *pDC, const CRect& rect );

	// Double buffering support for background GIFs
	void GetBackgroundDC( CDC *screenDC, CDC &newdc);

	// OLE Control Support
	BOOL 	CheckIfCreatedBefore(CFormatOLEControlItem* item);


	// Selection support
	virtual void 		SetSelection(CViewhtmlCntrItem* pNewSel);
	CViewhtmlCntrItem*	HitTestItems(CPoint point);
	
	// Drawing Support
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	virtual void SetupTracker(CRectTracker* pTracker, CViewhtmlCntrItem* pItem); 
	virtual void InvalidateItem(CViewhtmlCntrItem* pItem);	

	// Popup Menu Support
	void	OnRButtonDown2(UINT nFlags, CPoint point, CPoint cursorPos);
	void	CreatePopupMenu(CPoint point);

// Generated message map functions
protected:
	//{{AFX_MSG(CViewhtmlView)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnInsertObject();
	afx_msg void OnCancelEditCntr();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnDestroy();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in viewhvw.cpp
inline CViewhtmlDoc* CViewhtmlView::GetDocument()
   { return (CViewhtmlDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

// viewhvw.cpp : implementation of the CViewhtmlView class
//

#include "cross_p.h"
#include "viewhtml.h"

#ifndef _PICTURE_H_
#include "picture.h"
#endif

#ifndef _BITBLT_H_
#include "bitblt.h"
#endif

#include "mainfrm.h"
#include "viewhdoc.h"

#ifdef USES_OLE_CONTROLS
#include "cntlitem.h"
#endif

#include "cntritem.h"
#include "viewhvw.h"
#include "inctldlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// definitions

#define MARGIN_PIXELS	6

COLORREF ConvertToColorRef( INT32 color )
	{
	return RGB( GetBValue(color), GetGValue(color), GetRValue(color) );
	}

/////////////////////////////////////////////////////////////////////////////
// CViewhtmlView

IMPLEMENT_DYNCREATE(CViewhtmlView, CScrollView)

BEGIN_MESSAGE_MAP(CViewhtmlView, CScrollView)
	//{{AFX_MSG_MAP(CViewhtmlView)
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
	ON_COMMAND(ID_OLE_INSERT_NEW, OnInsertObject)
	ON_COMMAND(ID_CANCEL_EDIT_CNTR, OnCancelEditCntr)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_DESTROY()
	ON_WM_RBUTTONDOWN()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewhtmlView construction/destruction

CViewhtmlView::CViewhtmlView()
	{
	theApp.m_viewhtml_view = this;

	// added by Kerem - 6/29/95
	m_pSelection = NULL;

	m_format_html = NULL;
	m_printing_html = NULL;

	m_double_buffer_bmp = NULL;
	m_double_buffer_ptr = NULL;

	m_arrow_cursor = theApp.LoadStandardCursor( IDC_ARROW );
	m_wait_cursor = theApp.LoadStandardCursor( IDC_WAIT );
	m_hand_cursor = theApp.LoadCursor( IDC_HAND_CURSOR );
	}

CViewhtmlView::~CViewhtmlView()
	{
	if (m_format_html)
		delete m_format_html;

	if (m_double_buffer_bmp)
		::DeleteObject(m_double_buffer_bmp);

	theApp.m_viewhtml_view = NULL;
	}

/////////////////////////////////////////////////////////////////////////////
// CViewhtmlView drawing

void CViewhtmlView::SetupTracker(CRectTracker* pTracker, CViewhtmlCntrItem* pItem)
{
	ASSERT(pTracker != NULL);
	ASSERT(pItem != NULL);

	if (!m_nTrackerOn)
		return;

    pTracker->m_nHandleSize = MARGIN_PIXELS;
	pTracker->m_rect = pItem->GetRect();
    pTracker->m_rect.InflateRect(MARGIN_PIXELS, MARGIN_PIXELS);
	
	if (pItem == m_pSelection)
		pTracker->m_nStyle |= CRectTracker::resizeInside;

	if (pItem->GetType() == OT_LINK)
		pTracker->m_nStyle |= CRectTracker::dottedLine;
	else
		pTracker->m_nStyle |= CRectTracker::solidLine;

	if (pItem->GetItemState() == COleClientItem::openState ||
		pItem->GetItemState() == COleClientItem::activeUIState)
	{
		pTracker->m_nStyle |= CRectTracker::hatchInside;
	}
}

void CViewhtmlView::ScrollToAnchor( LPCSTR anchor_name )
	{
	if (m_format_html)
		{
		INT32 pos = m_format_html->FindAnchorHeight(anchor_name);
		if (pos != 0)
			ScrollToPosition( CPoint( 0, pos) );
		}
	}

void CViewhtmlView::ClearView()
	{
	if (m_format_html)
		delete m_format_html;
	m_format_html = NULL;
	}

void CViewhtmlView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	CScrollView::OnPrepareDC(pDC, pInfo);
}                        

void CViewhtmlView::OnDraw(CDC* pDC)
{
	pDC->SelectPalette( GetHalftonePalette(),FALSE);
	pDC->RealizePalette();	
	
	if (!m_format_html)
		{
		CRect clipbox;
		pDC->GetClipBox(&clipbox);
		pDC->SelectStockObject( LTGRAY_BRUSH );
		pDC->SelectStockObject( NULL_PEN );
		clipbox.right++;
		clipbox.bottom++;
		pDC->Rectangle(&clipbox);
		return;			// empty document
		}

	CViewhtmlDoc* pDoc = GetDocument();
	
	CDC bitmapDC;

	GetBackgroundDC( pDC, bitmapDC);

	CRect updateRect;
		
	pDC->GetClipBox(&updateRect);

	ClearBackground(&bitmapDC, 
		CRect( updateRect.left, updateRect.top, updateRect.right+1, updateRect.bottom+1));

	ASSERT_VALID(pDoc);

	// Draw all CViewhtmlCntrItems	- kerem 6/29/95
	POSITION pos = pDoc->GetStartPosition();
	while (pos != NULL)
	{
		CViewhtmlCntrItem* pItem = (CViewhtmlCntrItem*)pDoc->GetNextItem(pos);
		pItem->Draw(pDC, pItem->GetRect());
	  
		// draw the tracke
		if (m_nTrackerOn)
		{
			CRectTracker tracker;
			SetupTracker(&tracker, pItem);
			tracker.Draw(pDC);
		}
	}
	
	bitmapDC.SetBkMode(TRANSPARENT);
	
	CPtrList itemlist;
	REDRAW_PARAMS redraw_params(m_format_html->GetPlainText(), &bitmapDC, FALSE, 
		m_double_buffer_ptr, bitmapDC.GetViewportOrg(), m_double_buffer_view_size,
		m_text_color, m_hotlink_color, m_old_hotlink_color, m_back_color, pDoc->GetPictureInfo() );

	INT32 current_page = 1;

	m_format_html->GetFormatList(itemlist, updateRect);
	POSITION walk = itemlist.GetHeadPosition();
	while(walk)
		{
		CFormatItem *item = (CFormatItem *)itemlist.GetNext(walk);		
		item->OnRedraw(redraw_params);
		} 
	
	pDC->BitBlt( updateRect.left, updateRect.top,
				 updateRect.Width(), updateRect.Height(),
				 &bitmapDC,
				 updateRect.left, updateRect.top,
				 SRCCOPY);
	}

void CViewhtmlView::OnInitialUpdate()
	{
	CScrollView::OnInitialUpdate();

	CViewhtmlDoc* pDoc = GetDocument();

	// TODO: remove this code when final selection model code is written
	m_pSelection = NULL;    // initialize selection
	m_nTrackerOn = FALSE;	// tracker is off by default

	// These values are purposely RGB reversed
	m_back_color = RGB(255,255,255);
	m_text_color = RGB(0,0,0);
	m_hotlink_color = RGB(255,0,0);
	m_old_hotlink_color = RGB(255,0,255);
	m_background_picture_url.Empty();

	if (pDoc->GetMimeObject()==NULL)
		{
		m_format_html = NULL;
		SetScrollSizes( MM_TEXT, CSize( 1, 1 ) );
		return;			// empty document
		}

	CMimeObject *mime_object = pDoc->GetMimeObject();

	mime_object->DEBUG_LOCK();

	INT32 tmp = mime_object->GetBackgroundColor();
	if (tmp != -1)
		m_back_color = tmp;

	tmp = mime_object->GetTextColor();
	if (tmp != -1)
		m_text_color = tmp;

	tmp = mime_object->GetHotlinkColor();
	if (tmp != -1)
		m_hotlink_color = tmp;

	m_background_picture_url = mime_object->GetBackgroundPicture();

	mime_object->Unlock();

	pDoc->GetPictureInfo().UpdatePicture( m_background_picture_url );

	CDC *pDC = GetDC();

	CSize clientsize, scrollsize;
	GetTrueClientSize(clientsize, scrollsize);		
	
	FORMAT_PARAMS format_params( clientsize.cx - GetSystemMetrics(SM_CXVSCROLL) ,	// window_width
		1000000000,  		// page_height
		mime_object->GetURL(),		// pass URL
		pDoc->GetPictureInfo(),
		NULL,				// start_position
		pDC,
		pDoc);				// pDC
	
	if (!m_format_html)		
		m_format_html = DEBUG_NEW CFormatHTML;

	m_format_html->Format( mime_object, format_params );
	m_last_start_position = format_params.m_start_position; 

	UpdateOLEControlList();
	
	ReleaseDC(pDC);

	CSize s = m_format_html->GetActualSize();
	if (s.cx == 0)
		s.cx = 1;
	if (s.cy == 0)
		s.cy = 1;

	SetScrollSizes( MM_TEXT, s );
	}

void CViewhtmlView::InvalidateItem(CViewhtmlCntrItem* pItem)
{
	pItem->Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CViewhtmlView printing

BOOL CViewhtmlView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CViewhtmlView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
	{
	if (!m_format_html)
		return;			// empty document

	CViewhtmlDoc* pDoc = GetDocument();

	m_printing_html = DEBUG_NEW CFormatHTML();

	CMimeObject *mime_object = pDoc->GetMimeObject();
	
	FORMAT_PARAMS format_params(pDC->GetDeviceCaps(HORZRES),
	 	pDC->GetDeviceCaps(VERTRES),
		mime_object->GetURL(),
		pDoc->GetPictureInfo(),
		NULL,
		pDC,
		pDoc);

	m_printing_html->Format( mime_object, format_params);
	}

void CViewhtmlView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
	{
	if (!m_format_html)
		return;			// empty document

	if (m_printing_html)
		delete m_printing_html;
	m_printing_html = NULL;
	}

/////////////////////////////////////////////////////////////////////////////
// OLE Client support and commands

BOOL CViewhtmlView::IsSelected(const CObject* pDocItem) const
{
	// The implementation below is adequate if your selection consists of
	//  only CViewhtmlCntrItem objects.  To handle different selection
	//  mechanisms, the implementation here should be replaced.

	// TODO: implement this function that tests for a selected OLE client item

	return pDocItem == m_pSelection;
}

CViewhtmlCntrItem* CViewhtmlView::HitTestItems(CPoint point)
{
	CViewhtmlDoc* pDoc = GetDocument();
	CViewhtmlCntrItem* pCtlHit = NULL;
	POSITION pos = pDoc->GetStartPosition();
	while (pos != NULL)
	{
        CViewhtmlCntrItem* pCtl = (CViewhtmlCntrItem*)pDoc->GetNextItem(pos);
        if (pCtl->IsKindOf(RUNTIME_CLASS(CViewhtmlCntrItem)))
        {
			if (m_nTrackerOn)
			{
            	CRectTracker tracker;
            	SetupTracker(&tracker, pCtl);
            	if (tracker.HitTest(point) >= 0)
            	{
                	pCtlHit = pCtl;
                // items later in the list are drawn on top - so keep looking
            	}
			}
			else
			{
				CRect rect = pCtl->GetRect();
				if (rect.PtInRect(point) )
				{
					// point is inside

					pCtlHit = pCtl;
				}
			}
        }
	}
	return pCtlHit;    // return top item at point
}

void CViewhtmlView::SetSelection(CViewhtmlCntrItem* pNewSel)
{
	if (pNewSel != NULL && pNewSel == m_pSelection)
		return;

	// deactivate any in-place active item on this view!

	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
	if (pActiveItem != NULL && pNewSel != pActiveItem)
	{
		// if we found one, deactivate it
		//pActiveItem->Close();
		//ASSERT(GetDocument()->GetInPlaceActiveItem(this) == NULL);
		pActiveItem->OnDeactivateUI(FALSE);
	//	pActiveItem->OnActivate();
	}
	if (m_pSelection != NULL) // invalidate the old item
		InvalidateItem(m_pSelection);
	if ((m_pSelection = pNewSel) != NULL) // invalidate the new item
		InvalidateItem(m_pSelection);
}

void CViewhtmlView::OnInsertObject()
{
}

// The following command handler provides the standard keyboard
//  user interface to cancel an in-place editing session.  Here,
//  the container (not the server) causes the deactivation.
void CViewhtmlView::OnCancelEditCntr()
{
	// Close any in-place active item on this view.
	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
	if (pActiveItem != NULL)
	{
		pActiveItem->Close();
	}
	ASSERT(GetDocument()->GetInPlaceActiveItem(this) == NULL);
}

// Special handling of OnSetFocus and OnSize are required for a container
//  when an object is being edited in-place.
void CViewhtmlView::OnSetFocus(CWnd* pOldWnd)
{
	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
	if (pActiveItem != NULL &&
		pActiveItem->GetItemState() == COleClientItem::activeUIState)
	{
		// need to set focus to this item if it is in the same view
		CWnd* pWnd = pActiveItem->GetInPlaceWindow();
		if (pWnd != NULL)
		{
			pWnd->SetFocus();   // don't call the base class
			return;
		}
	}

	CScrollView::OnSetFocus(pOldWnd);
}

void CViewhtmlView::OnSize(UINT nType, int cx, int cy)
{
	CViewhtmlDoc* pDoc = GetDocument();
	
	if (!m_format_html)
		return;		// empty document
	
	INT32 top_text_pos = 0;
	
	if (GetDeviceScrollPosition().y > 0)
		m_format_html->GetTextPositionAtHeight( GetDeviceScrollPosition().y );

	CDC *pDC = GetDC();

	CSize clientsize, scrollsize;
	GetTrueClientSize(clientsize, scrollsize);	
	
	CMimeObject *mime_object = pDoc->GetMimeObject();
	
	FORMAT_PARAMS format_params( clientsize.cx - GetSystemMetrics(SM_CXVSCROLL),
		1000000000,	// never page wrap in normal mode
		mime_object->GetURL(),
		pDoc->GetPictureInfo(),
		NULL,
		pDC,
		pDoc);

	m_format_html->Format( mime_object, format_params );
	m_last_start_position = format_params.m_start_position; 
	ReleaseDC(pDC);

	UpdateOLEControlList();

	CSize s = m_format_html->GetActualSize();
	if (s.cx == 0)
		s.cx = 1;
	if (s.cy == 0)
		s.cy = 1;

	SetScrollSizes( MM_TEXT, s );
	
	CScrollView::OnSize(nType, cx, cy);
	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
	if (pActiveItem != NULL)	
		pActiveItem->SetItemRects();

	if (top_text_pos > 0)
		ScrollToPosition( CPoint(0, m_format_html->GetHeightAtTextPosition( top_text_pos )) );
	}

/////////////////////////////////////////////////////////////////////////////
// CViewhtmlView diagnostics

#ifdef _DEBUG
void CViewhtmlView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CViewhtmlView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CViewhtmlDoc* CViewhtmlView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CViewhtmlDoc)));
	return (CViewhtmlDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CViewhtmlView message handlers

void CViewhtmlView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	OnLButtonDown(nFlags, point);

	if (m_pSelection != NULL)
	{
		//m_pSelection->DoVerb(GetKeyState(VK_CONTROL) < 0 ?
		//	OLEIVERB_OPEN : OLEIVERB_PRIMARY, this);
		m_pSelection->DoVerb(OLEIVERB_UIACTIVATE, this);

	}

	CScrollView::OnLButtonDblClk(nFlags, point);
}

void CViewhtmlView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (!m_format_html)
		return;

	// Get the scroll windows position
	CPoint  scrollPos;
	scrollPos = GetDeviceScrollPosition();

	CPoint	newPoint;
	newPoint.y = point.y - scrollPos.y;
	newPoint.x = point.x - scrollPos.x;

	CViewhtmlCntrItem* pItemHit = HitTestItems(newPoint);
	SetSelection(pItemHit);

/*	if (pItemHit != NULL)
	{
		if (m_nTrackerOn)
		{

			CRectTracker tracker;
			SetupTracker(&tracker, pItemHit);

			UpdateWindow();
			if (tracker.Track(this, point))
			{
				pItemHit->Invalidate();    
				tracker.m_rect.InflateRect(-MARGIN_PIXELS, -MARGIN_PIXELS);			
				pItemHit->Move(tracker.m_rect);
				pItemHit->Invalidate();
				GetDocument()->SetModifiedFlag();
			} 
		}
	}
	else
		{*/
		SetFocus();

		// Do the hotlink thing
		CPtrList itemlist;

		CPoint p = GetDeviceScrollPosition();

		p.x += point.x;
		p.y += point.y;

		CRect itemrect( p.x, p.y, p.x+1, p.y+1 );

		m_format_html->GetFormatList(itemlist, itemrect);
		POSITION walk = itemlist.GetHeadPosition();
		while(walk)
			{
			CFormatItem *item = (CFormatItem *)itemlist.GetNext(walk);		
			CFormatHotlinkPictureItem *picture = (CFormatHotlinkPictureItem *)item;
			BOOL hotmapped = FALSE;
	
			CHotlink *hotlink = item->GetHotlink();

			CRect extent = item->GetExtent();
	
			if ( item->GetItemType() == ITEM_HOTLINK_PICTURE)
				{
				// we may have an image map
				hotmapped = picture->IsMapped();
				}

			if (hotlink)
				{
				CMimeObject *mime_object = GetDocument()->GetMimeObject();

				CString oldurl = mime_object->GetURL();
				
				CString newurl = hotlink->GetURL();
				if (hotmapped)
					{
					CString coord;
					coord.Format("?%d,%d", p.x - extent.left, p.y - extent.top);
					newurl += coord;
					}

				theApp.OpenDocumentFile( newurl );
				}
			} 
		//}

	CScrollView::OnLButtonDown(nFlags, point);
}

BOOL CViewhtmlView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (GetDocument()->IsBetweenLoads())
		{
		::SetCursor( m_wait_cursor );
		return TRUE;
		}	
	

	if (pWnd == this && m_pSelection != NULL)
	{
		// give the tracker for the selection a chance
		CRectTracker tracker;
		SetupTracker(&tracker, m_pSelection);
		if (tracker.SetCursor(this, nHitTest))
			return TRUE;
	}
	if (pWnd != this)
	{
		// We might be inside a control 
		
		// Get cursor position
		POINT 	point;
		GetCursorPos(&point);
		CPoint  cursorPos(point);
		
		// Get windows position
		CRect	rect;
		GetWindowRect(&rect);
		
		// Get the scroll windows position
		CPoint  scrollPos;
		scrollPos = GetDeviceScrollPosition();

		// Calculate 
		CPoint 	mousePos(point);
		mousePos.Offset(-rect.TopLeft());
		mousePos.Offset(scrollPos);
		
		// check if it is a WM_LBUTTONDOWN

		switch (message)
		{
			case	WM_LBUTTONDOWN :
					OnLButtonDblClk(0, mousePos);
					break;
			case 	WM_RBUTTONDOWN :
					// Call our second Rbuttondown function to process buttondown
					// for the window owned by OCX.

					OnRButtonDown2(0, mousePos, cursorPos);
					break;
		}
					
	}


// Set cursor to hand if it is above a hotlink
	if (nHitTest != HTCLIENT)
		{
		return CScrollView::OnSetCursor(pWnd, nHitTest, message);
		}

	if (!m_format_html)
		{
		::SetCursor( m_arrow_cursor );		
		return TRUE;
		}

	// Do the hotlink thing
	CPtrList itemlist;

	CPoint p = GetDeviceScrollPosition();

	POINT 	point;
	GetCursorPos(&point);
	ScreenToClient(&point);

	p.x += point.x;
	p.y += point.y;

	CRect itemrect( p.x, p.y, p.x+1, p.y+1 );

	m_format_html->GetFormatList(itemlist, itemrect);
	POSITION walk = itemlist.GetHeadPosition();
	while(walk)
		{
		CFormatItem *item = (CFormatItem *)itemlist.GetNext(walk);		
		CFormatHotlinkPictureItem *picture = (CFormatHotlinkPictureItem *)item;
		BOOL hotmapped = FALSE;
	
		CHotlink *hotlink = item->GetHotlink();

		CRect extent = item->GetExtent();

		if ( item->GetItemType() == ITEM_HOTLINK_PICTURE)
			{
			// we may have an image map
			hotmapped = picture->IsMapped();
			}

		if (hotlink)
			{
			::SetCursor( m_hand_cursor );

			if (hotlink != m_last_hotlink_shown || hotmapped)
				{
				if (hotmapped)
					{
					CString coord;
					coord.Format("?%d,%d", p.x - extent.left, p.y - extent.top);
					GetParentFrame()->SetMessageText( hotlink->GetURL() + coord );
					}
				else
					{
					GetParentFrame()->SetMessageText( hotlink->GetURL() );
					}
				m_last_hotlink_shown = hotlink;
				}
			return TRUE;
			}
		} 

	if (m_last_hotlink_shown != NULL)
		{
		GetParentFrame()->SetMessageText( "" );		
		m_last_hotlink_shown = NULL;
		}
	
	::SetCursor( m_arrow_cursor );		
	return TRUE;
	}

void CViewhtmlView::OnDestroy() 
{
	CScrollView::OnDestroy();
	
	// Deactivate the in place active item on this view

	// Note:  To handle multiple inplace items, you will need to
	//		iterate through all of the items and deactivate those that
	//		apply.

	//COleClientItem* pItem = GetDocument()->GetInPlaceActiveItem(this);
	//if (pItem != NULL && pItem->GetActiveView() == this)
	//{
	//	pItem->Deactivate();
	//	ASSERT(GetDocument()->GetInPlaceActiveItem(this) == NULL);
	//}

	/*CViewhtmlDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	POSITION pos = pDoc->GetStartPosition();
	
	while (pos != NULL)
	{
		CViewhtmlCntrItem* pItem = (CViewhtmlCntrItem*)pDoc->GetNextItem(pos);

		UINT state = pItem->GetItemState();
		if (state == COleClientItem::activeState || 
			state == COleClientItem::activeUIState)
		{
			pItem->Close();
		}
	} */
	
	ASSERT(GetDocument()->GetInPlaceActiveItem(this) == NULL);
	
}

void CViewhtmlView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
	{
	}

void CViewhtmlView::UpdateOLEControlList()
{
	POSITION walk = m_format_html->GetOLEControlHeadPosition();
	while(walk)
	{
		CFormatOLEControlItem* item = m_format_html->GetOLEControlNext(walk);

		BOOL OCXFound = TRUE;

		CDynLoadMemFile * m = item->GetMemFile();

		if (m == NULL)
		{
			// No src file is specified

			if (!CheckIfCreatedBefore(item))
			{
				OCXFound = FALSE;
			}
		}		
		else 
		{
			m->DEBUG_LOCK();
			LOAD_STATE l = m->GetLoadState();
			m->Unlock();
			if (l == LOAD_STATE_COMPLETE)
			{
			// input_file is the entire file loaded by the dynamic loader
				if (!CheckIfCreatedBefore(item))
				{
					OCXFound = FALSE;
				}
			}
		}

		if (!OCXFound)
		{
			// Create new item connected to this document.

			CViewhtmlCntrItem* pItem = NULL;
			CViewhtmlDoc* pDoc = GetDocument();
			ASSERT_VALID(pDoc);

			// Get the scroll windows position
			CPoint  scrollPos;
			scrollPos = GetDeviceScrollPosition();

			CRect rect = item->GetExtent();
			rect.top -= scrollPos.y;
			rect.bottom -= scrollPos.y;
			rect.left -= scrollPos.x;
			rect.right -= scrollPos.x;

			pItem = DEBUG_NEW CViewhtmlCntrItem(item->GetParseID(), 
												rect, 
												item->GetOCX_URL(), pDoc);
			ASSERT_VALID(pItem);

			// Save this view to the CControlItem
			// (this is a hack!!)
			pItem->SetView(this);
			pItem->SetDocument(pDoc);

			// First check to see if we are reading the object
			// from a file or creating it from scratch.
			if (item->GetMemFile() != NULL)
			{
				// We read a file, try to open a storage
				if (pItem->OpenStorage(item) == -1)
				{
					pItem->SetBroken();
				}
				else
				{
					// everything should be fine
					// Then attempt to get CLSID...
					if (pItem->GetCLSID(item))
					{
						// Try to create the object
						pItem->CheckAndInsertObject();
					}
					else 
					{
						// We don't have any class id, this item is 
						// not valid.
						pItem->SetBroken();	
					}					
				}				
			}
		}
	}
}

BOOL CViewhtmlView::CheckIfCreatedBefore(CFormatOLEControlItem* item)
{
	CViewhtmlDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	POSITION pos = pDoc->GetStartPosition();
	
	BOOL  OCXFound = FALSE;
	while (pos != NULL)
	{
		CViewhtmlCntrItem* pItem = (CViewhtmlCntrItem*)pDoc->GetNextItem(pos);

		if (pItem->m_parseID == item->GetParseID() )
		{	
			// It was created before

			// Get the scroll windows position
			CPoint  scrollPos;
			scrollPos = GetDeviceScrollPosition();

			CRect rect = item->GetExtent();
			rect.top -= scrollPos.y;
			rect.bottom -= scrollPos.y;
			rect.left -= scrollPos.x;
			rect.right -= scrollPos.x;

			pItem->Move(rect);
			OCXFound = TRUE;
			break;
		}
	}		
	return OCXFound;
}
	
void CViewhtmlView::OnDynamicNotify( INT32 change_flags, CDynamicLoad *source)
	{
	ASSERT_UI_THREAD();

	/* message sent to us telling us one of three things has happened:

	1) the parsed text just got longer, so we have to reformat
	2) a picture got more detailed, so update it
	3) something finished loading

	*/

	CViewhtmlDoc *pDoc = GetDocument();
 	
	// Ignore notifications from objects we don't know about, but do update the OLE list in
	// case a property got loaded
	if ( !source->IsKindOf( RUNTIME_CLASS( CMimeDynamicLoad) ))
		{
		UpdateOLEControlList();
		return;
		}

	CMimeDynamicLoad *dlobject = (CMimeDynamicLoad *)source;
	
	dlobject->DEBUG_LOCK();
	CMimeObject *mime_object = dlobject->GetMimeObject();
	dlobject->Unlock();
	
	CDC *pDC = GetDC();
	OnPrepareDC(pDC);
	pDC->SelectPalette( GetHalftonePalette(), FALSE);
	pDC->RealizePalette();
	
	// retrieve current clipping box to optimize redraws
	CRect clipbox;
	pDC->GetClipBox(&clipbox);
		
	CDC bitmapDC;
	GetBackgroundDC( pDC, bitmapDC);
	
	bitmapDC.SetBkMode(TRANSPARENT);		

	bitmapDC.SelectStockObject( NULL_PEN);
	bitmapDC.SelectStockObject( WHITE_BRUSH );

	REDRAW_PARAMS redraw_params(m_format_html->GetPlainText(), &bitmapDC, FALSE,
		m_double_buffer_ptr, bitmapDC.GetViewportOrg(), m_double_buffer_view_size,
		m_text_color, m_hotlink_color, m_old_hotlink_color, m_back_color,pDoc->GetPictureInfo() );

	if ( !mime_object )
		goto just_format; 

	// add all pictures to the update region
	if ( mime_object->GetObjectType() == MIME_OBJECT_PICTURE && (change_flags&CHANGEFLAG_PICTURE_CHANGE) )
		{
		// find all objects in current window, and invalidate the area covered by all copies of the picture

		CString picture_url = pDoc->GetPictureInfo().FindPicture( dlobject );

		if ( m_background_picture_url == picture_url)
			{
			// if the background picture changes, redraw everything
			InvalidateRect(NULL);
			}
		else
			{
			CPtrList itemlist;

			m_format_html->GetFormatList(itemlist, clipbox);

			POSITION walk = itemlist.GetHeadPosition();
			while(walk)
				{
				CFormatItem *item = (CFormatItem *)itemlist.GetNext(walk);
				if (item->GetPicture() && item->GetPicture() == picture_url )
					{
					CRect ex = item->GetExtent();

					ClearBackground(&bitmapDC, ex);

					item->OnRedraw(redraw_params);

					// bitblt the updated bitmap to the screen
					pDC->BitBlt( ex.left, ex.top,
				 		ex.Width(), ex.Height(),
				 		&bitmapDC,
				 		ex.left, ex.top,
				 		SRCCOPY);	
					}
				}
			}
		}
	else if ( mime_object->GetObjectType() == MIME_OBJECT_HTML )
		{
		if (change_flags& CHANGEFLAG_URL_REDIRECT)
			{
			mime_object->DEBUG_LOCK();
			CString url = mime_object->GetURL();
			mime_object->Unlock();
		
			pDoc->SetPathName( url );
			}

		if (change_flags& (CHANGEFLAG_TITLE|CHANGEFLAG_CACHED))
			{
			mime_object->DEBUG_LOCK();
			CString title = mime_object->GetTitle();
			mime_object->Unlock();

			if (!title.IsEmpty())
				pDoc->SetTitle( title );
			}
		if (change_flags & (CHANGEFLAG_BACK_COLOR|CHANGEFLAG_CACHED) )
			{
			mime_object->DEBUG_LOCK();
			INT32 back_color = mime_object->GetBackgroundColor();
			mime_object->Unlock();

			if (back_color != -1)
				{
				m_back_color = back_color;
				InvalidateRect(NULL);
				}
			}
		if (change_flags & (CHANGEFLAG_TEXT_COLOR|CHANGEFLAG_CACHED))
			{
			mime_object->DEBUG_LOCK();
			INT32 text_color = mime_object->GetTextColor();
			mime_object->Unlock();

			if (text_color != -1)
				{
				m_text_color = text_color;
				InvalidateRect(NULL);
				}
			}

		if (change_flags & (CHANGEFLAG_HOTLINK_COLOR|CHANGEFLAG_CACHED))
			{
			mime_object->DEBUG_LOCK();
			INT32 hotlink_color = mime_object->GetHotlinkColor();
			mime_object->Unlock();

			if (hotlink_color != -1)
				{
				m_hotlink_color = hotlink_color;
				InvalidateRect(NULL);
				}
			}

		if (change_flags & (CHANGEFLAG_BACK_PICTURE|CHANGEFLAG_CACHED))
			{
			mime_object->DEBUG_LOCK();
			CString background_picture_url =  mime_object->GetBackgroundPicture();
			mime_object->Unlock();

			if (!background_picture_url.IsEmpty())
				{
				m_background_picture_url = background_picture_url;
				pDoc->GetPictureInfo().UpdatePicture( m_background_picture_url );
				InvalidateRect(NULL);
				}
			}
		}

just_format:
	CSize clientsize, scrollsize;
	GetTrueClientSize(clientsize, scrollsize);	
		
	FORMAT_PARAMS format_params( clientsize.cx - GetSystemMetrics(SM_CXVSCROLL),
		1000000000,	// never page wrap in normal mode
		pDoc->GetMimeObject()->GetURL(),
		pDoc->GetPictureInfo(),
		m_last_start_position,
		pDC,
		pDoc);

	m_format_html->Format( pDoc->GetMimeObject(), format_params );	
	m_last_start_position = format_params.m_start_position; 

	UpdateOLEControlList();

	CRect itemextent;

	itemextent.SetRectEmpty();

	// find the union of all new objects
	POSITION walk = format_params.m_first_new_format_item_pos;
	while(walk)
		{
		CFormatItem *item = (CFormatItem *)m_format_html->GetNextFormatItem(walk);
		itemextent.UnionRect( itemextent, item->GetExtent() );
		}
	InvalidateRect(&itemextent);

	ReleaseDC(pDC);

	CSize s = m_format_html->GetActualSize();
	if (s.cx == 0)
		s.cx = 1;
	if (s.cy == 0)
		s.cy = 1;

	SetScrollSizes( MM_TEXT, s );
	}

void CViewhtmlView::GetBackgroundDC( CDC *screenDC, CDC& newdc)
	{
	CRect client;

	GetClientRect(&client);

	// find nearest bitmap size that is a multiple of 4 in width
	CSize double_buffer_view_size( RoundUp4(client.Width()+1), client.Height()+1 );

	if (double_buffer_view_size != m_double_buffer_view_size)
		{
		// the view size has changed, so we have to resize the bitmap
																			 
		if (m_double_buffer_bmp)
			::DeleteObject(m_double_buffer_bmp);

		struct
			{
			BITMAPINFOHEADER hdr;
			RGBQUAD color_tbl[256];
			} r;

		LPVOID ptr;		

		r.hdr.biSize = sizeof(r.hdr);
		r.hdr.biHeight = -(int)double_buffer_view_size.cy;
		r.hdr.biWidth = double_buffer_view_size.cx;
		r.hdr.biPlanes = 1;
		r.hdr.biBitCount = 8;
		r.hdr.biCompression = BI_RGB;
		r.hdr.biSizeImage = 0;
		r.hdr.biXPelsPerMeter = 300;
		r.hdr.biYPelsPerMeter = 300;
		r.hdr.biClrUsed = 256;
		r.hdr.biClrImportant = 256;

		PALETTEENTRY palette[256];
		GetHalftonePalette()->GetPaletteEntries(0, 256, palette);

		for(int i=0;i<256;i++)
			{
			r.color_tbl[i].rgbRed = palette[i].peRed;
			r.color_tbl[i].rgbGreen = palette[i].peGreen;
			r.color_tbl[i].rgbBlue = palette[i].peBlue;
			r.color_tbl[i].rgbReserved = 0;
			}

		m_double_buffer_bmp = CreateDIBSection( screenDC->m_hDC, (const BITMAPINFO *)&r, DIB_RGB_COLORS, &ptr,
				NULL, 0);
	
		ASSERT(m_double_buffer_bmp);	

		m_double_buffer_ptr = (LPBYTE)ptr;
		m_double_buffer_view_size = double_buffer_view_size;
		}

	newdc.CreateCompatibleDC(screenDC);
	newdc.SelectObject( m_double_buffer_bmp );

	CRect clipbox;
	screenDC->GetClipBox(&clipbox);

	CRgn region;

	CPoint p = GetDeviceScrollPosition();

	region.CreateRectRgn(clipbox.left - p.x, clipbox.top - p.y, clipbox.right - p.x, clipbox.bottom - p.y);
	newdc.SelectClipRgn( &region);

	// by default shift viewport origin in negative direction of scroll
	newdc.SetViewportOrg( -GetDeviceScrollPosition() );		

	newdc.SelectPalette( GetHalftonePalette(), FALSE );
	newdc.RealizePalette();
	}


/////////////////////////////////
// FormatItem stuff
/////////////////////////////////

	  	
CFormatTextItem::CFormatTextItem(INT32 text_ofs, INT32 text_len, 
	 INT32 flags, const CRect& extent, CFormatFrame *frame) : CFormatItem(ITEM_TEXT, extent, frame)
	{
	m_text_ofs = text_ofs;
	m_text_len = text_len;
	m_flags = flags;
	}	

// My attempt to make these fonts as netscape-like as possible
static INT32	fontsizetbl[8] =
	{ 0,		
	  110,		// fontsize 1
	  119,		//			2
	  145,		//			3
	  160,		//			4
	  240,		//			5
	  320,		//			6
	  400 };		//			7
	

static CFont* CreateHTMLFont(CDC *pDC, int flags)
	{
	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	lf.lfHeight = fontsizetbl[flags & 0xf] * pDC->GetDeviceCaps(LOGPIXELSX) / 720;
 	if (flags & FONTFLAG_FIXED)
		{
		//strcpy(lf.lfFaceName,"Courier New");
		lf.lfPitchAndFamily = FF_MODERN | FIXED_PITCH;
		}
	else
		{
		//strcpy(lf.lfFaceName,"Times New Roman");
		lf.lfPitchAndFamily = FF_ROMAN | VARIABLE_PITCH;
		}

	if (flags & FONTFLAG_BOLD)
		lf.lfWeight = 700;
	else
		lf.lfWeight = 400;
	if (flags & FONTFLAG_ITALIC)
		lf.lfItalic = TRUE;
	if ( flags & FONTFLAG_UNDERLINE)
		lf.lfUnderline = TRUE;
	if (flags & FONTFLAG_HOTLINK)
		{
		lf.lfUnderline = TRUE;
		}
		
	CFont *font = DEBUG_NEW CFont;
	font->CreateFontIndirect(&lf);
	return font;
	}

REDRAW_PARAMS::REDRAW_PARAMS(const CBigString& string, CDC *pDC, BOOL hilight, LPBYTE frame_buffer,
		CPoint frame_buffer_ofs, CSize frame_buffer_size, INT32 text_color, INT32 hotlink_color,
		INT32 old_hotlink_color, INT32 back_color, CPictureInfo& picture_info)
	: m_format_font(pDC), m_plain_text(string), m_picture_info( picture_info )
	{
	m_pDC = pDC;
	m_hilight = hilight;
	m_frame_buffer = frame_buffer;
	m_frame_buffer_ofs = frame_buffer_ofs;
	m_frame_buffer_size = frame_buffer_size;
	m_text_color = text_color;
	m_hotlink_color = hotlink_color;
	m_old_hotlink_color = old_hotlink_color;
	m_back_color = back_color;
	}

REDRAW_PARAMS::~REDRAW_PARAMS()
	{
	}

FORMAT_PARAMS::FORMAT_PARAMS(INT32 window_width, INT32 page_height, const char *base_url, CPictureInfo& picture_info, POSITION start_position, CDC *pDC, CNotifyObject *notify)
	: m_picture_info( picture_info )
	{
	m_window_width = window_width;
	m_page_height = page_height;
	m_pDC = pDC;
	m_base_url = base_url;
	m_start_position = start_position;
	m_first_new_format_item_pos = NULL;
	m_notify_object = notify;
	}


/*CMimeDynamicLoad *FORMAT_PARAMS::LoadPicture( LPCSTR url )
	{
	return (CMimeDynamicLoad *)m_notify_object->LoadDynamicObject( ConstructCMimeDynamicLoad, url, DEBUG_NEW CMethod(CMethod::get), NULL );
	}
*/

void CFormatTextItem::OnRedraw(REDRAW_PARAMS& params)
	{
	CString tstring;
	params.m_plain_text.GetString(tstring, m_text_ofs, m_text_len);

	CRect extent = GetExtent();

	params.m_format_font.TextOut(m_flags, params.m_text_color, extent.left, extent.top, tstring, m_text_len);
	}

CFormatHotlinkTextItem::CFormatHotlinkTextItem(CHotlink * hotlink, INT32 text_ofs, INT32 text_len, INT32 flags, const CRect& extent, CFormatFrame *frame)
	: CFormatTextItem(text_ofs, text_len, flags, extent, frame)
	{
	m_item_type = ITEM_HOTLINK_TEXT;
	
	m_hotlink = hotlink;
	}

void CFormatHotlinkTextItem::OnRedraw(REDRAW_PARAMS& params)
	{
	CString tstring;
	params.m_plain_text.GetString(tstring, m_text_ofs, m_text_len);

	CRect extent = GetExtent();
		
	params.m_format_font.TextOut(m_flags, params.m_hotlink_color, extent.left, extent.top, tstring, m_text_len);
	}

void CFormatHotlinkTextItem::OnClick(INT32 x, INT32 y)
	{
	}
	
CFormatPictureItem::CFormatPictureItem(LPCSTR url, const CRect& extent, CFormatFrame *frame)
	: CFormatItem(ITEM_PICTURE, extent, frame) 
	{
	m_url = url;
	}

void CFormatPictureItem::OnRedraw(REDRAW_PARAMS& params )
	{
	CRect r;

	CPicture *picture ;
	if (params.m_picture_info.GetPicture(m_url, picture) )
		DrawPicture( picture, params, GetExtent() );
	}

CFormatHotlinkPictureItem::CFormatHotlinkPictureItem(CHotlink *hotlink, INT32 border_size, BOOL is_mapped, LPCSTR url, const CRect& extent, CFormatFrame *frame)
	: CFormatPictureItem(url, extent, frame) 
	{
	m_border_size = border_size;
	m_hotlink = hotlink;
	m_item_type = ITEM_HOTLINK_PICTURE;
	m_is_mapped = is_mapped;
	}

LPCSTR CFormatPictureItem::GetPicture() const
	{
	return m_url;
	}

void CFormatHotlinkPictureItem::OnClick(INT32 x, INT32 y)
	{
	}

void CFormatHotlinkPictureItem::OnRedraw(REDRAW_PARAMS& params)
	{
	CRect r, extent( GetExtent() );
	
	r.left = extent.left + m_border_size;
	r.top = extent.top + m_border_size;
	r.right = extent.right- m_border_size;
	r.bottom = extent.bottom - m_border_size;

	CPicture *picture;
	if (params.m_picture_info.GetPicture(m_url, picture) )
		DrawPicture( picture, params, r);
	
	if (m_border_size > 0)
		{
		CPen pen;
		
		if (params.m_hilight)
			pen.CreatePen( PS_SOLID, (int)m_border_size, RGB(255,0,0));
		else
			pen.CreatePen( PS_SOLID, (int)m_border_size, params.m_hotlink_color);

		CPen *oldpen = params.m_pDC->SelectObject(&pen);
		params.m_pDC->SelectStockObject(HOLLOW_BRUSH);
		
		CRect frame;
		
		frame.left = r.left - m_border_size;
		frame.top = r.top - m_border_size;
		frame.right = r.right + m_border_size;
		frame.bottom = r.bottom + m_border_size;
	
		params.m_pDC->Rectangle(&frame);
		params.m_pDC->SelectObject(oldpen);
		}
	}

CFormatBulletItem::CFormatBulletItem(BULLET_TYPE bullet_type, INT32 numeric, const CRect& extent, CFormatFrame *frame)
	: CFormatItem(ITEM_BULLET, extent, frame)
	{
	m_bullet_type = bullet_type;
	m_numeric = numeric;
	}
	
void CFormatBulletItem::OnRedraw(REDRAW_PARAMS& params)
	{
	CRect extent ( GetExtent() );

	if (m_bullet_type == BULLET_NUMERIC)
		{
		char buffer[32];

		sprintf(buffer, "%d.", (int)m_numeric );

		params.m_format_font.TextOut(3, params.m_text_color, extent.left, extent.top, buffer, strlen(buffer) );	
		}
	else
		{
		extent.right++;
		extent.bottom++;
		CBrush brush;
		CPen pen;
		COLORREF color = params.m_pDC->GetNearestColor( ConvertToColorRef( params.m_text_color) );

		brush.CreateSolidBrush( color );
		pen.CreatePen(PS_SOLID, 1, color );
	
		switch( m_bullet_type )
			{
			case BULLET_HOLLOW_CIRCLE:
				params.m_pDC->SelectObject(&pen);
				params.m_pDC->SelectStockObject( HOLLOW_BRUSH );
				params.m_pDC->Ellipse(&extent);
				break;
			case BULLET_FILLED_CIRCLE:
				params.m_pDC->SelectStockObject( NULL_PEN );
				params.m_pDC->SelectObject(&brush);
				params.m_pDC->Ellipse(&extent);
				break;
			case BULLET_HOLLOW_SQUARE:
				params.m_pDC->SelectObject(&pen);
				params.m_pDC->SelectStockObject( HOLLOW_BRUSH );
				params.m_pDC->Rectangle(&extent);
				break;
			case BULLET_FILLED_SQUARE:
				params.m_pDC->SelectStockObject( NULL_PEN );
				params.m_pDC->SelectObject(&brush);
				params.m_pDC->Rectangle(&extent);
				break;
			}
		params.m_pDC->SelectStockObject( BLACK_PEN );
		params.m_pDC->SelectStockObject( HOLLOW_BRUSH );
		}
	}

CFormatRuleItem::CFormatRuleItem(const CRect& extent, CFormatFrame *frame)
	: CFormatItem(ITEM_RULE, extent, frame)
	{
	}
	
void CFormatRuleItem::OnRedraw(REDRAW_PARAMS& params)
	{
	params.m_pDC->SelectStockObject(GRAY_BRUSH);
	params.m_pDC->SelectStockObject(NULL_PEN);

	CRect extent( GetExtent() );

	params.m_pDC->Rectangle( extent.left, extent.top, extent.right+1, extent.bottom+1 );
	}

CFormatFont::CFormatFont(CDC *pDC)
	{
	m_flags = -1;
	m_last_font = NULL;
	m_pDC = pDC;
	}

CFormatFont::~CFormatFont()
	{
	if (m_last_font)
		{
		m_pDC->SelectStockObject(SYSTEM_FONT);
		delete m_last_font;
		}
	}

void CFormatFont::TextOut(INT32 flags, INT32 color, INT32 x, INT32 y, LPCSTR string, INT32 string_len)
	{
	UpdateChanged(flags);
	ASSERT(color != -1);
	m_pDC->SetTextColor( ConvertToColorRef(color) );
	m_pDC->TextOut(x,y,string, string_len);
	}

BOOL _GetTextExtentExPoint(

    HDC  hdc,	// handle of device context 
    LPCTSTR  lpszStr,	// address of character string 
    int  cchString,	// number of characters in string 
    int  nMaxExtent,	// maximum width for formatted string 
    LPINT  lpnFit,	// address of value specifying max. number of chars.  
    LPSIZE  lpSize 	// address of structure with string dimensions 
   )
   {
/*// do a binary search for the nearest match

	int amount = cchString/2, addamount = amount/2;
	SIZE  bound;


	while(addamount > 0)
		{
		GetTextExtentPoint32(hdc, lpszStr, amount, &bound);

		if (bound.cx > nMaxExtent)
			{
			amount -= addamount;
			}
		else
			{
			amount += addamount;
			}
		addamount/=2;
		}
	
	*lpSize = bound;
	*lpnFit = amount;

	ASSERT(amount > 0);
   */ 
   return GetTextExtentExPoint(hdc, lpszStr, cchString, nMaxExtent, lpnFit, NULL, lpSize);
   //return TRUE;
   }

void CFormatFont::UpdateChanged(INT32 new_flags)
	{
	if (new_flags != m_flags)
		{
		m_flags = new_flags;
		CFont * font = CreateHTMLFont(m_pDC, m_flags);
		m_pDC->SelectObject( font );
		if (m_last_font)
			delete m_last_font;

		m_last_font = font;
		m_pDC->GetTextMetrics(&m_last_text_metrics);
		}
	}

INT32 CFormatFont::GetLineBreak(INT32 new_flags, BOOL line_empty, const char *text, INT32 string_len, INT32 string_width)
	{
	UpdateChanged(new_flags);

	INT len;
	SIZE strsize;

	_GetTextExtentExPoint(m_pDC->m_hAttribDC, text, string_len, string_width, &len, &strsize);
	return len;
	}

INT32 CFormatFont::GetStringWidth(INT32 new_flags, LPCSTR string, INT32 string_len)
	{
	UpdateChanged(new_flags);

	SIZE strsize;

	GetTextExtentPoint32(m_pDC->m_hAttribDC, string, string_len, &strsize);
	return strsize.cx;
	}

INT32 CFormatFont::GetMaxCharWidth(INT32 new_flags)
	{
	UpdateChanged(new_flags);

	return m_last_text_metrics.tmMaxCharWidth;
	}

INT32 CFormatFont::GetAscent(INT32 new_flags)
	{
	UpdateChanged(new_flags);

	return m_last_text_metrics.tmAscent;
	}

INT32 CFormatFont::GetDescent(INT32 new_flags)
	{
	UpdateChanged(new_flags);

	return m_last_text_metrics.tmDescent;
	}

void CViewhtmlView::OnRButtonDown(UINT nFlags, CPoint point)
{
	CScrollView::OnRButtonDown(nFlags, point);
}
 
void CViewhtmlView::OnRButtonDown2(UINT nFlags, CPoint point, CPoint cursorPos) 
{
	// First select the active object 
	OnLButtonDblClk(0, point);

	if (m_pSelection != NULL)
	{
		// If there is an active OCX, create a popup menu for that

		CreatePopupMenu(cursorPos);
	}	
	

	CScrollView::OnRButtonDown(nFlags, point);
}

void CViewhtmlView::CreatePopupMenu(CPoint point)
{

	CMenu	bar;
	if (bar.LoadMenu(IDR_MENU1))
	{
		CMenu& popup = *bar.GetSubMenu(0);
		ASSERT(popup.m_hMenu != NULL);

		//ClientToScreen(&point);
		popup.TrackPopupMenu(TPM_LEFTALIGN, point.x,
			point.y, AfxGetMainWnd());
	}
}

#ifdef USES_OLE_CONTROLS
CFormatOLEControlItem::CFormatOLEControlItem(const CLSID& clsid, LPCSTR ocx_url, INT32 parse_id, LPCSTR version, CDynLoadMemFile * dyn_mem, const CRect& extent, CFormatFrame *frame)
	: CFormatItem( ITEM_OLECONTROL, extent,frame)
	{				 
	m_ocx_url = ocx_url;
	m_parse_id = parse_id;
	m_clsid = clsid;
	m_version = version;
	m_memfile = dyn_mem;
	}

void CFormatOLEControlItem::OnRedraw(REDRAW_PARAMS& params)
	{
	if (params.m_pDC->IsPrinting())
		{
		// draw metafile representation of control to dc.
		params.m_pDC->SelectStockObject(BLACK_BRUSH);
		params.m_pDC->Rectangle( GetExtent() );
		}
	else
		{
		// draw the control
//		params.m_pDC->SelectStockObject(BLACK_BRUSH);
//		params.m_pDC->Rectangle( m_extent );
		}
	}
#endif

BOOL CViewhtmlView::OnEraseBkgnd(CDC* pDC) 
{
	// no background erasing needed
	return TRUE;	
}

void CViewhtmlView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CScrollView::OnMouseMove(nFlags, point);
}

void CViewhtmlView::ClearBackground(CDC *pDC, const CRect& extent)
	{
	CPicture *picture = NULL;
	if (!m_background_picture_url.IsEmpty())
		{
		GetDocument()->GetPictureInfo().GetPicture( m_background_picture_url, picture );
		}

	BOOL transp = FALSE, header_read = FALSE;
	if (picture)
		{
		picture->DEBUG_LOCK();
		transp = picture->IsTransparent();
		header_read = picture->IsHeaderRead();
		picture->Unlock();
		}
		

	if (!picture || transp == TRUE || !header_read)
		{
		// clear the background with a solid color when there is either
		// no background picture or the background picture has a transparent
		// color

		CBrush brush;
		brush.CreateSolidBrush( pDC->GetNearestColor( ConvertToColorRef(m_back_color) ) );
	
		CBrush *oldbrush = pDC->SelectObject(&brush);
		pDC->SelectStockObject( NULL_PEN);
		pDC->Rectangle( &extent);
		pDC->SelectObject(oldbrush);
		}

	if (picture && header_read)
		{
		REDRAW_PARAMS redraw_params(m_format_html->GetPlainText(), pDC, FALSE, 
			m_double_buffer_ptr, pDC->GetViewportOrg(), m_double_buffer_view_size,
			m_text_color, m_hotlink_color, m_old_hotlink_color, m_back_color, GetDocument()->GetPictureInfo() );

		picture->DEBUG_LOCK();
		CSize s = picture->GetSize();
		picture->Unlock();

		INT32 i,j,
			ilimit = (extent.right + s.cx - 1)/ s.cx,
			jlimit = (extent.bottom + s.cy - 1) / s.cy;

		for (j = (extent.top / s.cy) ; j < jlimit; j++)
			{
			for ( i = (extent.left / s.cx); i < ilimit; i++)
				{
				DrawPicture( picture, redraw_params,
					CRect( i * s.cx, j * s.cy, (i+1) * s.cx,
						(j+1) * s.cy ) );
				}
			}
		}
	}
	

void CViewhtmlView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	pCmdUI->ContinueRouting();
}

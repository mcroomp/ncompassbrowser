// cntritem.cpp : implementation of the CViewhtmlCntrItem class
//

#include "cross_p.h"
#include "viewhtml.h"

#include "viewhdoc.h"

#ifdef USES_OLE_CONTROLS
#include "cntlitem.h"
#endif

#include "cntritem.h"

#ifndef _FMTHTML_H_
#include "fmthtml.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

	
/////////////////////////////////////////////////////////////////////////////
// CViewhtmlCntrItem implementation

//IMPLEMENT_SERIAL(CViewhtmlCntrItem, CControlItem, 0)

IMPLEMENT_DYNAMIC(CViewhtmlCntrItem, CControlItem);

CViewhtmlCntrItem::CViewhtmlCntrItem(INT32 parseID, 
									 CRect rect,
									 CString ocxURL,  
									 CViewhtmlDoc* pContainer)
	: CControlItem(rect, ocxURL, pContainer)
{
	// TODO: add one-time construction code here
//	m_rect = rect;				// An Initial Size for the control
//	m_extent = CSize(0, 0);		// Set extent to zero
	m_parseID = parseID;
}

CViewhtmlCntrItem::~CViewhtmlCntrItem()
{
	// TODO: add cleanup code here
}

void CViewhtmlCntrItem::Invalidate(CView* pNotThisView)
{
	GetDocument()->UpdateAllViews(pNotThisView, 0, this);
}

void CViewhtmlCntrItem::OnChange(OLE_NOTIFICATION nCode, DWORD dwParam)
{
	ASSERT_VALID(this);

	CControlItem::OnChange(nCode, dwParam);

	// When an item is being edited (either in-place or fully open)
	//  it sends OnChange notifications for changes in the state of the
	//  item or visual appearance of its content.

	// TODO: invalidate the item by calling UpdateAllViews
	//  (with hints appropriate to your application)

	GetDocument()->UpdateAllViews(NULL);
		// for now just update ALL views/no hints
	switch (nCode)
	{    
		case OLE_CHANGED:
			UpdateExtent();
			Invalidate();
			break;
		case OLE_CHANGED_ASPECT:
		case OLE_CHANGED_STATE:
			Invalidate();
			break;
	}

}

BOOL CViewhtmlCntrItem::OnChangeItemPosition(const CRect& rectPos)
{
	ASSERT_VALID(this);

	// During in-place activation CViewhtmlCntrItem::OnChangeItemPosition
	//  is called by the server to change the position of the in-place
	//  window.  Usually, this is a result of the data in the server
	//  document changing such that the extent has changed or as a result
	//  of in-place resizing.
	//
	// The default here is to call the base class, which will call
	//  COleClientItem::SetItemRects to move the item
	//  to the new position.

	return TRUE;

	if (!COleClientItem::OnChangeItemPosition(rectPos))
		return FALSE;

	// TODO: update any cache you may have of the item's rectangle/extent
	// Erase the old image of the item, set the new position and draw again.
	Invalidate();
	m_rect = rectPos;
	Invalidate();

	// mark document as dirty
	GetDocument()->SetModifiedFlag();

	return TRUE;
}

void CViewhtmlCntrItem::OnGetItemPosition(CRect& rPosition)
{
	ASSERT_VALID(this);

	// During in-place activation, CViewhtmlCntrItem::OnGetItemPosition
	//  will be called to determine the location of this item.  The default
	//  implementation created from AppWizard simply returns a hard-coded
	//  rectangle.  Usually, this rectangle would reflect the current
	//  position of the item relative to the view used for activation.
	//  You can obtain the view by calling CViewhtmlCntrItem::GetActiveView.

	// TODO: return correct rectangle (in pixels) in rPosition

	if (m_rect.Size() == CSize(0,0))
        UpdateItemExtentFromServer();

    // copy m_rect, which is in document coordinates
    rPosition = m_rect;

	// TODO:  Change any document coordinates here.
    return;
}

void CViewhtmlCntrItem::OnDeactivateUI(BOOL bUndoable)
{
	COleClientItem::OnDeactivateUI(bUndoable);

	// Close an in-place active item whenever it removes the user
	//  interface.  The action here should match as closely as possible
	//  to the handling of the escape key in the view.

	// Deactivate();   // nothing fancy here -- just deactivate the object
}

void CViewhtmlCntrItem::OnUpdateFrameTitle()
{
	// Do Nothing.  A Good OCX Container doesn't display the name
	// of its UIActive control.

	// Optionally, you could switch on the registry entry of the UIActive
	// item to determine if it is not a control and then call the base
	// class to update the title properly.
}

void CViewhtmlCntrItem::Serialize(CArchive& ar)
{
	ASSERT_VALID(this);

	// Call base class first to read in COleClientItem data.
	// Since this sets up the m_pDocument pointer returned from
	//  CViewhtmlCntrItem::GetDocument, it is a good idea to call
	//  the base class Serialize first.
	COleClientItem::Serialize(ar);

	// now store/retrieve data specific to CViewhtmlCntrItem
	if (ar.IsStoring())
	{
		// TODO: add storing code here
		ar << m_rect << m_extent;
	}
	else
	{
		// TODO: add loading code here
		ar >> m_rect >> m_extent;

	}
}
	

/////////////////////////////////////////////////////////////////////////////
//  Operations

                                        
void CViewhtmlCntrItem::Move(CRect &rc)
{
	// invalidate old rect
	Invalidate();
	// invalidate new
	m_rect = rc;
	Invalidate();

	// update item rect when in-place active
	if (IsInPlaceActive())
		SetItemRects();
}

BOOL CViewhtmlCntrItem::UpdateItemExtentFromServer()
{
    // get size in pixels
    CSize size;
    if (!GetExtent(&size))
        return FALSE;       // blank

    if (size == m_extent)
        return FALSE;

    // if new object (i.e. m_extent is empty) setup position
    if (m_extent == CSize(0,0))
    {
        m_rect.right = m_rect.left + MulDiv(size.cx,10,254);
        m_rect.bottom = m_rect.top - MulDiv(size.cy,10,254);
    }
    else
    {
        if (!IsInPlaceActive() && size != m_extent)
        {
            // data changed and not inplace, so scale up rect as well
            m_rect.right = m_rect.left +
                MulDiv(m_rect.Width(),size.cx,m_extent.cx);
            m_rect.bottom = m_rect.top +
                MulDiv(m_rect.Height(),size.cy,m_extent.cy);
        }
    }

    m_extent = size;
    Invalidate();   // as well as the new size/position
    return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CViewhtmlCntrItem diagnostics

#ifdef _DEBUG
void CViewhtmlCntrItem::AssertValid() const
{
	COleClientItem::AssertValid();
}

void CViewhtmlCntrItem::Dump(CDumpContext& dc) const
{
	COleClientItem::Dump(dc);
}
#endif

/////////////////////////////////////////////////////////////////////////////

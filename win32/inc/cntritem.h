// cntritem.h : interface of the CViewhtmlCntrItem class
//

class CViewhtmlDoc;
class CViewhtmlView;

class CViewhtmlCntrItem : public CControlItem
{
//	DECLARE_SERIAL(CViewhtmlCntrItem)
DECLARE_DYNAMIC(CViewhtmlCntrItem);
// Constructors
public:
	CViewhtmlCntrItem(INT32 parseID, CRect rect, CString ocxURL, 
					  CViewhtmlDoc* pContainer = NULL);
		// Note: pContainer is allowed to be NULL to enable IMPLEMENT_SERIALIZE.
		//  IMPLEMENT_SERIALIZE requires the class have a constructor with
		//  zero arguments.  Normally, OLE items are constructed with a
		//  non-NULL document pointer.

// Attributes
public:
	CViewhtmlDoc* GetDocument()
		{ return (CViewhtmlDoc*)COleClientItem::GetDocument(); }
	CViewhtmlView* GetActiveView()
		{ return (CViewhtmlView*)COleClientItem::GetActiveView(); }

	// Format related attributes
	INT32		m_parseID;

	// Size attributes
//	CRect		m_rect;
//	CSize		m_extent;

// Operations
public:
	void Move(CRect &rc);
	void Invalidate(CView* pNotThisView = NULL);
    BOOL UpdateItemExtentFromServer();


	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewhtmlCntrItem)
	public:
	virtual void OnChange(OLE_NOTIFICATION wNotification, DWORD dwParam);
	protected:
	virtual void OnGetItemPosition(CRect& rPosition);
	virtual void OnDeactivateUI(BOOL bUndoable);
	virtual BOOL OnChangeItemPosition(const CRect& rectPos);
	virtual BOOL OnUpdateFrameTitle();
	//}}AFX_VIRTUAL

// Implementation
public:
	~CViewhtmlCntrItem();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void Serialize(CArchive& ar);


};

/////////////////////////////////////////////////////////////////////////////

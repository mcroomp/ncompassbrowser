// viewhdoc.h : interface of the CViewhtmlDoc class
//
/////////////////////////////////////////////////////////////////////////////

#define _VIEWHDOC_H_

/////////////////////////////////////////////////////////////////////////////
// definitions

#define MAX_STD_APROP_CT (32)
#define MAX_NONSTD_APROP_CT (32)

#define DISPID_AMBIENT_USERDIRECTORY	(-720)
/////////////////////////////////////////////////////////////////////////////
// Global Functions

HRESULT TFVarCopy (VARIANT * pvarDest, VARIANT * pvarSrc);

#ifdef USES_OLE_CONTROLS
#include "cntlinfo.h"
#endif

#ifndef _FMTHTML_H_
#include "fmthtml.h"
#endif
		
class CViewhtmlDoc : public COleDocument, public CNotifyObject
{
protected: // create from serialization only
	CViewhtmlDoc();
	DECLARE_DYNCREATE(CViewhtmlDoc)

// Attributes
public:
	BOOL			m_add_to_back_array;

	// A list of CControlItems in the document
	CMapStringToOb	m_cntlItemList;
protected:

#ifdef USES_OLE_CONTROLS
	// Ambient Property Attributes	 - Kerem 6/28/95
    APROP 			m_apropStd[MAX_STD_APROP_CT];
    int 			m_nStdApropCt;
    APROP			m_apropDlgStd[MAX_STD_APROP_CT];
    int 			m_nDlgStdApropCt;
    APROP 			m_apropNonstd[MAX_NONSTD_APROP_CT];
    int 			m_nNonstdApropCt;
    APROP 			m_apropDlgNonstd[MAX_NONSTD_APROP_CT];
    int 			m_nDlgNonstdApropCt;
#endif

    CFontHolder* 	m_lpFontHolder;
    CStringW 		m_strFaceName;
    FONTDESC 		m_fntdesc;

// Operations
public:
	CMimeObject *GetMimeObject() const;
	virtual void OnNotify( UINT32 change_flags, CDynamicLoad *source);
	CPictureInfo& GetPictureInfo();
	void OpenURL( LPCSTR lpszPathName, METHOD_TYPE method = METHOD_GET, LPCSTR post_headers = NULL, LPCSTR post_data = NULL );
	BOOL IsBetweenLoads();
		// returns TRUE if we are in the process of loading something which has not been
		// established by the view yet.

// Control container related methods
#ifdef USES_OLE_CONTROLS
	void AnnounceApropChange (DISPID dispidAprop);
    BOOL SetDefaultAmbientProps();
    void DestroyAmbientProps(BOOL bInitializing = FALSE);
    void DestroyDlgAmbientProps(BOOL bInitializing = FALSE);
    LPAPROP FindAprop (DISPID dispid);
    LPAPROP FindAprop (const TCHAR * pszName);
#endif

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewhtmlDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	virtual void OnCloseDocument();
	virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);
	protected:
	virtual BOOL SaveModified();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CViewhtmlDoc();
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CMimeDynamicLoad *m_loading_mime_object;
	CMimeObject *m_current_mime_object;
	CString m_anchor;
	CPictureInfo m_picture_info;

// Generated message map functions
protected:
	//{{AFX_MSG(CViewhtmlDoc)
	afx_msg void OnLoadImages();
	afx_msg void OnUpdateLoadImages(CCmdUI* pCmdUI);
	afx_msg void OnReload();
	afx_msg void OnUpdateReload(CCmdUI* pCmdUI);
	afx_msg void OnStopLoading();
	afx_msg void OnUpdateStopLoading(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CViewhtmlDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
};

/////////////////////////////////////////////////////////////////////////////

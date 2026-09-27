// viewhdoc.cpp : implementation of the CViewhtmlDoc class
//

#include "cross_p.h"
#include "viewhtml.h"

#include "mainfrm.h"
#include "cntlinfo.h"
#include "viewhdoc.h"
#include "viewhvw.h"
#include "cntlitem.h"
#include "cntritem.h"
#include "dopenurl.h"

#ifndef _SMTP_H_
#include "smtp.h"
#endif

#ifndef _PICTURE_H_
#include "picture.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewhtmlDoc

IMPLEMENT_DYNCREATE(CViewhtmlDoc, COleDocument)

BEGIN_MESSAGE_MAP(CViewhtmlDoc, COleDocument)
	//{{AFX_MSG_MAP(CViewhtmlDoc)
	ON_COMMAND(ID_LOAD_IMAGES, OnLoadImages)
	ON_UPDATE_COMMAND_UI(ID_LOAD_IMAGES, OnUpdateLoadImages)
	ON_COMMAND(ID_RELOAD, OnReload)
	ON_UPDATE_COMMAND_UI(ID_RELOAD, OnUpdateReload)
	ON_COMMAND(ID_STOP_LOADING, OnStopLoading)
	ON_UPDATE_COMMAND_UI(ID_STOP_LOADING, OnUpdateStopLoading)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	//}}AFX_MSG_MAP
	// Enable default OLE container implementation
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, COleDocument::OnUpdatePasteMenu)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_LINK, COleDocument::OnUpdatePasteLinkMenu)
	ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_LINKS, COleDocument::OnUpdateEditLinksMenu)
	ON_COMMAND(ID_OLE_EDIT_LINKS, COleDocument::OnEditLinks)
	ON_UPDATE_COMMAND_UI(ID_OLE_VERB_FIRST, COleDocument::OnUpdateObjectVerbMenu)
	ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_CONVERT, COleDocument::OnUpdateObjectVerbMenu)
	ON_COMMAND(ID_OLE_EDIT_CONVERT, COleDocument::OnEditConvert)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CViewhtmlDoc, COleDocument)
	//{{AFX_DISPATCH_MAP(CViewhtmlDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//      DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewhtmlDoc construction/destruction

CViewhtmlDoc::CViewhtmlDoc()
	{
	// For most containers, using compound files is a good idea.
	//EnableCompoundFile();		// we don't want compound files

	// TODO: add one-time construction code here
	// Setup the ambient properties of OCX container - Kerem 6/28/95

	theApp.m_viewhtml_doc = this;

	m_picture_info.Reset(this);
	
	m_lpFontHolder = NULL;
	DestroyAmbientProps(TRUE);
	DestroyDlgAmbientProps(TRUE);
	SetDefaultAmbientProps();

	EnableAutomation();

	AfxOleLockApp();

	m_loading_mime_object = NULL;
	m_current_mime_object = NULL;
	}

CViewhtmlDoc::~CViewhtmlDoc()
	{
	// Get rid of the ambient props - Kerem 6/28/95
    DestroyAmbientProps();
    DestroyDlgAmbientProps();
    if (m_lpFontHolder) 
        m_lpFontHolder->ReleaseFont();

	AfxOleUnlockApp();

	// Kerem 6/28/95
	delete m_lpFontHolder;
	m_lpFontHolder = NULL;

	theApp.m_viewhtml_doc = NULL;
	}

BOOL CViewhtmlDoc::OnNewDocument()
{
	if (!COleDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	m_current_mime_object = NULL;
	m_loading_mime_object = NULL;
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CViewhtmlDoc serialization

void CViewhtmlDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
		{
		// TODO: add storing code here
		}
	else
		{
		// No loading code necessary, document is loaded dynamically
		}

	// Calling the base class COleDocument enables serialization
	//  of the container document's COleClientItem objects.
	//COleDocument::Serialize(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CViewhtmlDoc diagnostics

#ifdef _DEBUG
void CViewhtmlDoc::AssertValid() const
{
	COleDocument::AssertValid();
}

void CViewhtmlDoc::Dump(CDumpContext& dc) const
{
	COleDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CViewhtmlDoc commands

BOOL CViewhtmlDoc::OnOpenDocument(LPCTSTR lpszPathName) 
	{
	// this routine should never get called
	ASSERT(FALSE);
	
	return FALSE;
	}

void CViewhtmlDoc::OpenURL( LPCSTR lpszPathName, METHOD_TYPE method, LPCSTR post_headers, LPCSTR post_data )
	{
	if (memicmp(lpszPathName, "mailto:",7) == 0 && method == METHOD_GET)
		{
		CProtocolSMTP::ShowDialog(lpszPathName);
		return;
		}

	LPCSTR pound_sign = strrchr(lpszPathName, '#');
	CString url;

	if (pound_sign)
		{
		url = CString( lpszPathName, pound_sign - lpszPathName );
		m_anchor = pound_sign+1;
		}
	else
		{
		url = lpszPathName;		
		m_anchor.Empty();
		}

	// If we are already loading a document that hasn't established a mime type, kill it first
	if (IsBetweenLoads())
		{
		m_loading_mime_object->AbortLoading();
		delete m_loading_mime_object;
		}

	// Start loading the new document (while keeping current document)
	m_loading_mime_object = new CMimeDynamicLoad( this, url, method, post_headers, post_data );
	m_loading_mime_object->StartLoading();

	m_loading_mime_object->DEBUG_LOCK();
	CMimeObject *mime_object = m_loading_mime_object->GetMimeObject();
	m_loading_mime_object->Unlock();
	
	// If the document was cached, then there is no need to wait
	if (mime_object != NULL)
		{
		SetPathName( mime_object->GetURL() );
		}
	}
	
CMimeObject * CViewhtmlDoc::GetMimeObject() const
	{
	return m_current_mime_object;
	}
										

void CViewhtmlDoc::OnNotify( UINT32 change_flags, CDynamicLoad *source )
	{
	// notify all views of the change

	if (source == m_loading_mime_object )
		{
		m_loading_mime_object->DEBUG_LOCK();
		CMimeObject *mime_object = m_loading_mime_object->GetMimeObject();
		CString newurl = m_loading_mime_object->GetURL();
		m_loading_mime_object->Unlock();

		if (mime_object && mime_object->UsesInternalViewer() == FALSE)
			{
			if (change_flags & CHANGEFLAG_DONE)
				{
				CDynamicLoad *t = m_loading_mime_object;
				m_loading_mime_object = NULL;
				mime_object->LaunchViewer();
				delete t;
				}
			// don't allow views to see these update messages, since they use an external viewer
			return;
			}

		if (change_flags & CHANGEFLAG_URL_REDIRECT)
			{
			SetPathName( newurl );

			if (!mime_object)
				return;
			}

		/* Trigger change of document when either of the following conditions occcurs:

		1) The mime header is read, in which cas the mime_object must exist
		2) There was a redirect, and the mime object was cached, in which case
		   we must also trigger a change
		3) The mime object was in the cache, in which case we must also immediately switch

		*/
		if ((change_flags & CHANGEFLAG_MIMEHEADER_READ) ||
			((change_flags & CHANGEFLAG_URL_REDIRECT) && mime_object) ||
			(change_flags & CHANGEFLAG_CACHED))
			{
			ASSERT(mime_object);

			if (m_current_mime_object != mime_object )
				{
				if (m_add_to_back_array && m_current_mime_object)
					{
					theApp.m_forward_array.RemoveAll();
					theApp.m_back_array.Add(m_current_mime_object->GetURL() );
					}

				DeleteContents();
				m_current_mime_object = mime_object;

				source->DEBUG_LOCK();
				CString newurl = source->GetURL();
				source->Unlock();

				SetPathName( newurl );

				theApp.m_viewhtml_view->OnInitialUpdate();
				theApp.m_viewhtml_view->ScrollToPosition( CPoint(0,0) );
				}
			}

		if (change_flags & CHANGEFLAG_DONE )
			{
			if (mime_object == NULL)
				{
				m_loading_mime_object->DEBUG_LOCK();
				CString error_message = m_loading_mime_object->GetErrorMessage();
				m_loading_mime_object->Unlock();
				delete m_loading_mime_object;
				m_loading_mime_object = NULL;

				// Show the user the error message if there was one
				if (!error_message.IsEmpty())
					AfxMessageBox( error_message );

				// don't allow views to see this update message, 
				// since nothing was loaded
				return;
				}
			else
				{
				if (!m_anchor.IsEmpty())
					{
					theApp.m_viewhtml_view->ScrollToAnchor(m_anchor);
					}
				}
			}
		}
	
	theApp.m_viewhtml_view->OnDynamicNotify(change_flags, source);
	}

void CViewhtmlDoc::DeleteContents() 
	{
	m_picture_info.Reset(this);

	m_current_mime_object = NULL;

	if (m_loading_mime_object)
		{
		// delete everything except for the object we are loading
		POSITION walk = GetFirstDynamicLoadPosition();
		while(walk)
			{
			CDynamicLoad *dlobject = GetNextDynamicLoad(walk);
			if (m_loading_mime_object != dlobject)
				{
				dlobject->AbortLoading();
				delete dlobject;
				}
			}
		}
	else
		{
		// just kill everything
		POSITION walk = GetFirstDynamicLoadPosition();
		while(walk)
			{
			CDynamicLoad *dlobject = GetNextDynamicLoad(walk);
			dlobject->AbortLoading();
			delete dlobject;
			}
		}

	// view may be deleted at this point so don't use global variable
	POSITION pos = GetFirstViewPosition();
	while(pos)
		{
		CViewhtmlView *view = (CViewhtmlView *)GetNextView( pos );
		view->ClearView();
		}
	
	pos = GetStartPosition();
	
	while (pos != NULL)
	{
		CViewhtmlCntrItem* pItem = (CViewhtmlCntrItem*) GetNextItem(pos);
		
		// First clean up any waiting OCXs on the list.
		COleInfo*	oleInfo;
		if (!m_cntlItemList.IsEmpty())
		{ 
			if (m_cntlItemList.Lookup(pItem->m_szObjCLSID, (CObject*&) oleInfo) )
			{
				if (oleInfo != NULL)
				{
					oleInfo->m_objectList.RemoveAll();
					delete oleInfo;
				}
				m_cntlItemList.RemoveKey(pItem->m_szObjCLSID);
			}

		}
		// Now unload the file loader notify objects...
		pItem->ExitFileLoaders();
	
		// Now we are ready to totally get rid of the OCX.
		UINT state = pItem->GetItemState();
		if (state == COleClientItem::activeState || 
			state == COleClientItem::activeUIState)
		{
			pItem->Close();
		}
		if (pItem->m_lpObject == NULL)
		{
			RemoveItem(pItem);  		// disconnect from document
			pItem->InternalRelease();   // may 'delete pItem'
		}
	}
	m_cntlItemList.RemoveAll();
	
	COleDocument::DeleteContents();
}

/////////////////////////////////////////////////////////////////////////////
// CViewhtmlDoc commands (related to OCX container)

/////////////////////////////////////////////////////////////////////////////
// CViewhtmlDoc Ambient Property Handling Functions

BOOL CViewhtmlDoc::SetDefaultAmbientProps()
{
    BOOL bRet = TRUE;
    VARIANT FAR * lpVar;
    
    int nStdIdx = 0;

    m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_USERMODE;
    m_apropStd[nStdIdx].strName.LoadString (IDS_AMODENAME_USERMODE);
    lpVar = &m_apropStd[nStdIdx].varValue;
    V_VT (lpVar) = VT_BOOL;
    V_BOOL (lpVar) = (~(VARIANT_BOOL)0);
    m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTBOOL;

    m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_UIDEAD;
    m_apropStd[nStdIdx].strName.LoadString (IDS_AMODENAME_UIDEAD);
    lpVar = &m_apropStd[nStdIdx].varValue;
    V_VT (lpVar) = VT_BOOL;
    V_BOOL (lpVar) = 0;
    m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTBOOL;

    m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_SHOWHATCHING;
    m_apropStd[nStdIdx].strName.LoadString (IDS_AMODENAME_DISPLAYHATCHING);
    lpVar = &m_apropStd[nStdIdx].varValue;
    V_VT (lpVar) = VT_BOOL;
    //V_BOOL (lpVar) = (~(VARIANT_BOOL)0);
    V_BOOL (lpVar) = (VARIANT_BOOL)0;
    m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTBOOL;

    m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_SHOWGRABHANDLES;
    m_apropStd[nStdIdx].strName.LoadString (IDS_AMODENAME_DISPLAYGRABHANDLES);
    lpVar = &m_apropStd[nStdIdx].varValue;
    V_VT (lpVar) = VT_BOOL;
    //V_BOOL (lpVar) = (~(VARIANT_BOOL)0);
    V_BOOL (lpVar) = (VARIANT_BOOL)0;
    m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTBOOL;

    m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_TEXTALIGN;
    m_apropStd[nStdIdx].strName.LoadString (IDS_APROPNAME_TEXTALIGN);
    lpVar = &m_apropStd[nStdIdx].varValue;
    V_VT (lpVar) = VT_I2;
    V_I2 (lpVar) = 0;
    m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTI2;

    m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_BACKCOLOR;
    m_apropStd[nStdIdx].strName.LoadString (IDS_APROPNAME_BACKCOLOR);
    lpVar = &m_apropStd[nStdIdx].varValue;
    V_VT (lpVar) = VT_I4;
    V_I4 (lpVar) = GetSysColor (COLOR_WINDOW);
    m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTCOLOR;

    m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_FONT;
    m_apropStd[nStdIdx].strName.LoadString (IDS_APROPNAME_FONT);
    lpVar = &m_apropStd[nStdIdx].varValue;
    VariantClear (lpVar);
    {
        m_strFaceName = L"MS Sans Serif";
		m_fntdesc.cbSizeofstruct = sizeof(m_fntdesc);
        m_fntdesc.lpstrName = m_strFaceName.GetBuffer();
        m_fntdesc.cySize.Lo = 80000L;
        m_fntdesc.cySize.Hi = 0;
        m_fntdesc.sWeight = FW_BOLD;
        m_fntdesc.fItalic = FALSE;
        m_fntdesc.fUnderline = FALSE;
        m_fntdesc.fStrikethrough = FALSE;
        if (m_lpFontHolder) 
            m_lpFontHolder->ReleaseFont ();
        
        delete m_lpFontHolder;
		m_lpFontHolder = NULL;

        m_lpFontHolder = DEBUG_NEW CFontHolder (NULL);
        if (m_lpFontHolder) {
            m_lpFontHolder->InitializeFont (&m_fntdesc);
            V_VT (lpVar) = VT_DISPATCH;
            V_DISPATCH (lpVar) = m_lpFontHolder->GetFontDispatch ();
        }
    }
    m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTFONT;

    m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_FORECOLOR;
    m_apropStd[nStdIdx].strName.LoadString (IDS_APROPNAME_FORECOLOR);
    lpVar = &m_apropStd[nStdIdx].varValue;
    V_VT (lpVar) = VT_I4;
    V_I4 (lpVar) = GetSysColor (COLOR_WINDOWTEXT);
    m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTCOLOR;

    m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_SCALEUNITS;
    m_apropStd[nStdIdx].strName.LoadString (IDS_APROPNAME_SCALEUNITS);
    lpVar = &m_apropStd[nStdIdx].varValue;
    V_VT (lpVar) = VT_BSTR;
    V_BSTR (lpVar) = SysAllocStringLen (L"", 32);
    m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTBSTR;

    m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_DISPLAYNAME;
    m_apropStd[nStdIdx].strName.LoadString (IDS_APROPNAME_DISPLAYNAME);
    lpVar = &m_apropStd[nStdIdx].varValue;
    V_VT (lpVar) = VT_BSTR;
    V_BSTR (lpVar) = SysAllocStringLen (L"", 32);
    m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTBSTR;

    m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_LOCALEID;
    m_apropStd[nStdIdx].strName.LoadString (IDS_APROPNAME_LOCALEID);
    lpVar = &m_apropStd[nStdIdx].varValue;
    V_VT (lpVar) = VT_I4;
    V_I4 (lpVar) = GetUserDefaultLCID ();
    m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTI4;

    m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_MESSAGEREFLECT;
    m_apropStd[nStdIdx].strName.LoadString (IDS_APROPNAME_MESSAGEREFLECT);
    lpVar = &m_apropStd[nStdIdx].varValue;
    V_VT (lpVar) = VT_BOOL;
    V_BOOL (lpVar) = 0;
	//V_BOOL (lpVar) = (~(VARIANT_BOOL)0);
	m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTBOOL;
    
    m_nStdApropCt = nStdIdx;
    
    // Here are our non standard properties...

	int nNonstdIdx = 0;
    m_apropNonstd[nNonstdIdx].dispid = DISPID_AMBIENT_USERDIRECTORY;
    m_apropNonstd[nNonstdIdx].strName.LoadString (IDS_APROPNAME_USERDIRECTORY);
    lpVar = &m_apropNonstd[nNonstdIdx].varValue;
    V_VT (lpVar) = VT_BSTR;
    CString	temp = ((CViewhtmlApp *) AfxGetApp())->m_tempStorageDir;
    int	sLen = temp.GetLength();	
    CStringW wideTemp(temp);
    V_BSTR (lpVar) = SysAllocStringLen(wideTemp, sLen);
    m_apropNonstd[nNonstdIdx++].idsTypeInterp = IDS_VTBSTR;

	m_nNonstdApropCt = nNonstdIdx;

    return (bRet);
}

void CViewhtmlDoc::DestroyAmbientProps(BOOL bInitializing)
{
    int nApropIdx = 0;
    VARIANT FAR * lpVar;
    while (nApropIdx < MAX_STD_APROP_CT) 
    {
        lpVar = &m_apropStd[nApropIdx].varValue;
        
        if ((! bInitializing) && (nApropIdx < m_nStdApropCt)) 
            VariantClear(lpVar);
        else 
            VariantInit(lpVar);
        
        m_apropStd[nApropIdx].idsTypeInterp = IDS_VTOTHER;
        nApropIdx++;
    }

    nApropIdx = 0;
    while (nApropIdx < MAX_NONSTD_APROP_CT) 
    {
        lpVar = &m_apropNonstd[nApropIdx].varValue;
        if ((! bInitializing) && (nApropIdx < m_nNonstdApropCt)) 
            VariantClear (lpVar);
        else 
            VariantInit (lpVar);

        m_apropNonstd[nApropIdx].idsTypeInterp = IDS_VTOTHER;
        nApropIdx++;
    }
    m_nStdApropCt = 0;
    m_nNonstdApropCt = 0;    
}    

void CViewhtmlDoc::DestroyDlgAmbientProps(BOOL bInitializing)
{
    int nApropIdx = 0;
    VARIANT FAR * lpVar;
    while (nApropIdx < MAX_STD_APROP_CT) 
    {
        lpVar = &m_apropDlgStd[nApropIdx].varValue;
        if ((! bInitializing) && (nApropIdx < m_nDlgStdApropCt)) 
        {
            m_apropDlgStd[nApropIdx].strName = _T("");
            VariantClear (lpVar);
        } 
        else 
        {
            VariantInit (lpVar);
        }
        m_apropDlgStd[nApropIdx].idsTypeInterp = IDS_VTOTHER;
        nApropIdx++;
    }

    nApropIdx = 0;
    while (nApropIdx < MAX_NONSTD_APROP_CT) 
    {
        lpVar = &m_apropDlgNonstd[nApropIdx].varValue;
        if ((! bInitializing) && (nApropIdx < m_nDlgNonstdApropCt)) 
        {
            m_apropDlgNonstd[nApropIdx].strName = _T("");
            VariantClear (lpVar);
        } 
        else 
        {
            VariantInit (lpVar);
        }
        m_apropDlgNonstd[nApropIdx].idsTypeInterp = IDS_VTOTHER;
        nApropIdx++;
    }
    m_nDlgStdApropCt = 0;
    m_nDlgNonstdApropCt = 0;    
}

LPAPROP CViewhtmlDoc::FindAprop (DISPID dispid)
{
    LPAPROP papropRet = NULL;
    LPAPROP papropCand = m_apropNonstd;
    int nTryCt = m_nNonstdApropCt;
    while ((papropRet == NULL) && (nTryCt--)) 
    {
        if ((papropCand->dispid == dispid)
                            && (V_VT (&papropCand->varValue) != VT_EMPTY)) 
		{
            papropRet = papropCand;
        }
        papropCand++;
    }
    papropCand = m_apropStd;
    nTryCt = m_nStdApropCt;
    while ((papropRet == NULL) && (nTryCt--)) 
    {
        if ((papropCand->dispid == dispid)
                            && (V_VT (&papropCand->varValue) != VT_EMPTY)) 
		{
            papropRet = papropCand;
        }
        papropCand++;
    }
    return (papropRet);
}

LPAPROP CViewhtmlDoc::FindAprop (const TCHAR * pszName)
{
    CString strTarget = pszName;
    LPAPROP papropRet = NULL;
    LPAPROP papropCand = m_apropNonstd;
    int nTryCt = m_nNonstdApropCt;
    while ((papropRet == NULL) && (nTryCt--)) 
    {
        if ((papropCand->strName == strTarget) 
                            && (V_VT (&papropCand->varValue) != VT_EMPTY)) 
		{
            papropRet = papropCand;
        }
        papropCand++;
    }
    papropCand = m_apropStd;
    nTryCt = m_nStdApropCt;

    while ((papropRet == NULL) && (nTryCt--)) 
    {
        if ((papropCand->strName == strTarget) 
                            && (V_VT (&papropCand->varValue) != VT_EMPTY)) 
            papropRet = papropCand;
        papropCand++;
    }
    return (papropRet);
}    

HRESULT TFVarCopy (VARIANT * pvarDest, VARIANT * pvarSrc)
{
    HRESULT hr = NOERROR;
    VARTYPE vt = V_VT (pvarSrc);
    if(
        (
            (V_ISBYREF (pvarSrc))   || (V_ISARRAY (pvarSrc))
            ||
            (vt == VT_VARIANT)      || (vt == VT_DISPATCH)  || (vt == VT_UNKNOWN)
            || 
            (vt == VT_PTR)          || (vt == VT_SAFEARRAY) || (vt == VT_CARRAY)
            ||
            (vt == VT_BSTR)         || (vt == VT_LPSTR)     || (vt == VT_LPWSTR)
        )
        &&
        (V_I2REF (pvarSrc) == NULL)
    ) 
    {
        VariantClear (pvarDest);
        V_VT (pvarDest) = V_VT (pvarSrc);
        V_I2REF (pvarDest) = NULL;
    } 
    else 
    {
	    if ((vt == VT_BSTR) && (SysStringLen(V_BSTR(pvarSrc)) == 0))
	    {
	        VariantClear (pvarDest);
	        V_VT (pvarDest) = V_VT (pvarSrc);
	        V_BSTR (pvarDest) = SysAllocStringLen (L"", 32);
    	} 
    	else 
    	{
	        hr = VariantCopy (pvarDest, pvarSrc);
    	}
	}

    return (hr);
}

void CViewhtmlDoc::OnCloseDocument() 
{
	// TODO: Add your specialized code here and/or call the base class

	POSITION pos = GetStartPosition();
	
	while (pos != NULL)
	{
		CViewhtmlCntrItem* pItem = (CViewhtmlCntrItem*) GetNextItem(pos);

		UINT state = pItem->GetItemState();
		if (state == COleClientItem::activeState || 
			state == COleClientItem::activeUIState)
		{
			pItem->Close();
		}
	}
	
	COleDocument::OnCloseDocument();
}

void CViewhtmlDoc::OnLoadImages() 
	{
	// TODO: Add your command handler code here
	
	}

void CViewhtmlDoc::OnUpdateLoadImages(CCmdUI* pCmdUI) 
	{
	pCmdUI->Enable(FALSE);
	
	}

void CViewhtmlDoc::OnReload() 
	{
	CString url = m_current_mime_object->GetURL();

	SetPathName(url);

	m_current_mime_object = NULL;
	m_loading_mime_object = NULL;

	POSITION walk = GetFirstDynamicLoadPosition();
	while(walk)
		{
		CDynamicLoad *dlobject = GetNextDynamicLoad(walk);

		dlobject->AbortLoading();
		dlobject->ClearCacheEntry();
		}

	DeleteContents();
	theApp.OpenDocumentFile(url);
	}

void CViewhtmlDoc::OnUpdateReload(CCmdUI* pCmdUI) 
	{
	pCmdUI->Enable(m_current_mime_object != NULL );
	}

void CViewhtmlDoc::OnStopLoading() 
	{
	POSITION walk = GetFirstDynamicLoadPosition();
	while(walk)
		{
		CDynamicLoad *dlobject = GetNextDynamicLoad(walk);
		dlobject->AbortLoading();
		}
	}

void CViewhtmlDoc::OnUpdateStopLoading(CCmdUI* pCmdUI) 
	{
	pCmdUI->Enable( CDynamicLoad::IsAnyDynamicLoading() );
	}

void CViewhtmlDoc::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU) 
	{
	if (m_current_mime_object)
		{
		m_current_mime_object->DEBUG_LOCK();
		CString s = m_current_mime_object->GetTitle();
		m_current_mime_object->Unlock();
	
		m_strPathName = lpszPathName;

		if (s.IsEmpty())
			{
			SetTitle( lpszPathName );
			}
		else
			{
			SetTitle( s );
			}

		((CMainFrame *)(theApp.m_pMainWnd))->SetLocationText( lpszPathName );		
		}
	else
		{
		SetTitle("Untitled");
		((CMainFrame *)(theApp.m_pMainWnd))->SetLocationText( "" );		
		}
	}

BOOL CViewhtmlDoc::SaveModified() 
{
	// return TRUE so that the user is never asked to save a file
	
	return TRUE;
}

CPictureInfo& CViewhtmlDoc::GetPictureInfo()
	{
	return m_picture_info;
	}


BOOL CViewhtmlDoc::IsBetweenLoads()
	{
	if (m_loading_mime_object)
		{
		m_loading_mime_object->DEBUG_LOCK();
		CMimeObject * d = m_loading_mime_object->GetMimeObject();
		m_loading_mime_object->Unlock();
		return d != m_current_mime_object;
		}
	return FALSE;
	}

////////////////////////////////////////////////////////////

CPictureInfo::CPictureInfo( )
	{
	m_notify = NULL;
	}

CPictureInfo::~CPictureInfo()
	{
	m_picture_map.RemoveAll();
	}
			
void CPictureInfo::Reset( CNotifyObject *notify)
	{
	m_notify = notify;

	m_picture_map.RemoveAll();
	}

void CPictureInfo::UpdatePicture( LPCSTR url )
	{
	LPVOID ptr;

	if ( !m_picture_map.Lookup( url, ptr ) )
		{
		CMimeDynamicLoad *dlobject = new CMimeDynamicLoad( m_notify, url );
		dlobject->StartLoading();
		m_picture_map[ url ] =  dlobject;
		}
	}

BOOL CPictureInfo::GetPictureSize( LPCSTR url, CSize &size)
	{
	CMimeDynamicLoad *dyn_load;

	VERIFY( m_picture_map.Lookup(url, (void *&)dyn_load) );
	
	dyn_load->DEBUG_LOCK();
	CMimeObject *mime_object = dyn_load->GetMimeObject();
	
	if (!mime_object)
		{
		if (dyn_load->GetLoadState() == LOAD_STATE_ABORTED )
			{
			size.cx = 32;
			size.cy = 32;
			dyn_load->Unlock();
			return TRUE;
			}
		dyn_load->Unlock();
		return FALSE;		// don't know yet
		}
	else
		{
		if (mime_object->GetObjectType() != MIME_OBJECT_PICTURE)
			{
			size.cx = 32;
			size.cy = 32;
			dyn_load->Unlock();
			return TRUE;
			}
		CPicture *mime_picture = (CPicture *)mime_object;

		mime_picture->DEBUG_LOCK();

		if (mime_picture->IsHeaderRead() == FALSE)
			{
			if (dyn_load->GetLoadState() == LOAD_STATE_ABORTED )
				{
				size.cx = 32;
				size.cy = 32;
				dyn_load->Unlock();
				mime_picture->Unlock();
				return TRUE;
				}
			dyn_load->Unlock();
			mime_picture->Unlock();
			return FALSE;
			}
		size = mime_picture->GetSize();
		dyn_load->Unlock();
		mime_picture->Unlock();
		return TRUE;
		}
	}

BOOL CPictureInfo::GetPicture(LPCSTR url, CPicture * &picture)
	{
	BOOL rvalue;
	CMimeDynamicLoad *dyn_load = (CMimeDynamicLoad *)m_picture_map[ url ];
	dyn_load->DEBUG_LOCK();
	CMimeObject *mime_object = dyn_load->GetMimeObject();
	if (mime_object)
		mime_object->DEBUG_LOCK();

	if (!mime_object)
		{
		if (dyn_load->GetLoadState() == LOAD_STATE_ABORTED )
			{
			picture = NULL;
			rvalue = TRUE;
			goto exit;
			}
		rvalue = FALSE;
		goto exit;
		}
	else
		{
		if (mime_object->GetObjectType() != MIME_OBJECT_PICTURE)
			{
			picture = NULL;
			rvalue = TRUE;
			goto exit;
			}
		CPicture *mime_picture = (CPicture *)mime_object;

		if (mime_picture->IsHeaderRead() == FALSE)
			{
			if (dyn_load->GetLoadState() == LOAD_STATE_ABORTED )
				{
				picture = NULL;
				rvalue = TRUE;
				goto exit;
				}
			rvalue = FALSE;
			goto exit;
			}
		picture = mime_picture;
		rvalue = TRUE;
		}
exit:
	if (mime_object)
		mime_object->Unlock();
	dyn_load->Unlock();
	return rvalue;
	}
	
CString CPictureInfo::FindPicture( CMimeDynamicLoad *dlobject )
	{
	POSITION walk = m_picture_map.GetStartPosition();
	CString url;
	LPVOID ptr;
		
	while(walk)
		{
		m_picture_map.GetNextAssoc(walk, url, ptr );

		if ((CMimeDynamicLoad *)ptr == dlobject)
			return url;
		}
	return CString();
	}


void CViewhtmlDoc::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	pCmdUI->ContinueRouting();	
}

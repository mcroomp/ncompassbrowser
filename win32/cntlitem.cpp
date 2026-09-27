///////////////////////////////////////////////////////////////////////////
// CNTLITEM.CPP -- Implementation for the CControlItem class


#include "cross_p.h"

#include "cntlinfo.h"

#ifdef USES_OLE_CONTROLS
#include "cntlitem.h"
#endif

#include "viewhdoc.h"
#include "resource.h"

#ifndef _DDSKFILE_H_
#include "ddskfile.h"
#endif

#ifndef _VIEWHTML_H_
#include "viewhtml.h"
#endif

#include "oleimpl.h"

// Hard coded browser version
#define BROWSERVERSION	"0.9"

/////////////////////////////////////////////////////////////////////////////
// Definitions

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#ifdef _WIN32	
	static char szToolboxBitmap[] = "ToolboxBitmap32" ;
	static char szInprocServer[] = "InprocServer32" ;
	#define LOADLIBRARY_SUCCEEDED(x) (x != 0)
#else	
	static char szToolboxBitmap[] = "ToolboxBitmap" ;
	static char szInprocServer[] = "InprocServer" ;
	#define LOADLIBRARY_SUCCEEDED(x) (x > HINSTANCE_ERROR)
#endif	

#define new DEBUG_NEW

///////////////////////////////////////////////////////////////////////////
// struct NCompassBindInfo

NCompassBindInfo::~NCompassBindInfo()
{
    delete [] m_lpParamProps;
	delete [] m_lpFuncProps;
}

///////////////////////////////////////////////////////////////////////////
// class CControlItem

IMPLEMENT_DYNAMIC(CControlItem, COleClientItem)

///////////////////////////////////////////////////////////////////////////
// Constructors & Destructors

CControlItem::CControlItem(CRect rect, CString ocxURL, CViewhtmlDoc* pContainerDoc)
	: COleClientItem(pContainerDoc)
{
	m_pConnPt = NULL;
	m_pEventInfo = NULL;
	m_pCtlDispatch = NULL;
		
	// Setup Last Params Structure
	m_LastParams.cArgs = m_LastParams.cNamedArgs = 0;
	m_LastParams.rgvarg = NULL;
	m_LastParams.rgdispidNamedArgs = NULL;

	// Binding members
	m_pConnPtrBind = NULL;
	m_dwBindConnection = 0;
	m_nBinds = 0;
	m_pBindInfo = NULL;

	// Some OCX related initialization
	m_destination = disk;
	m_ocxURL = ocxURL;
	m_rect = rect;			// Location for the control.
	m_extent = CSize(0,0);	// Set the initial extent to zero.
	m_state = idle;
	m_broken = FALSE; 		// Initially assume everything is fine.
	m_storageOpen = FALSE;
}


CControlItem::~CControlItem()
{
	if (m_pConnPtrBind != NULL)
	{
		m_pConnPtrBind->Release();
		m_pConnPtrBind = NULL;
	}

	if (m_pBindInfo != NULL)
	{
		delete m_pBindInfo;
		m_pBindInfo = NULL;
	}

    if (m_pCtlDispatch != NULL)
    {
    	m_pCtlDispatch->Release();
		m_pCtlDispatch = NULL;
	}
	
	if (m_LastParams.cArgs > 0)
		CleanUpParams();
}

// More initialization code
void CControlItem::Initialize(CString ocxURL)
{
	m_ocxURL = ocxURL;
}

///////////////////////////////////////////////////////////////////////////
// Operations
                              
/////////////////////////////////////////////////////////////////////////////
// CControlItem diagnostics

#ifdef _DEBUG
void CControlItem::AssertValid() const
{
	COleClientItem::AssertValid();
}

void CControlItem::Dump(CDumpContext& dc) const
{
	COleClientItem::Dump(dc);
}
#endif

/////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
// Attributes
                              

/////////////////////////////////////////////////////////////////////////////
//  CControlItem interface map
 
 
BEGIN_INTERFACE_MAP(CControlItem, COleClientItem)
    INTERFACE_PART(CControlItem, IID_IDispatch, AmbientProps)
    INTERFACE_PART(CControlItem, IID_IPropertyNotifySink, PropertyNotifySink)
END_INTERFACE_MAP()


LPUNKNOWN CControlItem::GetInterfaceHook(const void FAR* iid)
{
    //
    //  If requested IID is same as the one we looked up in the registry
    //  (see InitControlInfo), return pointer to our event handler code.
    //
    
    if (*(IID FAR*)iid == m_iidEvents)
        return &m_xEventHandler;
    
    return NULL;
}
                                          
                                   
             
///////////////////////////////////////////////////////////////////////////
// Overrides
             
BOOL CControlItem::FinishCreate(HRESULT hr)
{
    BOOL bSuper = COleClientItem::FinishCreate(hr);

    if (bSuper)
    {
        m_pView = ((CFrameWnd *)(AfxGetApp()->m_pMainWnd))->GetActiveView();

		// Note we don't explicitly handle freezing events here, if we wished to,
		// we would grab the OLE object and inform it that its events are frozen.

        InitControlInfo();
		InitBindInfo();

		// Initialize our event controller
//		m_eventCtrl = new CEventController;

        //  Wire the control to our event handler
        LPCONNECTIONPOINTCONTAINER lpContainer;
        if (SUCCEEDED(m_lpObject->QueryInterface(
                                IID_IConnectionPointContainer,
                                (LPVOID FAR*)&lpContainer)))
        {
            ASSERT(lpContainer != NULL);
            if (SUCCEEDED(lpContainer->FindConnectionPoint(m_iidEvents, &m_pConnPt)))
            {
                ASSERT(m_pConnPt != NULL);
                m_pConnPt->Advise(&m_xEventHandler, &m_dwEventConnection);
            }

            if (SUCCEEDED(lpContainer->FindConnectionPoint(IID_IPropertyNotifySink, &m_pConnPtrBind)))
            {
                ASSERT(m_pConnPtrBind != NULL);
                if (m_pConnPtrBind != NULL) 
                {
                    m_pConnPtrBind->Advise(&m_xPropertyNotifySink, &m_dwBindConnection);
                }
            }

            lpContainer->Release();
        }
    }
                    
    return bSuper;
}

void CControlItem::Release(OLECLOSE dwCloseOption)
{
    UINT nSt = GetItemState ();
    if ((nSt == activeUIState) || (nSt == activeState) || (nSt == openState)) 
        Close (OLECLOSE_NOSAVE);

    if (m_pConnPt != NULL)
    {
        m_pConnPt->Unadvise(m_dwEventConnection);
        m_pConnPt->Release();
    }

    if (m_pConnPtrBind != NULL)
    {
        m_pConnPtrBind->Unadvise(m_dwBindConnection);
        m_pConnPtrBind->Release();
		m_pConnPtrBind = NULL;
    }

    FreeControlInfo();
    COleClientItem::Release(dwCloseOption);
}
             
             
                                   
///////////////////////////////////////////////////////////////////////////
// Implementation
        
LPUNKNOWN CControlItem::GetCtlInterface(IID iidRequested)
{
    LPUNKNOWN lpUnknown = NULL;
    if (m_lpObject != NULL) 
    { 
        IID iidUnknown = IID_IUnknown;
        if (IsEqualIID(iidRequested, iidUnknown)) 
        {
            lpUnknown = m_lpObject;
            lpUnknown->AddRef();
        } 
        else 
		{
        	if (FAILED (m_lpObject->QueryInterface(iidRequested, 
        		(LPVOID *) &lpUnknown))) 
        	{
	            lpUnknown = NULL;
    	    }
		}
    }
    return lpUnknown;
}
        
void CControlItem::InitEventInfo(LPTYPEINFO lpTypeInfo)
{
    LPTYPEATTR lpType = NULL;
    LPFUNCDESC lpFuncDesc = NULL;

    if (lpTypeInfo != NULL && SUCCEEDED(lpTypeInfo->GetTypeAttr(&lpType)))
    {
        m_iidEvents = lpType->guid;
        m_nEvents = lpType->cFuncs;

        if (m_nEvents > 0)
        {
			TRY
			{
            	m_pEventInfo = new EVENTINFO[m_nEvents];
                        
	            USHORT nCount;
                        
	            //
	            //  Enumerate events, getting their member id's and names
	            //
                
	            for (nCount = 0; nCount < m_nEvents; nCount++)
	            {
	                m_pEventInfo[nCount].memid = -1;
	                m_pEventInfo[nCount].cParams = 0;
	                m_pEventInfo[nCount].pbstr = NULL;
    
	                if (SUCCEEDED(lpTypeInfo->GetFuncDesc(nCount, &lpFuncDesc)))
	                {
	                    m_pEventInfo[nCount].memid = lpFuncDesc->memid;
	                    m_pEventInfo[nCount].cParams = lpFuncDesc->cParams;
    
	                    m_pEventInfo[nCount].pbstr = new BSTR[lpFuncDesc->cParams+1];
	                    UINT cNames;
	                    lpTypeInfo->GetNames(lpFuncDesc->memid, m_pEventInfo[nCount].pbstr, 
	                            lpFuncDesc->cParams+1, &cNames);
                                        
	                    ASSERT((unsigned)lpFuncDesc->cParams+1 == cNames);
	                    lpTypeInfo->ReleaseFuncDesc(lpFuncDesc);
						lpFuncDesc = NULL;
	                }
	            }
			}
			CATCH(CMemoryException, e)
			{
				if (lpType != NULL)
					lpTypeInfo->ReleaseTypeAttr(lpType);

				if (lpFuncDesc != NULL)
	                    lpTypeInfo->ReleaseFuncDesc(lpFuncDesc);
	                    					
				lpType = NULL;
				lpFuncDesc = NULL;
			}
			END_CATCH				
		}
        lpTypeInfo->ReleaseTypeAttr(lpType);
		lpType = NULL;
    }
}


void CControlItem::InitBindInfo()
{
	LPTYPEINFO lpTypeInfo = NULL;
	LPTYPEATTR lpType = NULL;
	LPVARDESC lpVarDesc = NULL;
	LPFUNCDESC lpFuncDesc = NULL;

	TRY
	{
        m_pBindInfo = new NCompassBindInfo;
        ASSERT(m_pBindInfo != NULL);
        m_pCtlDispatch = (LPDISPATCH) GetCtlInterface(IID_IDispatch);

        if (m_pCtlDispatch != NULL) 
        {
            UINT nTypeCount = 0;
            if ((SUCCEEDED(m_pCtlDispatch->GetTypeInfoCount(&nTypeCount))) && (nTypeCount == 1)) 
            {
                if (SUCCEEDED(m_pCtlDispatch->GetTypeInfo(0, GetUserDefaultLCID(), &lpTypeInfo))) 
         		{
                    if (SUCCEEDED(lpTypeInfo->GetTypeAttr(&lpType))) 
                    {
                        if ((m_pBindInfo->m_nParamCount = lpType->cVars) != 0) 
                        {
                            m_pBindInfo->m_lpParamProps = new PARAMPROPINFO [m_pBindInfo->m_nParamCount];
                            ASSERT(m_pBindInfo->m_lpParamProps != NULL);
                            UINT nDesc = 0;
                            UINT nParam = 0;
                            while (nParam < m_pBindInfo->m_nParamCount) 
                            {
                                lpTypeInfo->GetVarDesc(nDesc++, &lpVarDesc);
                                ASSERT(lpVarDesc != NULL);
                                if (lpVarDesc->varkind == VAR_DISPATCH) 
                                {
                                    LPPARAMPROPINFO lpParam = &m_pBindInfo->m_lpParamProps[nParam];
                                    lpParam->id = lpVarDesc->memid;
                                    BSTR bstr = NULL;
                                    UINT nNameCount;
                                    lpTypeInfo->GetNames(lpParam->id, &bstr, 1, &nNameCount);
                                    ASSERT (nNameCount == 1);
                                    if (nNameCount > 0) 
                                    {
									/*	char sz[80];
										
			                       		wcstombs(sz,
											(const unsigned short*) bstr, 80);
               						*/
										char sz[80];
										lstrcpy(sz, (const char*) bstr);
               							lpParam->strName = sz;
                                    }
                                    ::SysFreeString(bstr);
									bstr = NULL;
                                    nParam++;
                                } 
                                else 
                                {
                                    m_pBindInfo->m_nParamCount--;
                                }
                                lpTypeInfo->ReleaseVarDesc(lpVarDesc);
								lpVarDesc = NULL;
                            }
                        }

						// Do the methods of the object.
                        if ((m_pBindInfo->m_nFuncCount = lpType->cFuncs) != 0) 
                        {
                            m_pBindInfo->m_lpFuncProps = new PARAMPROPINFO [m_pBindInfo->m_nFuncCount];
                            ASSERT(m_pBindInfo->m_lpFuncProps != NULL);
                            UINT nDesc = 0;
                            UINT nParam = 0;
                            while (nParam < m_pBindInfo->m_nFuncCount) 
                            {
                                lpTypeInfo->GetFuncDesc(nDesc++, &lpFuncDesc);
                                ASSERT(lpFuncDesc != NULL);
                                if (lpFuncDesc->funckind == FUNC_DISPATCH) 
                                {
                                    LPPARAMPROPINFO lpParam = &m_pBindInfo->m_lpFuncProps[nParam];
                                    lpParam->id = lpFuncDesc->memid;
                                    BSTR bstr = NULL;
                                    UINT nNameCount;
                                    lpTypeInfo->GetNames(lpParam->id, &bstr, 1, &nNameCount);
                                    ASSERT (nNameCount == 1);
                                    if (nNameCount > 0) 
                                    {
									/*	char sz[80];
										
			                       		wcstombs(sz,
											(const unsigned short*) bstr, 80);
               						*/	

										char sz[80];
										lstrcpy(sz, (const char*) bstr);
               							lpParam->strName = sz;
                                    }
                                    ::SysFreeString(bstr);
									bstr = NULL;
                                    nParam++;
                                } 
                                else 
                                {
                                    m_pBindInfo->m_nParamCount--;
                                }
                                lpTypeInfo->ReleaseFuncDesc(lpFuncDesc);
								lpFuncDesc = NULL;
                            }
                        }

                        lpTypeInfo->ReleaseTypeAttr(lpType);
						lpType = NULL;
                    }
                    lpTypeInfo->Release();
					lpTypeInfo = NULL;
                }
				m_pCtlDispatch->Release();
				m_pCtlDispatch = NULL;
				ASSERT(m_pCtlDispatch == NULL);
				ASSERT(lpTypeInfo == NULL);
				ASSERT(lpType == NULL);
				ASSERT(lpFuncDesc == NULL);
				ASSERT(lpVarDesc == NULL);
            } 
            else 
            {
				TRACE(_T("Controls primary IDispatch could not be found!\n"));
				AfxThrowMemoryException(); // force cleanup
            }
        }
	}
	CATCH(CMemoryException, e)
	{
		if (m_pCtlDispatch != NULL)
		{
			m_pCtlDispatch->Release();
			m_pCtlDispatch = NULL;
		}

		if (lpTypeInfo != NULL)
		{
			if (lpType != NULL)
				lpTypeInfo->ReleaseTypeAttr(lpType);

			if (lpVarDesc != NULL)
				lpTypeInfo->ReleaseVarDesc(lpVarDesc);

			if (lpFuncDesc != NULL)
				lpTypeInfo->ReleaseFuncDesc(lpFuncDesc);

			lpTypeInfo->Release();
		}

		lpTypeInfo = NULL;
		lpType = NULL;
		lpVarDesc = NULL;
		lpFuncDesc = NULL;
	}
	END_CATCH
}

void CControlItem::InitControlInfo()
{
    //
    //  Use the control's class info to obtain information about
    //  its events, etc.
    //
    
    ASSERT_VALID(this);
    ASSERT(m_lpObject != NULL);
    
    LPPROVIDECLASSINFO lpProvide = NULL;
            
    if (SUCCEEDED(m_lpObject->QueryInterface(
                IID_IProvideClassInfo,
                (LPVOID FAR*)&lpProvide)))
    {
        ASSERT(lpProvide != NULL);
                
        LPTYPEINFO lpClassInfo = NULL;

        if (SUCCEEDED(lpProvide->GetClassInfo(&lpClassInfo)))
        {
            ASSERT(lpClassInfo != NULL);

            LPTYPEATTR lpType;
            if (SUCCEEDED(lpClassInfo->GetTypeAttr(&lpType)))
            {
                ASSERT(lpType != NULL);
                ASSERT(lpType->typekind == TKIND_COCLASS);
                
                UINT nCount;
                int iFlags;
                HREFTYPE hRefType;
                
                //
                //  Search for typeinfo of the default events interface.
                //
    
                for (nCount = 0; nCount < lpType->cImplTypes; nCount++)
                {
                    if (SUCCEEDED(lpClassInfo->GetImplTypeFlags(nCount, &iFlags)) &&
                        ((iFlags & IMPLTYPE_MASK) == IMPLTYPE_DEFAULTSOURCE))
                    {
                        LPTYPEINFO lpTypeInfo = NULL;
                        
                        if (SUCCEEDED(lpClassInfo->GetRefTypeOfImplType(nCount, &hRefType)) &&
                            SUCCEEDED(lpClassInfo->GetRefTypeInfo(hRefType, &lpTypeInfo)))
                        {
                            //
                            //  Found it!  Use it to initialize event table.
                            //

                            ASSERT(lpTypeInfo != NULL);
                            InitEventInfo(lpTypeInfo);                           
                            lpTypeInfo->Release();
							lpTypeInfo = NULL;
                        }
                        
                        break;
                    }
                }
            
                lpClassInfo->ReleaseTypeAttr(lpType);
				lpType = NULL;
            }
                            
            lpClassInfo->Release();
			lpClassInfo = NULL;
        }
            
        lpProvide->Release();
    }
}


void CControlItem::FreeControlInfo(void)
{
    if (m_pEventInfo != NULL)
    {
        USHORT i;
    
        for (i = 0; i < m_nEvents; i++)
            if (m_pEventInfo[i].pbstr != NULL)
            {
                SHORT j;
                for (j = 0; j < m_pEventInfo[i].cParams+1; j++)
	                ::SysFreeString(m_pEventInfo[i].pbstr[j]);
                delete [] m_pEventInfo[i].pbstr;
            }
        delete [] m_pEventInfo;
		m_pEventInfo = NULL;
    }                            
}

PARAMPROPINFO* CControlItem::GetParamPropInfo(DISPID id)
{
	USHORT i;

	for (i = 0; i < m_pBindInfo->m_nParamCount; i++)
		if (id == m_pBindInfo->m_lpParamProps[i].id)
			return &(m_pBindInfo->m_lpParamProps[i]);

	for (i = 0; i < m_pBindInfo->m_nFuncCount; i++)
		if (id == m_pBindInfo->m_lpFuncProps[i].id)
			return &(m_pBindInfo->m_lpFuncProps[i]);

	return NULL;
}

PARAMPROPINFO* CControlItem::GetParamPropInfo(CString propName)
{
 	USHORT i;

	for (i = 0; i < m_pBindInfo->m_nParamCount; i++)
		if (propName == m_pBindInfo->m_lpParamProps[i].strName)
			return &(m_pBindInfo->m_lpParamProps[i]);

	for (i = 0; i < m_pBindInfo->m_nFuncCount; i++)
		if (propName == m_pBindInfo->m_lpFuncProps[i].strName)
			return &(m_pBindInfo->m_lpFuncProps[i]);

	return NULL;
}
	
EVENTINFO* CControlItem::GetEventInfo(MEMBERID memid)
{
    USHORT i;

    for (i = 0; i < m_nEvents; i++)
        if (memid == m_pEventInfo[i].memid)
            return &m_pEventInfo[i];
        
    return NULL;
}
                                   
        
void CControlItem::CleanUpParams()
{              
	ASSERT_VALID(this);          
	
	DISPPARAMS FAR* lpDispparams = &m_LastParams;
	      
	if (lpDispparams->rgvarg != NULL)	      
	{
		for (UINT i=0; i<lpDispparams->cArgs; i++)
		{
			switch(lpDispparams->rgvarg[i].vt)
			{
				case VT_BSTR:
					::SysFreeString(lpDispparams->rgvarg[i].bstrVal);
					break;
				case VT_DISPATCH:
					lpDispparams->rgvarg[i].pdispVal->Release();
					break;
				case VT_UNKNOWN:
					lpDispparams->rgvarg[i].punkVal->Release();
					break;
				default:
					break;
			}
		}
    	delete lpDispparams->rgvarg;
    }

    lpDispparams->rgvarg = NULL;
    
    if (lpDispparams->rgdispidNamedArgs != NULL)
    	delete lpDispparams->rgdispidNamedArgs;
    lpDispparams->rgdispidNamedArgs = NULL;
}		
                                 
void CControlItem::CopyParams(DISPPARAMS FAR* lpDispparams)
{       
	UINT i;             

	if (lpDispparams == NULL)
		return;					// can't copy bogus params
		
	if (m_LastParams.cArgs > 0)
		CleanUpParams();

	// Change state information about incoming 		                  
	m_LastParams.cArgs = lpDispparams->cArgs;    
	if (lpDispparams->cArgs == 0)
		m_LastParams.rgvarg = NULL;
	else
		m_LastParams.rgvarg = new VARIANTARG[lpDispparams->cArgs];
    
	m_LastParams.cNamedArgs = lpDispparams->cNamedArgs;
	if(lpDispparams->cNamedArgs == 0)
  		m_LastParams.rgdispidNamedArgs = NULL;
	else
	{
  		m_LastParams.rgdispidNamedArgs = new DISPID[lpDispparams->cNamedArgs];
      		
		for(i = 0; i < m_LastParams.cNamedArgs; ++i)
			m_LastParams.rgdispidNamedArgs[i] = lpDispparams->rgdispidNamedArgs[i];
	}
	
	for (i = 0; i < m_LastParams.cArgs; i++)
	{               
		// Copy Parameter type
		m_LastParams.rgvarg[i].vt = lpDispparams->rgvarg[i].vt;

		switch(lpDispparams->rgvarg[i].vt)
        {       
	        case VT_I2:
	        	m_LastParams.rgvarg[i].iVal = lpDispparams->rgvarg[i].iVal;
	            break;
	 
	        case VT_I4:
	        	m_LastParams.rgvarg[i].lVal = lpDispparams->rgvarg[i].lVal;
	            break;
	 
	        case VT_R4:
	        	m_LastParams.rgvarg[i].fltVal = lpDispparams->rgvarg[i].fltVal;
	            break;
	 
	        case VT_R8:
	        	m_LastParams.rgvarg[i].dblVal = lpDispparams->rgvarg[i].dblVal;
	            break;

			case VT_BOOL:
	        	m_LastParams.rgvarg[i].boolVal = lpDispparams->rgvarg[i].boolVal;
	            break;
	                                                                      
	        case VT_ERROR:
	        	m_LastParams.rgvarg[i].scode = lpDispparams->rgvarg[i].scode;
	            break;
	                                                                        
	        case VT_CY:
	        	m_LastParams.rgvarg[i].cyVal = lpDispparams->rgvarg[i].cyVal;
	            break;

	        case VT_DATE:
	        	m_LastParams.rgvarg[i].date = lpDispparams->rgvarg[i].date;
	            break;
	            
	        case VT_BSTR:
	        	m_LastParams.rgvarg[i].bstrVal = ::SysAllocString(lpDispparams->rgvarg[i].bstrVal);
	            break;
                                                                             
	        case VT_UNKNOWN:
	        	m_LastParams.rgvarg[i].punkVal = lpDispparams->rgvarg[i].punkVal;
	            break;
	                                                                            
	        case VT_DISPATCH:
	        	m_LastParams.rgvarg[i].pdispVal = lpDispparams->rgvarg[i].pdispVal;
	            break;
	            
	        case (VT_I2 | VT_BYREF):
	        	m_LastParams.rgvarg[i].piVal = lpDispparams->rgvarg[i].piVal;
	            break;
	            
	        case (VT_I4 | VT_BYREF):
	        	m_LastParams.rgvarg[i].plVal = lpDispparams->rgvarg[i].plVal;
	            break;
	            
	        case (VT_R4 | VT_BYREF):
	        	m_LastParams.rgvarg[i].pfltVal = lpDispparams->rgvarg[i].pfltVal;
	            break;

	        case (VT_R8 | VT_BYREF):
	        	m_LastParams.rgvarg[i].pdblVal = lpDispparams->rgvarg[i].pdblVal;
	            break;

			case (VT_BOOL | VT_BYREF):
	        	m_LastParams.rgvarg[i].pboolVal = lpDispparams->rgvarg[i].pboolVal;
	            break;
	                                                                      
	        case (VT_ERROR | VT_BYREF):
	        	m_LastParams.rgvarg[i].pscode = lpDispparams->rgvarg[i].pscode;
	            break;
	                                                                        
	        case (VT_CY | VT_BYREF):
	        	m_LastParams.rgvarg[i].pcyVal = lpDispparams->rgvarg[i].pcyVal;
	            break;

	        case (VT_DATE | VT_BYREF):
	        	m_LastParams.rgvarg[i].pdate = lpDispparams->rgvarg[i].pdate;
	            break;
	            
	        case (VT_BSTR | VT_BYREF):
	        	m_LastParams.rgvarg[i].pbstrVal = lpDispparams->rgvarg[i].pbstrVal;
	            break;
                             
			case (VT_VARIANT | VT_BYREF):
				m_LastParams.rgvarg[i].pvarVal = lpDispparams->rgvarg[i].pvarVal;
				break;
				                                                                             
	        case (VT_UNKNOWN | VT_BYREF):
	        	m_LastParams.rgvarg[i].ppunkVal = lpDispparams->rgvarg[i].ppunkVal;
	            break;
	                                                                            
	        case (VT_DISPATCH | VT_BYREF):
	        	m_LastParams.rgvarg[i].ppdispVal = lpDispparams->rgvarg[i].ppdispVal;
	            break;
	            
	        default:
	        	break;
        }
	}
}
                                          
/////////////////////////////////////////////////////////////////////////////
//  Callbacks

void CControlItem::OnEvent(DISPID dispID, DISPPARAMS FAR* lpDispparams)
{
	// Just set the dispid and dispparams pointers to the latest information

	m_EventID = dispID;
	CopyParams(lpDispparams);

	// find the event

	EVENTINFO* pEvent = GetEventInfo(m_EventID);
	if (pEvent == NULL)
	{ 
  		TRACE1("Unknown event ID: %u\n", m_EventID);
		return;
	}

	// Our browser can understand the following events :
	CString	eventName = pEvent->pbstr[0];

	ParseEvent(eventName);

//	ASSERT(this->m_eventCtrl != NULL);
		
//	if (!this->m_eventCtrl->ParseEvent(eventName, m_LastParams))
//	{
		// We don't know how to handle your event, or 
		// something else went wrong
//	}				
}

void CControlItem::ParseEvent(CString eventName)
{
	if (eventName == "ReadFile")
	{
		ReadFile();
	}
}

BOOL CControlItem::ReadFile()
{
	if (m_LastParams.cArgs == 0)
	{
		// There is nothing to read..

		return FALSE;
	}	
	
 	if (m_LastParams.rgvarg[0].vt != VT_BSTR)
	{
		NotifyOCX(CString(), FALSE);
		return FALSE;
	}
	else
	{
		CString		url(m_LastParams.rgvarg[0].bstrVal);
		CString		fileName("");
		if (m_LastParams.cArgs > 1)
		{
			if (m_LastParams.rgvarg[1].vt == VT_BSTR)			
			{
				// We have filename 

				fileName = CString(m_LastParams.rgvarg[1].bstrVal);
			}	
		}

		// Don't do anything with that extra filename just an experiment...

		DownloadFile(url);
		return TRUE;
	}	
}

HRESULT CControlItem::OnPropertyNotification(DISPID dispID, UINT idsOccurence)
{
	CString strMsg;
	PARAMPROPINFO* pInfo = GetParamPropInfo(dispID);

	if (pInfo == NULL)
	{
		strMsg.Format(_T("%s: Unknown DISPID = %d"),
			(idsOccurence == IDS_BOUNDPROP_REQEDIT) ? _T("Edit Request") : _T("Change Notify"),
			dispID);
	}
	else
	{
		strMsg.Format(_T("%s: %s"),
			(idsOccurence == IDS_BOUNDPROP_REQEDIT) ? _T("Edit Request") : _T("Change Notify"),
			pInfo->strName);
	}

	TRACE(strMsg);
	TRACE(_T("\n"));
	m_pView->MessageBox(strMsg,
		_T("CControlItem::OnPropertyNotifcation()"), MB_OK);
	return NOERROR;
}

void CControlItem::OnNotify( UINT32 change_flags, CDynamicLoad *source)
{
	LOAD_STATE l;

	switch (m_state)
	{
		case ocxdownload :
				m_loader->DEBUG_LOCK();
				l = m_loader->GetLoadState();
				m_loader->Unlock();

				switch (l)
				{
					case LOAD_STATE_COMPLETE:
						
						m_state = idle;
						delete m_loader;
						m_loader = NULL;	
							
						// OK finished downloading, now proceed to registration
						if (RegisterOCX(FALSE, m_ocxFilename))
						{
							m_broken = FALSE;

							// Now create all the objects waiting on the list
							// to be created
							COleInfo*	oleInfo;

							if (m_pDoc->m_cntlItemList.Lookup(m_szObjCLSID,(CObject*&) oleInfo) )
							{
								oleInfo->m_objectState = COleInfo::oleLoaded;
								POSITION walk = oleInfo->m_objectList.GetHeadPosition();
								while (walk)
								{
									CControlItem* cntlItem = (CControlItem *)
														oleInfo->m_objectList.GetNext(walk);
									cntlItem->CheckAndInsertObject();						
								}
							}
						}
						else
						{
							// Now set all the objects waiting on the list
							// to be broken.
							SetObjectsBroken();
						}							
						break;

					case LOAD_STATE_ABORTED:

						// Something went wrong
						m_state = idle;
						delete m_loader;
						m_loader = NULL;
						
						// Now set all the objects waiting on the list
						// to be broken.
						SetObjectsBroken();
						break;
				}
				break;
		case filedownload :
				if (m_loader->GetLoadState() == LOAD_STATE_COMPLETE)
				{
					// OK finished downloading
					m_state = idle;
				}
				break;
	}
}

BOOL CControlItem::CheckIfValidURL(CString url, CString& fileName, CString& endURL,
										CString baseDirectory)
{
	// Check if we have a valid URL
	CString	tag = url.Left(5);

	// 1. Does url have "file:" or "http:" in the beginning ?
	if (tag == "file:" || tag == "http:")
	{	
		// YES,  it is a URL 
		fileName = ExtractFileNameFromURL(url);
//		fileName = ((CViewhtmlApp *) AfxGetApp())->m_ocxStorageDir + fileName; 
		fileName = baseDirectory + fileName; 
		endURL = url; 
		return TRUE;
	}
	if (url.Find(_T(":")) != -1)
	{
		// It might be 'driveletter:', 'ftp:' or 'gopher:'
		// We don't support those...
		
		return FALSE;
	}
	else
	{
		// We have a relative URL... 

	   	fileName = ExtractFileNameFromURL(url);
//		fileName = ((CViewhtmlApp *) AfxGetApp())->m_tempStorageDir + fileName;
		fileName = baseDirectory + fileName;
	   	endURL = CombineURL(LPCTSTR(m_pDoc->GetMimeObject()->GetURL()),LPCTSTR(url));
	   	return TRUE; 
	}
	return FALSE; 
}

void CControlItem::SetObjectsBroken()
{
	COleInfo*	oleInfo;

	if (m_pDoc->m_cntlItemList.Lookup(m_szObjCLSID, (CObject*&) oleInfo) )
	{
		oleInfo->m_objectState = COleInfo::oleLoaded;
		POSITION walk = oleInfo->m_objectList.GetHeadPosition();
		while (walk)
		{
			CControlItem* cntlItem = (CControlItem *)
								oleInfo->m_objectList.GetNext(walk);
			cntlItem->SetBroken();						
		}
	}
}	

// CControlItem, runtime OCX downloading and registration functions

void CControlItem::DownloadOCX()
{
	// OCXs are downloaded to the disk by default
	m_destination = disk;
	CString directory = ((CViewhtmlApp *) AfxGetApp())->m_ocxStorageDir;
	if (!CheckIfValidURL(m_ocxURL, m_ocxFilename, m_ocxURL, directory))
	{
		SetObjectsBroken(); 
		return;
	}

	// Check the OLE info to see if a previous OCX of same type is
	// already loaded.
	COleInfo*	oleInfo;
	if (m_pDoc->m_cntlItemList.Lookup(m_szObjCLSID, (CObject*&) oleInfo))
	{
		if (oleInfo->m_objectState == COleInfo::oleNotLoaded)
		{
			oleInfo->m_objectState = COleInfo::oleLoading;
			
			// We are about to load an OCX ask for user consent?
			CCntlDlg 	cntlDlg;

			cntlDlg.m_productName = m_ocxInfo.m_productName;
			cntlDlg.m_companyName = m_ocxInfo.m_companyName;
			cntlDlg.m_fileDescription = m_ocxInfo.m_fileDescription;
			cntlDlg.m_legalCopyright = m_ocxInfo.m_legalCopyright;
			cntlDlg.m_legalTrademarks = m_ocxInfo.m_legalTrademarks;
			cntlDlg.m_productVersion = m_ocxInfo.m_productVersion;
			cntlDlg.m_httpSite = m_ocxURL;

			if (cntlDlg.DoModal() != IDOK)
			{
				// Whoops the user does not want to download the OCX.
				SetObjectsBroken();
				return ;
			}

			AppendToLogFile();
			// Start loading the OCX
			m_state = ocxdownload;
			m_loader = new CDynLoadDiskFile( this, m_ocxURL, m_ocxFilename);
			m_loader->StartLoading();
		}
	}
}	

void CControlItem::AppendToLogFile()
{
	char 	logFilename[512];
	lstrcpy(logFilename, LPCTSTR(((CViewhtmlApp *) AfxGetApp())->m_tempStorageDir));
	lstrcat(logFilename, "ocxlog.log");

	CFile	file;
	OFSTRUCT OpenBuff ;
	if (::OpenFile(logFilename, &OpenBuff, OF_EXIST) == HFILE_ERROR)
	{
		// does not exist

		file.Open(logFilename, CFile::modeCreate | CFile::modeWrite );
	}
	else
	{
		// Already exists
		file.Open(logFilename, CFile::modeWrite); 
		file.SeekToEnd();
	}
	char	buffer1[512];
	sprintf(buffer1, "Product Name = %s \r\n", LPCTSTR(m_ocxInfo.m_productName));
	file.Write(buffer1, lstrlen(buffer1));
	sprintf(buffer1, "Company Name = %s \r\n", LPCTSTR(m_ocxInfo.m_companyName));
	file.Write(buffer1, lstrlen(buffer1));
	sprintf(buffer1, "File Description = %s \r\n", LPCTSTR(m_ocxInfo.m_fileDescription));
	file.Write(buffer1, lstrlen(buffer1));
	sprintf(buffer1, "Legal Copyright = %s \r\n", LPCTSTR(m_ocxInfo.m_legalCopyright));
	file.Write(buffer1, lstrlen(buffer1));
	sprintf(buffer1, "Legal Trademarks = %s \r\n", LPCTSTR(m_ocxInfo.m_legalTrademarks));
	file.Write(buffer1, lstrlen(buffer1));
	sprintf(buffer1, "Product Version = %s \r\n", LPCTSTR(m_ocxInfo.m_productVersion));
	file.Write(buffer1, lstrlen(buffer1));
	sprintf(buffer1, "HTTP Site = %s \r\n", LPCTSTR(m_ocxURL));
	file.Write(buffer1, lstrlen(buffer1));
	sprintf(buffer1, "----------------------------------------------------------\r\n");
	file.Write(buffer1, lstrlen(buffer1));

	file.Close();
}

BOOL CControlItem::DownloadFile(CString originalURL)
{
	// For one control item,we can have more than a single file loading.
	CString		fileName;
	CString		url;
	CString		directory = ((CViewhtmlApp *) AfxGetApp())->m_tempStorageDir;
	if (!CheckIfValidURL(originalURL, fileName, url, directory))
	{
		// Sorry we cannot load the file, invalid URL
		return FALSE;				
	}
		 
	m_fileNo++; // Increment the file load number, it also serves as a id number 

	// Do garbage collection for file loading items
//	FileLoaderGarbageCollect();

	// Add a new file loader to our list
	CFileLoader*	fileLoader = new CFileLoader;

	fileLoader->m_url = url;
	fileLoader->m_originalURL = originalURL;
	fileLoader->m_destination = CFileLoader::disk;
	fileLoader->m_callingOCX = this;
	fileLoader->m_diskFile = new CDynLoadDiskFile( fileLoader, url, fileName);
	fileLoader->m_diskFile->StartLoading();
	 
	fileLoader->m_loadingState = CFileLoader::loading;
	m_fileLoadingList.AddTail(fileLoader);
	return TRUE;							
}

void CControlItem::FileLoaderGarbageCollect()
{
	POSITION walk = m_fileLoadingList.GetHeadPosition();
	POSITION prevWalk;
	while (walk)
	{
		prevWalk = walk;
		CFileLoader*   fileLoader = (CFileLoader *)m_fileLoadingList.GetNext(walk);
		if (fileLoader->m_loadingState == CFileLoader::idle)
		{
			// delete this, doing nothing...

			m_fileLoadingList.RemoveAt(prevWalk);

			delete fileLoader->m_diskFile;
			delete fileLoader;
			m_fileNo--;
		}
	}
}

void CControlItem::ExitFileLoaders()
{
	POSITION walk = m_fileLoadingList.GetHeadPosition();
	POSITION prevWalk;
	while (walk)
	{
		prevWalk = walk;
		CFileLoader*   fileLoader = (CFileLoader *)m_fileLoadingList.GetNext(walk);
		if (fileLoader->m_loadingState == CFileLoader::idle ||
			fileLoader->m_loadingState == CFileLoader::loading)
		{
			// Stop loading files regardless of whether they are loaded or not.

			m_fileLoadingList.RemoveAt(prevWalk);
			delete fileLoader->m_diskFile;	
			delete fileLoader;
			m_fileNo--;
		}
	}
}

BOOL CControlItem::NotifyOCX(CString url, BOOL isLoaded)
{
	// Notify the OCX

	PARAMPROPINFO* 	paramPropInfo = GetParamPropInfo(CString("OnReadFileNotify"));
	if (paramPropInfo == NULL)
	{
		// Sorry but the OCX does not provide any OnNotify 
		// function, we can't help it.
		return FALSE;
	}
	
	COleDispatchDriver	pCtlDriver;
    m_pCtlDispatch = (LPDISPATCH) GetCtlInterface(IID_IDispatch);
	ASSERT(m_pCtlDispatch != NULL);
	pCtlDriver.AttachDispatch(m_pCtlDispatch);

  	char	targetURL[100];
	lstrcpy(targetURL, LPCTSTR(url));
	char 	directory[100];
	lstrcpy(directory, LPCTSTR(((CViewhtmlApp *) AfxGetApp())->m_tempStorageDir));

	static BYTE BASED_CODE parms[] = VTS_BSTR VTS_BSTR VTS_BOOL;
	pCtlDriver.InvokeHelper(paramPropInfo->id, 
							DISPATCH_METHOD, VT_EMPTY, NULL,
							parms, 
							directory,
							targetURL, isLoaded);
	
	pCtlDriver.ReleaseDispatch();
//	m_pCtlDispatch->Release();
	m_pCtlDispatch = NULL;
	FileLoaderGarbageCollect();	
	return TRUE;						 	
}


BOOL CControlItem::RegisterOCX(BOOL fUnreg, CString ocxPath)
{
	HINSTANCE hMod;

	//Do this for the sake of DLLs.
	if (FAILED(CoInitialize(NULL)))
	    return 0;

	hMod = LoadLibraryA(LPCTSTR(ocxPath));
	BOOL fRes = FALSE;

	if (hMod > (HINSTANCE)HINSTANCE_ERROR)
	{
	    HRESULT (STDAPICALLTYPE *pfn)(void);

	    if (fUnreg)
	    {
	        (FARPROC&) pfn = GetProcAddress(hMod
	            , "DllUnregisterServer");

	        if (NULL != pfn)
	            fRes=SUCCEEDED((*pfn)());

			CoFreeLibrary(hMod);
//	        MessageBoxA(NULL, fRes
//	            ? "DLL unregistration succeeded."
//	            : "DLL unregistration failed.", "SelfReg", MB_OK);
	    }
	    else
	    {
	        (FARPROC&)pfn = GetProcAddress(hMod
	            , "DllRegisterServer");

	        if (NULL != pfn)
	            fRes = SUCCEEDED((*pfn)());

//	        MessageBoxA(NULL , fRes
//	            ? "DLL registration succeeded."
//	            : "DLL registration failed.", "SelfReg", MB_OK);
	    }

	    //CoFreeLibrary(hMod);
	    CoUninitialize();
	}

	return fRes;
}

// Insert new OCXs

void CControlItem::CheckAndInsertObject()
{
	// First check the registration database for object

	if (ReportIfLocalOCX())
	{
		BeginWaitCursor();

		// Create the OCX

		if (m_storageOpen)
		{
			// We have an open storage so read object from there
			CreateItemFromStorage();
		}
		else
		{	
			// We have a new object
			CreateNewItem(m_objCLSID); 
		}
		UpdateLink();
		SetExtent(m_rect.Size());
		UpdateExtent();
		DoVerb(OLEIVERB_SHOW, m_pView);

		CViewhtmlDoc* pDoc = (CViewhtmlDoc* ) GetDocument();
		ASSERT_VALID(pDoc);

		pDoc->UpdateAllViews(NULL);

		EndWaitCursor();
	}
	else 
	{
		// Not a local OCX we should load from the OCXURL
		// Check the URL
		if (m_ocxURL.IsEmpty())
		{
			// We don't know where our OCX is ??

			SetBroken();
			return;
		}

		// Prepare to load our OCX
		
		DownloadOCX();				
	}			
}

BOOL CControlItem::UpdateExtent()
{
	CSize size;
	if (GetExtent(&size))
	{
		// OLE returns the extent in HIMETRIC units -- we need pixels
		CClientDC dc(NULL);
		dc.HIMETRICtoDP(&size);

		// only invalidate if it has actually changed
		if (size != m_rect.Size())
		{
			// invalidate old, update, invalidate new
			//Invalidate();
			m_rect.bottom = m_rect.top + size.cy;
			m_rect.right = m_rect.left + size.cx;
			//Invalidate();

			// mark document as modified
			//GetDocument()->SetModifiedFlag();
		}         
		return TRUE;
	}               
	else
		return FALSE;
}

// CControlItem, serialize from memory file instead of disk file.

BOOL CControlItem::GetCLSID(CFormatOLEControlItem* item)
{
	// Find CLSID from HTML src

	CLSID	nilCLSID;

	memset(&nilCLSID, 0, sizeof(CLSID));

	// First attempt to get the CLSID from parser information.
	item->GetClassID(m_objCLSID);
	if (IsEqualCLSID(m_objCLSID, nilCLSID))
	{
		// the CLSID field not found in HTML, illegal CLSID
		// Check the storage of object if it exists
		if (item->GetMemFile() != NULL)
		{	
			if (m_storageOpen)
			{
				// Storage should have been open before
				if (!ReadCLSIDFromStorage())
				{
					return FALSE;
				}
			}	
		}
		else
		{
			// CLSID does not exist at all 
			return FALSE;
		}
	}

	// If we are here, we should have a m_objCLSID
	LPOLESTR clsidString = NULL;
	if (StringFromCLSID(m_objCLSID, &clsidString) == S_OK)
	{
		m_szObjCLSID = clsidString;
		CoTaskMemFree(clsidString);
		COleInfo*	objEntry; 
		if (m_pDoc->m_cntlItemList.Lookup(m_szObjCLSID, (CObject* &) objEntry))
		{
			ASSERT(objEntry != NULL);

			// We have a previous entry so add this one to 
			// the list in that entry.
			objEntry->m_objectList.AddTail(this);
		}
		else
		{
			// We should create a new entry, a different
			// CLSID found.
			COleInfo* newEntry = new COleInfo;
			newEntry->m_objectState = COleInfo::oleNotLoaded;
			newEntry->m_objectList.AddTail(this);
			m_pDoc->m_cntlItemList.SetAt(m_szObjCLSID, newEntry);
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CControlItem::ReportIfLocalOCX()
{
	CString	ocxPath;
	BOOL 	isLocalOCX = CheckIfLocalOCX(ocxPath);

	if (isLocalOCX)
	{
		// Cool!, everything is in the system...

		return TRUE;
	}
	else
	{

		ASSERT(m_reasonRegistry != startSearching);
		ASSERT(m_reasonRegistry != correctVersion);
		switch (m_reasonRegistry)
		{
			case	clsidNotFound :
					
					// First time we are registering so go ahead with registration
					break;
			case 	incorrectSystem :
					
					// We should never get this (16 and 32 bit OCXs shouldn't have
					// the same clsid
					ASSERT(FALSE);
					break;
			case 	fileNotExist :
					
					// Cleanup from the registry. OCX doesnot exist so has to be
					// cleaned up from the registry manually.
					ManualOCXCleanup(ocxPath);
					break; 
			case 	incorrectVersion :
					
					// Unregister the old version of OCX before proceeding...
					BOOL isUnregistered = RegisterOCX(TRUE, ocxPath);
					if (!isUnregistered)
					{
						ManualOCXCleanup(ocxPath);
					}
					break;
		}
		return FALSE;
	}
}

void CControlItem::ManualOCXCleanup(CString ocxPath)
{
	HKEY 		hKeyClsid ;
	HKEY 		hKeyX ;
	HKEY 		hKeyTypeLib;
	char 		szClsidName[MAX_PATH+1] ;
	char 		szBuffer[MAX_PATH*2] ;
	LONG 		lSize ;
	CPtrList 	PathList ;
	int 		iNonExisting = 0 ;

	VERIFY( AfxCheckMemory() );
	// Open the "CLSID" key
	LONG regResult = ::RegOpenKey(HKEY_CLASSES_ROOT, "CLSID", &hKeyClsid);
	if (regResult != ERROR_SUCCESS) 
		return ;

	BeginWaitCursor() ;

    regResult = ::RegOpenKey(HKEY_CLASSES_ROOT, "TypeLib", &hKeyTypeLib) ;
	if (regResult != ERROR_SUCCESS) 
		hKeyTypeLib = NULL ;
	
	// Remove ProgID
	if (m_pRegEntry.m_pProgID) 
		WipeOut(HKEY_CLASSES_ROOT, m_pRegEntry.m_pProgID) ;
	
	// Remove TypeLib
	if (m_pRegEntry.m_pTypeLib && hKeyTypeLib) 
		WipeOut(hKeyTypeLib, m_pRegEntry.m_pTypeLib) ;
					 	
	// Remove current entry
	WipeOut(hKeyClsid, m_pRegEntry.m_pClsid ) ;

	::RegCloseKey(hKeyTypeLib) ;

	//
	// Look for anything under \CLSID that has the same InprocServer 
	//
	
	// Enum all entries under CLSID
	DWORD dwIndex = 0 ;
	while(::RegEnumKey(hKeyClsid, dwIndex++, szClsidName, sizeof(szClsidName)) == ERROR_SUCCESS)
	{
		// Open the CLSID key
		regResult = ::RegOpenKey(hKeyClsid, szClsidName, &hKeyX) ;
		if (regResult != ERROR_SUCCESS) 
			continue;

		// Get the InprocServer path ;
		lSize = sizeof(szBuffer) ;
		regResult = ::RegQueryValue(hKeyX, szInprocServer, szBuffer, &lSize) ;
		::RegCloseKey(hKeyX) ;
		if (regResult == ERROR_SUCCESS)
		{
			if (ocxPath == (LPTSTR)szBuffer)
			{
				WipeOut(hKeyClsid, szClsidName) ;				
				dwIndex-- ;
			}
		}
	}
	::RegCloseKey(hKeyClsid) ;
	VERIFY( AfxCheckMemory() );

	EndWaitCursor() ;	
}

//
// Wipe out lpszSubKey and below
//
BOOL CControlItem::WipeOut(HKEY hKey, LPCTSTR lpszSubKey)
{
	HKEY hSubKey ;
	char szSubSubName[MAX_PATH+1] ;

	VERIFY( AfxCheckMemory() );
	LONG regResult = ::RegOpenKey(hKey, lpszSubKey, &hSubKey);
	if (regResult != ERROR_SUCCESS) return FALSE ;
		
	//DWORD dwIndex = 0 ;
	while(::RegEnumKey(hSubKey, 0/*dwIndex++*/, szSubSubName, sizeof(szSubSubName)) == ERROR_SUCCESS)
	{
		WipeOut(hSubKey, szSubSubName) ;
	}		

	::RegCloseKey(hSubKey) ;
	regResult = ::RegDeleteKey(hKey, lpszSubKey) ;
	TRACE("WipeOut: %s, %d\r\n", lpszSubKey, regResult) ;
	VERIFY( AfxCheckMemory() );
	
	return (regResult == ERROR_SUCCESS) ;
}


BOOL CControlItem::CheckIfLocalOCX(CString& ocxPath)
{
	DWORD dwIndex = 0 ;
	HKEY hKeyClsid ;
	HKEY hKeyX ;
	HKEY hKeyControl ;
	HKEY hKeyInsertable; 
	LONG lSize ;
	LONG regResult = ERROR_SUCCESS;
	char szClsidName[MAX_PATH+1] ;
	char szBuffer[MAX_PATH*2] ;
	

	//HKEY_CLASSES_ROOT
	//	CLSID
	//		{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}	
	//			Control
	//			Insertable
	//			ToolbarBitmap		
	regResult = ::RegOpenKey(HKEY_CLASSES_ROOT, "CLSID", &hKeyClsid);
	if (regResult != ERROR_SUCCESS) return FALSE ;

	BeginWaitCursor() ;

	VERIFY( AfxCheckMemory() );
	// Enum all entries under CLSID
	BOOL 	ocxFound = FALSE;
	m_reasonRegistry = startSearching;
	while(::RegEnumKey(hKeyClsid, dwIndex++, szClsidName, MAX_PATH+1) == ERROR_SUCCESS)
	{
		// Check if it is what we are looking for?
		CLSID	lookupId;
		CLSIDFromString((LPOLESTR) szClsidName, (LPCLSID) &lookupId);
		if (!IsEqualCLSID(m_objCLSID, lookupId)) 
		{
			if ( m_reasonRegistry != incorrectSystem )
				m_reasonRegistry = clsidNotFound;
			continue;
		}
	
		// Open the CLSID key
		regResult = ::RegOpenKey(hKeyClsid,szClsidName,&hKeyX) ;
		if (regResult != ERROR_SUCCESS)
		{
			if ( m_reasonRegistry != incorrectSystem )
				m_reasonRegistry = clsidNotFound;
		 	continue;
		}

		// Is this a Control?
		regResult = ::RegOpenKey(hKeyX, "Control", &hKeyControl) ;	
		if (regResult == ERROR_SUCCESS)
		{
			// Yes, we have a control 

			// Look for path of Inproc Server 
			lSize = sizeof(szBuffer) ;
			regResult = ::RegQueryValue(hKeyX, szInprocServer, szBuffer, &lSize) ;
			if (regResult != ERROR_SUCCESS)
			{
				// Didn't find it. It might be 16bit instead of 32bit or vice versa
			    m_reasonRegistry = incorrectSystem;
			    continue ; 
			}
			
			// Store the ocxName
//			LPTSTR 	ocxName = new char[lSize];
//			memcpy(ocxName, szBuffer, (int)lSize);
			ocxPath = (LPTSTR) szBuffer;

			// Check to see .OCX file actually exits
			OFSTRUCT OpenBuff ;
			if (::OpenFile(ocxPath, &OpenBuff, OF_EXIST) == HFILE_ERROR)
			{
				// Get necessary information for cleanup!

				VERIFY( AfxCheckMemory() );
				// Store CLSID for Cleanup
				m_pRegEntry.m_pClsid = new char[strlen(szClsidName) + 1] ;
				strcpy(m_pRegEntry.m_pClsid, szClsidName) ;
				VERIFY( AfxCheckMemory() );

				// Get ProdID and store it for later Cleanup
				lSize = sizeof(szBuffer); 
				regResult = ::RegQueryValue(hKeyX, "ProgID", szBuffer, &lSize) ;
				if (regResult == ERROR_SUCCESS)
				{
					m_pRegEntry.m_pProgID = new char[lSize] ;
					memcpy(m_pRegEntry.m_pProgID, szBuffer, (int)lSize) ;
				}
				
				VERIFY( AfxCheckMemory() );
				// Get TypeLib ID and store it for later Cleanup
				lSize = sizeof(szBuffer); 
				regResult = ::RegQueryValue(hKeyX, "TypeLib", szBuffer, &lSize) ;
				if (regResult == ERROR_SUCCESS)
				{
					m_pRegEntry.m_pTypeLib = new char[lSize] ;
					memcpy(m_pRegEntry.m_pTypeLib, szBuffer, (int)lSize) ;
				}

				VERIFY( AfxCheckMemory() );
				m_reasonRegistry = fileNotExist;
/*				if (ocxName != NULL)
				{
					delete [] ocxName;
					ocxName = NULL;
				} */
				::RegCloseKey(hKeyControl);
				::RegCloseKey(hKeyX);
				break;
			}	
					
			// Check the version number
			LPTSTR fileName = ocxPath.GetBuffer(ocxPath.GetLength());
			CString currentVersion = GetVersionInfo(fileName);
			ocxPath.ReleaseBuffer();
			
			// m_ocxInfo.m_fileVersion holds the version number of the file
			// we are about to load compare with our version!
			if (currentVersion == m_ocxInfo.m_productVersion)
			{
				// Correct version
				
				ocxFound = TRUE;
				m_reasonRegistry = correctVersion;
				::RegCloseKey(hKeyControl) ;
				::RegCloseKey(hKeyX) ; 
/*				if (ocxName != NULL)
				{
					delete [] ocxName;
					ocxName = NULL;
		  		}*/
				break;					
			}

			// If we are here we sure have an incorrect version!!

/*			if (ocxName != NULL)
			{
				delete [] ocxName;
				ocxName = NULL;
	  		}*/
			m_reasonRegistry = incorrectVersion;							
			
			// Cleanup	
			::RegCloseKey(hKeyControl) ;
			::RegCloseKey(hKeyX);
			break;
		}
		else
		{
			m_reasonRegistry = clsidNotFound;
		}
		::RegCloseKey(hKeyX) ; 
	}
	::RegCloseKey(hKeyClsid); 
	VERIFY( AfxCheckMemory() );

	EndWaitCursor() ;
	return ocxFound;
}

CString CControlItem::GetVersionInfo(LPTSTR filename)
{
	DWORD 	handle;
	CString	versionNo;
	UINT 	uiVerSize;
	UINT 	uiSize;
	DWORD* 	lpBuffer;

	UINT uiInfoSize = ::GetFileVersionInfoSize(filename, &handle);
	if (uiInfoSize == 0)
	{
		return versionNo;
	}

	BYTE*	pbData = NULL;
	pbData = new BYTE[uiInfoSize];

	BOOL bResult = ::GetFileVersionInfo(filename, handle,
						uiInfoSize, pbData);
	if (!bResult)
	{
		delete [] pbData;
		return versionNo;
	}

	bResult = ::VerQueryValue(pbData, "\\VarFileInfo\\Translation",
							(void**)&lpBuffer,
							&uiVerSize);

 	if (!bResult)
	{
		delete [] pbData;
		return versionNo;
	}

	bResult = uiVerSize;
	if (!bResult)
	{
		delete [] pbData;
		return versionNo;
	}

	char	szName[512];
	sprintf(szName, "\\StringFileInfo\\%04hX%04hX\\ProductVersion",
			LOWORD(*lpBuffer),
			HIWORD(*lpBuffer));

	bResult = ::VerQueryValue(pbData, 
					szName,
					(void**)&lpBuffer,
					&uiSize);
	versionNo = (LPCTSTR) lpBuffer;
	delete [] pbData;
	return versionNo;
}

// Overrides

void CControlItem::SetExtent(CSize  size)
{
	// Convert from DP to HIMETRIC

	CClientDC dc(NULL);
	dc.DPtoHIMETRIC(&size);

	COleClientItem::SetExtent(size);
}

BOOL CControlItem::CreateItemFromStorage()
{
	ASSERT_VALID(this);
	ASSERT(m_lpStorage != NULL);
	ASSERT(m_lpLockBytes != NULL);
	ASSERT(m_storageOpen);

	// attempt to load the object from the storage
	LPUNKNOWN lpUnk = NULL;
	SCODE sc = ::OleLoad(m_lpStorage, IID_IUnknown, GetClientSite(),
		(LPLP)&lpUnk);
	CheckGeneral(sc);

	ASSERT(lpUnk != NULL);
	m_lpObject = WRAPINTERFACE(lpUnk, IOleObject);
	lpUnk->Release();
	if (m_lpObject == NULL)
		AfxThrowOleException(E_OUTOFMEMORY);

	BOOL bResult = FinishCreate(sc);
	ASSERT_VALID(this);

	return bResult;
}

BOOL	CControlItem::ReadCLSIDFromStorage()
{
	ASSERT(m_storageOpen);
	ASSERT(m_lpStorage != NULL);

	SCODE sc = ::ReadClassStg(m_lpStorage, &m_objCLSID);
	if (sc != S_OK)
	{
		// Storage is not a valid OCX storage
		
		return FALSE;
	}
	return TRUE; 				
}

int	CControlItem::OpenStorage(CFormatOLEControlItem* item)
{
	ASSERT_VALID(this);
	
	CMemFile*	inputFile = (CMemFile *) item->GetMemFile()->GetFile(); 

	// Go to the beginning of file
	inputFile->Seek(0, CFile::begin);
	CArchive	ar(inputFile, CArchive::load);

	m_storageOpen = FALSE;

	// Now read the OCX info
	try 
	{
		ar >> m_ocxInfo.m_browserVersion;
		if (m_ocxInfo.m_browserVersion != BROWSERVERSION)
			AfxThrowArchiveException(CArchiveException::endOfFile);
		ar >> m_ocxInfo.m_companyName;
		ar >> m_ocxInfo.m_fileDescription;
		ar >> m_ocxInfo.m_fileVersion;
		ar >> m_ocxInfo.m_internalName;
		ar >> m_ocxInfo.m_legalCopyright;
		ar >> m_ocxInfo.m_legalTrademarks;
		ar >> m_ocxInfo.m_originalFilename;
		ar >> m_ocxInfo.m_productName;
		ar >> m_ocxInfo.m_productVersion;
	}
	catch (CException* e)
	{
		// Whoops we don't have the data here
		item->GetMemFile()->ReleaseFile();
		e->Delete();
		return -1;
	}

	// read number of bytes in the ILockBytes
	DWORD dwBytes;
	ar >> dwBytes;

	if (dwBytes <= 0)
	{
		// This is probably not a correct file

		item->GetMemFile()->ReleaseFile();
		return -1;
	}
	// allocate enough memory to read entire block
	HGLOBAL hStorage = ::GlobalAlloc(GMEM_SHARE|GMEM_MOVEABLE, dwBytes);
	if (hStorage == NULL)
		AfxThrowMemoryException();

	LPVOID lpBuf = ::GlobalLock(hStorage);
	ASSERT(lpBuf != NULL);
	DWORD dwBytesRead = ar.Read(lpBuf, dwBytes);
	::GlobalUnlock(hStorage);

	// Now read the OCX info
	try 
	{
		// throw exception in case of partial object
		if (dwBytesRead != dwBytes)
		{
			AfxThrowArchiveException(CArchiveException::endOfFile);
		}
	}
	catch (CException* e)
	{
		// Whoops we don't have the data here
		::GlobalFree(hStorage);
		item->GetMemFile()->ReleaseFile();
		e->Delete();
		return -1;
	}
							
			
	// Ok we are finished with archive ar
	item->GetMemFile()->ReleaseFile();

	// throw exception in case of partial object
//	if (dwBytesRead != dwBytes)
//	{
//		::GlobalFree(hStorage);
//		AfxThrowArchiveException(CArchiveException::endOfFile);
//	}

	SCODE sc = CreateILockBytesOnHGlobal(hStorage, TRUE, &m_lpLockBytes);
	if (sc != NOERROR)
	{
		::GlobalFree(hStorage);
		AfxThrowOleException(sc);
	}
	ASSERT(m_lpLockBytes != NULL);
	ASSERT(::StgIsStorageILockBytes(m_lpLockBytes) == NOERROR);

	sc = ::StgOpenStorageOnILockBytes(m_lpLockBytes, NULL,
		STGM_SHARE_EXCLUSIVE|STGM_READWRITE, NULL, 0, &m_lpStorage);
	if (sc != NOERROR)
	{
		VERIFY(m_lpLockBytes->Release() == 0);
		m_lpLockBytes = NULL;
			// ILockBytes::Release will GlobalFree the hStorage
		AfxThrowOleException(sc);
	}	
	m_storageOpen = TRUE;
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
//  CControlItem::XEventHandler
 
 
STDMETHODIMP_(ULONG) CControlItem::XEventHandler::AddRef()
{
    METHOD_PROLOGUE(CControlItem, EventHandler)
    return (ULONG)pThis->ExternalAddRef();
}
 
 
STDMETHODIMP_(ULONG) CControlItem::XEventHandler::Release()
{
    METHOD_PROLOGUE(CControlItem, EventHandler)
    return (ULONG)pThis->ExternalRelease();
}
 
 
STDMETHODIMP CControlItem::XEventHandler::QueryInterface(
    REFIID iid, LPVOID far* ppvObj)
{
    METHOD_PROLOGUE(CControlItem, EventHandler)
    return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}
 
 
STDMETHODIMP CControlItem::XEventHandler::GetTypeInfoCount(unsigned int FAR* pctinfo)
{
    METHOD_PROLOGUE(CControlItem, EventHandler)
    ASSERT_VALID(pThis);

    *pctinfo = 0;
    return NOERROR;
}
 
 
STDMETHODIMP CControlItem::XEventHandler::GetTypeInfo(unsigned int itinfo,
      LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
    METHOD_PROLOGUE(CControlItem, EventHandler)
    ASSERT_VALID(pThis);
 
    return ResultFromScode(E_NOTIMPL);
}
 
 
STDMETHODIMP CControlItem::XEventHandler::GetIDsOfNames(REFIID riid,
      LPOLESTR FAR* rgszNames, unsigned int cNames, LCID lcid,
      DISPID FAR* rgdispid)
{
    METHOD_PROLOGUE(CControlItem, EventHandler)
    ASSERT_VALID(pThis);
 
    return ResultFromScode(E_NOTIMPL);
}

 
STDMETHODIMP CControlItem::XEventHandler::Invoke(DISPID dispidMember,
      REFIID riid, LCID lcid, unsigned short wFlags, DISPPARAMS FAR* lpDispparams,
      VARIANT FAR* pvarResult, EXCEPINFO FAR* pexcepinfo, 
      unsigned int FAR* puArgErr)
{
    METHOD_PROLOGUE(CControlItem, EventHandler)
    ASSERT_VALID(pThis);

	pThis->OnEvent(dispidMember, lpDispparams);	// Call the event handler, who also cleans up.
	
	return NOERROR;
}
                                          

/////////////////////////////////////////////////////////////////////////////
//  CControlItem::XPropertyNotifySink

STDMETHODIMP_(ULONG) CControlItem::XPropertyNotifySink::AddRef()
{
    METHOD_PROLOGUE(CControlItem, PropertyNotifySink)
    return (ULONG)pThis->ExternalAddRef();
}
 
 
STDMETHODIMP_(ULONG) CControlItem::XPropertyNotifySink::Release()
{
    METHOD_PROLOGUE(CControlItem, PropertyNotifySink)
    return (ULONG)pThis->ExternalRelease();
}
 
 
STDMETHODIMP CControlItem::XPropertyNotifySink::QueryInterface(
    REFIID iid, LPVOID far* ppvObj)
{
    METHOD_PROLOGUE(CControlItem, PropertyNotifySink)
    return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP CControlItem::XPropertyNotifySink::OnChanged(
      DISPID dispid)
{
    METHOD_PROLOGUE(CControlItem, PropertyNotifySink)
    return (pThis->OnPropertyNotification(dispid, IDS_BOUNDPROP_CHANGED));
}


STDMETHODIMP CControlItem::XPropertyNotifySink::OnRequestEdit(
      DISPID dispid)
{
    METHOD_PROLOGUE(CControlItem, PropertyNotifySink)
    return (pThis->OnPropertyNotification(dispid, IDS_BOUNDPROP_REQEDIT));
}


/////////////////////////////////////////////////////////////////////////////
//  CControlItem::XAmbientProps
 
 
STDMETHODIMP_(ULONG) CControlItem::XAmbientProps::AddRef()
{
    METHOD_PROLOGUE(CControlItem, AmbientProps)
    return (ULONG)pThis->ExternalAddRef();
}
 
 
STDMETHODIMP_(ULONG) CControlItem::XAmbientProps::Release()
{
    METHOD_PROLOGUE(CControlItem, AmbientProps)
    return (ULONG)pThis->ExternalRelease();
}
 
 
STDMETHODIMP CControlItem::XAmbientProps::QueryInterface(
    REFIID iid, LPVOID far* ppvObj)
{
    METHOD_PROLOGUE(CControlItem, AmbientProps)
    return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}
 
 
STDMETHODIMP CControlItem::XAmbientProps::GetTypeInfoCount(unsigned int FAR* pctinfo)
{
    METHOD_PROLOGUE(CControlItem, AmbientProps)
    ASSERT_VALID(pThis);
    *pctinfo = 0;
    return NOERROR;
}
 
 
STDMETHODIMP CControlItem::XAmbientProps::GetTypeInfo(unsigned int itinfo,
      LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
    METHOD_PROLOGUE(CControlItem, AmbientProps)
    ASSERT_VALID(pThis);
    return ResultFromScode(E_NOTIMPL);
}
 
 
STDMETHODIMP CControlItem::XAmbientProps::GetIDsOfNames(REFIID riid,
      LPOLESTR FAR* rgszNames, unsigned int cNames, LCID lcid,
      DISPID FAR* rgdispid)
{
    METHOD_PROLOGUE(CControlItem, AmbientProps)
    ASSERT_VALID(pThis);

    UINT nIdx = 0;
	CViewhtmlDoc* pDoc = (CViewhtmlDoc*)pThis->m_pDocument;

    while (nIdx < cNames) 
    {
        LPAPROP lpAprop = pDoc->FindAprop(CString(rgszNames[nIdx]));
        if (lpAprop)
        {
            rgdispid[nIdx] = lpAprop->dispid;
        } 
        else 
        {
            rgdispid[nIdx] = DISPID_UNKNOWN;
        }
        nIdx++;
    }
    return NOERROR;
}

 
STDMETHODIMP CControlItem::XAmbientProps::Invoke(DISPID dispidMember, REFIID riid,
      LCID lcid, unsigned short wFlags, DISPPARAMS FAR* lpDispparams, 
      VARIANT FAR* pvarResult, EXCEPINFO FAR* pexcepinfo, unsigned int FAR* puArgErr)
{
    METHOD_PROLOGUE(CControlItem, AmbientProps)
    ASSERT_VALID(pThis);

    HRESULT hr = ResultFromScode (DISP_E_MEMBERNOTFOUND);

	CViewhtmlDoc* pDoc = (CViewhtmlDoc*)pThis->m_pDocument;
	LPAPROP lpAprop = pDoc->FindAprop(dispidMember);
    if (lpAprop) 
    {
        TFVarCopy(pvarResult, &lpAprop->varValue);
        hr = NOERROR;
    }
    return (hr);
}

CRegEntry::CRegEntry()
: m_pName(NULL), m_pPath(NULL), m_hBitmap(NULL),  m_pTypeLib(NULL),
  m_bInsertable(FALSE), m_bExists(TRUE), m_pClsid(NULL),m_pProgID(NULL) 
{
}

CRegEntry::~CRegEntry()
{
	if (m_pName) 
		delete m_pName ;
	if (m_pPath) 
		delete m_pPath ;
	if (m_pClsid) 
		delete m_pClsid ;
	if (m_pProgID) 
		delete m_pProgID ;
	if (m_pTypeLib) 
		delete m_pTypeLib;
	if (m_hBitmap) ::DeleteObject(m_hBitmap) ;
}


CFileLoader::CFileLoader()
{
	m_loadingState = idle;
	m_callingOCX = NULL;
	m_memFile = NULL;
	m_diskFile = NULL;
}

CFileLoader::~CFileLoader()
{
}

void CFileLoader::OnNotify( UINT32 change_flags, CDynamicLoad *source )
{
	CDynamicLoad* 	loadedFile;

	if (m_destination == disk)
	{
		ASSERT(m_diskFile != NULL);
		loadedFile = (CDynamicLoad* ) m_diskFile;
	}
	else
	{
		ASSERT(m_memFile != NULL);
		loadedFile = (CDynamicLoad* ) m_memFile;
	}

	loadedFile->DEBUG_LOCK();
	::LOAD_STATE l = loadedFile->GetLoadState();
	loadedFile->Unlock();

	switch (l)
	{
		case LOAD_STATE_COMPLETE:

				ASSERT(m_callingOCX != NULL);	
				
				// Notify the calling object
				m_loadingState = idle;
				m_callingOCX->NotifyOCX(m_originalURL, TRUE);
				break;

		case LOAD_STATE_ABORTED:
				
				ASSERT(m_callingOCX != NULL);

				// Notify the calling object
				m_loadingState = idle;
				m_callingOCX->NotifyOCX(m_originalURL, FALSE);
				break;
	}				
}


/////////////////////////////////////////////////////////////////////////////
// CCntlDlg dialog


CCntlDlg::CCntlDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCntlDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCntlDlg)
	m_companyName = _T("");
	m_fileDescription = _T("");
	m_httpSite = _T("");
	m_legalCopyright = _T("");
	m_legalTrademarks = _T("");
	m_productName = _T("");
	m_productVersion = _T("");
	//}}AFX_DATA_INIT
}


void CCntlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCntlDlg)
	DDX_Text(pDX, IDC_COMPANY_NAME, m_companyName);
	DDX_Text(pDX, IDC_FILE_DESCRIPTION, m_fileDescription);
	DDX_Text(pDX, IDC_HTTP_SITE, m_httpSite);
	DDX_Text(pDX, IDC_LEGAL_COPYRIGHT, m_legalCopyright);
	DDX_Text(pDX, IDC_LEGAL_TRADEMARKS, m_legalTrademarks);
	DDX_Text(pDX, IDC_PRODUCT_NAME, m_productName);
	DDX_Text(pDX, IDC_PRODUCT_VERSION, m_productVersion);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCntlDlg, CDialog)
	//{{AFX_MSG_MAP(CCntlDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCntlDlg message handlers

///////////////////////////////////////////////////////////////////////////
// CNTLITEM.H -- Prototypes and declarations for the CControlItem class.
//		This class processes OLE Custom Control Events and allows a
//		container to add in event handling capability without a lot of fuss.


#ifndef __CNTLITEM_H__
#define __CNTLITEM_H__

#ifndef _DDSKFILE_H_
#include "ddskfile.h"
#endif

#ifndef _DMEMFILE_H_
#include "dmemfile.h"
#endif

#ifndef _FMTHTML_H_
#include "fmthtml.h"
#endif

#include "resource.h"

///////////////////////////////////////////////////////////////////////////
// classes declared in this file.

class CControlItem;	// Provides OCX Container Item Support
class CFileLoader;
class COleInfo;
class CRegEntry;
                                                          
///////////////////////////////////////////////////////////////////////////
// forward declarations

class CViewhtmlDoc;
class CViewhtmlView;

class CRegEntry
{
	public:
		CRegEntry() ; 
		virtual ~CRegEntry() ;

		char* m_pName ;
		char* m_pPath ;
		char* m_pClsid ; // For cleanup
		char* m_pProgID ; // For cleanup
		char* m_pTypeLib ; // For cleanup
		HBITMAP m_hBitmap ;
		BOOL m_bInsertable ;
		BOOL m_bExists ;
		//Bitmap ;		
};
                                                       
class CFileLoader  : public CObject, public CNotifyObject
{
public :
	CFileLoader();
	virtual ~CFileLoader();

	typedef enum
	{
		memory, disk
	}
	LOAD_DESTINATION;

	typedef enum
	{
		idle, loading
	}
	LOAD_STATE;

	LOAD_DESTINATION 	m_destination;
	CString				m_url;
	CString				m_originalURL;
	LOAD_STATE			m_loadingState;
	CDynLoadDiskFile*	m_diskFile;
	CDynLoadMemFile*	m_memFile;
	CControlItem*		m_callingOCX;
		
protected :
	long				m_fileID;
	virtual void 		OnNotify( UINT32 change_flags, CDynamicLoad *source);
};


class COleInfo : public CObject
{	
public :
	typedef enum
	{
		oleNotLoaded, oleLoading, oleLoaded
	}
	OLE_LOADING_STATE;
	
	OLE_LOADING_STATE	m_objectState;
	CObList				m_objectList;
	CObList				m_fileList;
}; 
	                                            	                                                         
///////////////////////////////////////////////////////////////////////////
// class CControlItem

class CControlItem : public COleClientItem, public CNotifyObject
{
	DECLARE_DYNAMIC(CControlItem)

// Constructors
public:
	CControlItem(CRect m_rect, CString ocxURL, CViewhtmlDoc* pContainerDoc = NULL);
	virtual 	 ~CControlItem();      

// Initialization
	void	Initialize(CString ocxURL);

	typedef enum
	{
		ocxdownload, filedownload, idle
	}
	CONTROLSTATE;

	typedef enum
	{
		memory, disk
	}
	LOADDESTINATION;

	typedef enum
	{
		startSearching,
		clsidNotFound,
		fileNotExist,
		incorrectVersion,
		incorrectSystem,
		correctVersion
	}
	REASONREGISTRY;
		
// Attributes
public:  
	// Event Information
    DISPPARAMS	m_LastParams;              
    DISPID		m_EventID;   
    
    DISPPARAMS FAR* 	GetParams() { return &m_LastParams; };
	LPTSTR		m_szObjCLSID;	// string CLSID of the 	object

protected:                          
	// Source location for the OCX in URL format
	CString				m_ocxURL;
 	CRect				m_rect;
	CSize				m_extent;
	LOADDESTINATION		m_destination;
	CONTROLSTATE		m_state;
	BOOL 				m_broken; 			// Broken OCX, not loaded
	BOOL				m_storageOpen;
	CDynLoadDiskFile*	m_loader;
	CLSID				m_objCLSID;
	CString				m_ocxFilename;
	CView*				m_pView;
	CViewhtmlDoc*		m_pDoc;
	OCXINFO				m_ocxInfo;

	REASONREGISTRY		m_reasonRegistry;
	CRegEntry			m_pRegEntry;

	// Some file loading related stuff
	int					m_fileNo;
	CObList				m_fileLoadingList;

	// This keeps a list of CControlItem objects
//	static CMapStringToOb	m_cntlItemList;
	 	 	
	// Attributes for the event connection and information
    LPCONNECTIONPOINT 	m_pConnPt;
    DWORD 				m_dwEventConnection;
    USHORT 				m_nEvents;
    EVENTINFO* 			m_pEventInfo;
	LPDISPATCH			m_pCtlDispatch;

	// Attributes for the data binding notification connection
    LPCONNECTIONPOINT 	m_pConnPtrBind;
	DWORD				m_dwBindConnection;
	USHORT				m_nBinds;
	BINDINFO*			m_pBindInfo;

public:
	// Attributes for the OLE event interface
	virtual LPUNKNOWN 	GetInterfaceHook(const void FAR* iid);
	LPUNKNOWN			GetCtlInterface(IID iidRequested);
	BOOL 				GetCLSID(CFormatOLEControlItem* item);

	// Info
public:
	void SetBroken()
	{  m_broken = TRUE; }
	void ResetBroken()
	{  m_broken = FALSE; }
	BOOL IsBroken() const
	{  return m_broken; }
	CRect GetRect() const
	{  return m_rect; }
	void SetRect(CRect rect)
	{  m_rect = rect; }
	void SetView(CView* pView)
	{  m_pView = pView; }
	void SetDocument(COleDocument* pDoc)
	{  m_pDoc = (CViewhtmlDoc *) pDoc; }
	BOOL CheckIfValidURL(CString url, CString& fileName, CString& endURL, CString baseDirectory);
	void SetObjectsBroken();	
	void ExitFileLoaders();
		
protected:	
	IID					m_iidEvents;


// Operations
public:
	// Handler routines for Events and Property Notifications
	// At some point, this could be replaced by maps, but not here.
	virtual void 	OnEvent(DISPID dispID, DISPPARAMS FAR* pDispParams);
	virtual HRESULT	OnPropertyNotification(DISPID dispID, UINT idsOccurence);

	void			CleanUpParams();
	void			CopyParams(DISPPARAMS FAR* lpDispparams);

	// Methods to insert OCXs
	void 			CheckAndInsertObject();
	BOOL 			UpdateExtent();


	// Notifications from the dynamic loader
	virtual void 	OnNotify( UINT32 change_flags, CDynamicLoad *source);
	
// Overrides
protected:
    virtual 		BOOL FinishCreate(HRESULT hr);
    virtual 		void Release(OLECLOSE dwCloseOption = OLECLOSE_NOSAVE);
//	virtual			void ReadItem(CArchive& ar);
	virtual			void SetExtent(CSize size);

// Override helpers
//	void 			ReadItemFlatConditional(CArchive& ar);
	BOOL			ReadCLSIDFromStorage();
	BOOL			CreateItemFromStorage();
	CString			GetVersionInfo(LPTSTR filename);

public:
	int				OpenStorage(CFormatOLEControlItem* item);
	
	// File loading related functions
	BOOL 			NotifyOCX(CString url, BOOL isLoaded);


// Implementation
protected:     
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	
	// Control Initialization Routines
    void 		InitControlInfo();
    void 		InitEventInfo(LPTYPEINFO);
	void		InitBindInfo();

	// Control Maintenance Routines
    void 			FreeControlInfo();
    EVENTINFO* 		GetEventInfo(MEMBERID memid);
	PARAMPROPINFO* 	GetParamPropInfo(DISPID id);
	PARAMPROPINFO*  GetParamPropInfo(CString propName);

	// OCX downloading and registration methods
	void 		DownloadOCX();
	BOOL 		RegisterOCX(BOOL fUnreg, CString ocxPath);
	void		ManualOCXCleanup(CString ocxPath);
	BOOL 		WipeOut(HKEY hKey, LPCTSTR lpszSubKey);
	BOOL		ReportIfLocalOCX();
	BOOL 		CheckIfLocalOCX(CString& ocxPath);
	void		AppendToLogFile();

	// File downloading related methods	
	BOOL 		DownloadFile(CString originalURL);
	void 		FileLoaderGarbageCollect();

	// Event related methods
	void 		ParseEvent(CString eventName);
	BOOL		ReadFile();


// Implemented Interfaces
protected:
	// Interface for Event Handling
    BEGIN_INTERFACE_PART(EventHandler, IDispatch)
        STDMETHOD(GetTypeInfoCount)(unsigned int FAR*);
        STDMETHOD(GetTypeInfo)(unsigned int, LCID, ITypeInfo FAR* FAR*);
        STDMETHOD(GetIDsOfNames)(REFIID, LPTSTR FAR*, unsigned int, LCID, DISPID FAR*);
        STDMETHOD(Invoke)(DISPID, REFIID, LCID, unsigned short, DISPPARAMS FAR*,
                          VARIANT FAR*, EXCEPINFO FAR*, unsigned int FAR*);
    END_INTERFACE_PART(EventHandler)

	// Interface for Property Notifications
    BEGIN_INTERFACE_PART(PropertyNotifySink, IPropertyNotifySink)
        STDMETHOD(OnChanged)(DISPID dispid);
        STDMETHOD(OnRequestEdit)(DISPID dispid);
    END_INTERFACE_PART(PropertyNotifySink)

	// Interface for Ambient Properties
    BEGIN_INTERFACE_PART(AmbientProps, IDispatch)
        STDMETHOD(GetTypeInfoCount)(unsigned int FAR*);
        STDMETHOD(GetTypeInfo)(unsigned int, LCID, ITypeInfo FAR* FAR*);
        STDMETHOD(GetIDsOfNames)(REFIID, LPTSTR FAR*, unsigned int, LCID, DISPID FAR*);
        STDMETHOD(Invoke)(DISPID, REFIID, LCID, unsigned short, DISPPARAMS FAR*,
                          VARIANT FAR*, EXCEPINFO FAR*, unsigned int FAR*);
    END_INTERFACE_PART(AmbientProps)

	DECLARE_INTERFACE_MAP()
};

///////////////////////////////////////////////////////////////////////////

#endif // __CNTLITEM_H__

///////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// CCntlDlg dialog

class CCntlDlg : public CDialog
{
// Construction
public:
	CCntlDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCntlDlg)
	enum { IDD = IDD_CNTL_DLG };
	CString	m_companyName;
	CString	m_fileDescription;
	CString	m_httpSite;
	CString	m_legalCopyright;
	CString	m_legalTrademarks;
	CString	m_productName;
	CString	m_productVersion;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCntlDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCntlDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

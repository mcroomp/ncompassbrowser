// inc\dlgpref.h : header file
//

#define _DLGPREF_H_

/////////////////////////////////////////////////////////////////////////////
// CDialogPref

class CDialogPref : public CPropertySheet
{
	DECLARE_DYNAMIC(CDialogPref)

// Construction
public:
	CDialogPref(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CDialogPref(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogPref)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDialogPref();

	// Generated message map functions
protected:
	//{{AFX_MSG(CDialogPref)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CDialogPrefUserInfo dialog

class CDialogPrefUserInfo : public CPropertyPage
{
	DECLARE_DYNCREATE(CDialogPrefUserInfo)

// Construction
public:
	CDialogPrefUserInfo();
	~CDialogPrefUserInfo();

// Dialog Data
	//{{AFX_DATA(CDialogPrefUserInfo)
	enum { IDD = IDD_PREF_USERINFO };
	CString	m_HomePage;
	BOOL	m_GotoHomePage;
	CString	m_UserEmail;
	CString	m_UserName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CDialogPrefUserInfo)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDialogPrefUserInfo)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CDialogPrefNetworkAndCache dialog

class CDialogPrefNetworkAndCache : public CPropertyPage
{
	DECLARE_DYNCREATE(CDialogPrefNetworkAndCache)

// Construction
public:
	CDialogPrefNetworkAndCache();
	~CDialogPrefNetworkAndCache();

// Dialog Data
	//{{AFX_DATA(CDialogPrefNetworkAndCache)
	enum { IDD = IDD_PREF_NETWORK };
	CString	m_CacheDirectory;
	CString	m_CacheMegabytes;
	CString	m_HttpProxy;
	UINT	m_MaxConnections;
	CString	m_SmtpServer;
	UINT	m_HttpProxyPort;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CDialogPrefNetworkAndCache)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDialogPrefNetworkAndCache)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CDialogPrefOleControls dialog

class CDialogPrefOleControls : public CPropertyPage
{
	DECLARE_DYNCREATE(CDialogPrefOleControls)

// Construction
public:
	CDialogPrefOleControls();
	~CDialogPrefOleControls();

// Dialog Data
	//{{AFX_DATA(CDialogPrefOleControls)
	enum { IDD = IDD_PREF_OLE_CONTROLS };
	CListBox	m_ControlList;
	CString	m_OcxDirectory;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CDialogPrefOleControls)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDialogPrefOleControls)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

// dlgpref.cpp : implementation file
//

#include "cross_p.h"
#include "resource.h"

#include "viewhtml.h"
#include "inc\dlgpref.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDialogPref

IMPLEMENT_DYNAMIC(CDialogPref, CPropertySheet)

CDialogPref::CDialogPref(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CDialogPref::CDialogPref(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

CDialogPref::~CDialogPref()
{
}


BEGIN_MESSAGE_MAP(CDialogPref, CPropertySheet)
	//{{AFX_MSG_MAP(CDialogPref)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDialogPref message handlers

/////////////////////////////////////////////////////////////////////////////
// CDialogPrefUserInfo property page

IMPLEMENT_DYNCREATE(CDialogPrefUserInfo, CPropertyPage)

CDialogPrefUserInfo::CDialogPrefUserInfo() : CPropertyPage(CDialogPrefUserInfo::IDD)
{
	//{{AFX_DATA_INIT(CDialogPrefUserInfo)
	m_HomePage = _T("");
	m_GotoHomePage = FALSE;
	m_UserEmail = _T("");
	m_UserName = _T("");
	//}}AFX_DATA_INIT
}

CDialogPrefUserInfo::~CDialogPrefUserInfo()
{
}

void CDialogPrefUserInfo::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogPrefUserInfo)
	DDX_Text(pDX, IDC_HOME_PAGE, m_HomePage);
	DDX_Check(pDX, IDC_LOAD_HOME_PAGE, m_GotoHomePage);
	DDX_Text(pDX, IDC_USER_EMAIL, m_UserEmail);
	DDX_Text(pDX, IDC_USER_NAME, m_UserName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogPrefUserInfo, CPropertyPage)
	//{{AFX_MSG_MAP(CDialogPrefUserInfo)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDialogPrefUserInfo message handlers

/////////////////////////////////////////////////////////////////////////////
// CDialogPrefNetworkAndCache property page

IMPLEMENT_DYNCREATE(CDialogPrefNetworkAndCache, CPropertyPage)

CDialogPrefNetworkAndCache::CDialogPrefNetworkAndCache() : CPropertyPage(CDialogPrefNetworkAndCache::IDD)
{
	//{{AFX_DATA_INIT(CDialogPrefNetworkAndCache)
	m_CacheDirectory = _T("");
	m_CacheMegabytes = _T("");
	m_HttpProxy = _T("");
	m_MaxConnections = 0;
	m_SmtpServer = _T("");
	m_HttpProxyPort = 0;
	//}}AFX_DATA_INIT
}

CDialogPrefNetworkAndCache::~CDialogPrefNetworkAndCache()
{
}

void CDialogPrefNetworkAndCache::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogPrefNetworkAndCache)
	DDX_Text(pDX, IDC_CACHE_DIRECTORY, m_CacheDirectory);
	DDX_Text(pDX, IDC_CACHE_MEGABYTES, m_CacheMegabytes);
	DDX_Text(pDX, IDC_HTTP_PROXY, m_HttpProxy);
	DDX_Text(pDX, IDC_NUM_CONNECTIONS, m_MaxConnections);
	DDV_MinMaxUInt(pDX, m_MaxConnections, 1, 4);
	DDX_Text(pDX, IDC_SMTP_MAIL_SERVER, m_SmtpServer);
	DDX_Text(pDX, IDC_PORT_NUMBER, m_HttpProxyPort);
	DDV_MinMaxUInt(pDX, m_HttpProxyPort, 1, 9999);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogPrefNetworkAndCache, CPropertyPage)
	//{{AFX_MSG_MAP(CDialogPrefNetworkAndCache)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDialogPrefNetworkAndCache message handlers
/////////////////////////////////////////////////////////////////////////////
// CDialogPrefOleControls property page

IMPLEMENT_DYNCREATE(CDialogPrefOleControls, CPropertyPage)

CDialogPrefOleControls::CDialogPrefOleControls() : CPropertyPage(CDialogPrefOleControls::IDD)
{
	//{{AFX_DATA_INIT(CDialogPrefOleControls)
	m_OcxDirectory = _T("");
	//}}AFX_DATA_INIT
}

CDialogPrefOleControls::~CDialogPrefOleControls()
{
}

void CDialogPrefOleControls::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogPrefOleControls)
	DDX_Control(pDX, IDC_CONTROL_LIST, m_ControlList);
	DDX_Text(pDX, IDC_OCX_DIRECTORY, m_OcxDirectory);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogPrefOleControls, CPropertyPage)
	//{{AFX_MSG_MAP(CDialogPrefOleControls)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDialogPrefOleControls message handlers

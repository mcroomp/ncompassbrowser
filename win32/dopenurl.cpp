// dopenurl.cpp : implementation file
//

#include "cross_p.h"
#include "viewhtml.h"
#include "inc\dopenurl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDialogOpenURL dialog


CDialogOpenURL::CDialogOpenURL(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogOpenURL::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogOpenURL)
	m_url = _T("");
	//}}AFX_DATA_INIT
}


void CDialogOpenURL::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogOpenURL)
	DDX_Text(pDX, IDC_URL_EDIT, m_url);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogOpenURL, CDialog)
	//{{AFX_MSG_MAP(CDialogOpenURL)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDialogOpenURL message handlers

// inc\dopenurl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDialogOpenURL dialog

class CDialogOpenURL : public CDialog
{
// Construction
public:
	CDialogOpenURL(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDialogOpenURL)
	enum { IDD = IDD_OPEN_URL };
	CString	m_url;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogOpenURL)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogOpenURL)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

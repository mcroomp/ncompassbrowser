// inctldlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COleInsertCtlDlg dialog

class COleInsertCtlDlg : public COleInsertDialog
{
// Construction
public:
	COleInsertCtlDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COleInsertCtlDlg)
	enum { IDD =  IDD_INSERTCTL };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COleInsertCtlDlg)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CString		m_strName;
	CLSID		m_CLSID;

	// Generated message map functions
	//{{AFX_MSG(COleInsertCtlDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

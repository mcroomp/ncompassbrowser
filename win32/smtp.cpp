// smtp.cpp : implementation file
//

#include "cross_p.h"
#include "resource.h"

#include "viewhtml.h"
#include "viewhdoc.h"

#include "smtp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDialogSendMail dialog

class CDialogSendMail : public CDialog
{
// Construction
public:
	CDialogSendMail(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDialogSendMail)
	enum { IDD = IDD_SEND_MAIL };
	CString	m_To;
	CString	m_Subject;
	CString	m_CC;
	CString	m_Body;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogSendMail)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogSendMail)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CDialogSendMail::CDialogSendMail(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogSendMail::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogSendMail)
	m_To = _T("");
	m_Subject = _T("");
	m_CC = _T("");
	m_Body = _T("");
	//}}AFX_DATA_INIT
}


void CDialogSendMail::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogSendMail)
	DDX_Text(pDX, IDC_MESSAGE_TO, m_To);
	DDX_Text(pDX, IDC_MESSAGE_SUBJECT, m_Subject);
	DDX_Text(pDX, IDC_MESSAGE_CC, m_CC);
	DDX_Text(pDX, IDC_MESSAGE_BODY, m_Body);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogSendMail, CDialog)
	//{{AFX_MSG_MAP(CDialogSendMail)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDialogSendMail message handlers


void CProtocolSMTP::ShowDialog(LPCSTR url)
	{
	CDialogSendMail dialog;

	dialog.m_To = CString( url + 7 );
	if (dialog.DoModal() == TRUE)
		{
		CString header = 	"To: " + dialog.m_To + '\n';

		if (!dialog.m_Subject.IsEmpty())
			header += "Subject: " + dialog.m_Subject + '\n';
		else
			header += "Subject: (no subject)\n";
		
		if (!dialog.m_CC.IsEmpty())
			header += "cc: " + dialog.m_CC + '\n';
		
		theApp.m_viewhtml_doc->OpenURL( url, METHOD_POST, header, dialog.m_Body);
		}
	}


CProtocolSMTP::CProtocolSMTP(CDynamicLoad *load_object)
	: CProtocol(load_object)
	{

	}

CProtocolSMTP::~CProtocolSMTP()
	{

	}

void CProtocolSMTP::BeginLoadThread()
	{
	m_load_object->DEBUG_LOCK();
	METHOD_TYPE m = m_load_object->GetMethodType();
	m_load_object->Unlock();

	ASSERT(m == METHOD_POST);
	
	m_thread_handle = AfxBeginThread( SMTPWorkerThread, (LPVOID)this );
	}

void CProtocolSMTP::AbortLoadThread()
	{
	m_socket.Abort();
	WaitForSingleObject( m_thread_done_semaphore, INFINITE );
	m_thread_handle = NULL;
	}

void CProtocolSMTP::WaitEndLoadThread()
	{
	WaitForSingleObject( m_thread_done_semaphore, INFINITE );
	m_thread_handle = NULL;
	}

UINT SMTPWorkerThread( LPVOID lparam )
	{
	CProtocolSMTP *protocol = (CProtocolSMTP *)lparam;
	CDynamicLoad *dlobject = protocol->m_load_object;

	CMapStringToString mime_header;

	mime_header[ "content-type" ] = "internal/smtp-post";

	protocol->CallOnBeginLoading(mime_header);
	LOAD_STATE l = protocol->CallOnEndLoading();

	dlobject->DEBUG_LOCK();
	protocol->CallNotify(CHANGEFLAG_DONE);
	protocol->SetLoadState(l);
	dlobject->Unlock();
	
	ReleaseSemaphore(protocol->m_thread_done_semaphore,1,NULL);
	return 0;
	}

CMimeSMTP::CMimeSMTP(LPCSTR url, LPCSTR mime_type)
	: CMimeObject(MIME_OBJECT_LAUNCH_SMTP, url, mime_type)
	{}
	
LOAD_STATE CMimeSMTP::OnReadData(LPCBYTE buffer, INT32 buffer_size)
	{ return LOAD_STATE_LOADING; }
LOAD_STATE CMimeSMTP::OnEndOfFile()
	{ return LOAD_STATE_ABORTED; }			// always return aborted to prevent caching of mail

BOOL CMimeSMTP::UsesInternalViewer() const
	{ return FALSE; }

void CMimeSMTP::LaunchViewer()
	{
	}

// inctldlg.cpp : implementation file
//

#include "cross_p.h"
#include "viewhtml.h"
#include "inctldlg.h"
#include <olectl.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COleInsertCtlDlg dialog

COleInsertCtlDlg::COleInsertCtlDlg(CWnd* pParent /*=NULL*/)
	: COleInsertDialog(IOF_SELECTCREATENEW, pParent)
{
	//{{AFX_DATA_INIT(COleInsertCtlDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void COleInsertCtlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COleInsertCtlDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COleInsertCtlDlg, CDialog)
	//{{AFX_MSG_MAP(COleInsertCtlDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COleInsertCtlDlg message handlers

BOOL COleInsertCtlDlg::OnInitDialog() 
{
	COleInsertDialog::OnInitDialog();

	CListBox*	pBox;
	pBox = (CListBox*) GetDlgItem(IDC_CONTROLS);
	ASSERT(pBox != NULL);

    UINT        cStrings=0;
	int			cch;
    HKEY        hKey;
    LONG        lRet;
    LPTSTR      pszClass;
    LPTSTR      pszClsid;
	LPTSTR		pszKey;
	HKEY		hKeyTemp;
	TCHAR		szID[50];

    pszClass=(LPTSTR) DEBUG_NEW TCHAR[_MAX_PATH*3];
    if (NULL==pszClass)
        return FALSE;
    pszClsid=pszClass+_MAX_PATH;
	pszKey=pszClsid+_MAX_PATH;

    //Open up the root key.
    lRet=RegOpenKey(HKEY_CLASSES_ROOT, NULL, &hKey);

    if ((LONG)ERROR_SUCCESS!=lRet)
    {
        delete pszClass;
        return FALSE;
    }

    //Clean out the existing strings.
	pBox->ResetContent();

    while (TRUE)
    {
        lRet=RegEnumKey(hKey, cStrings++, pszClass, _MAX_PATH);

        if ((LONG)ERROR_SUCCESS!=lRet)
            break;

        // Get full user type name
        DWORD dw=_MAX_PATH;
        lRet=RegQueryValue(hKey, pszClass, pszKey, (LONG*)&dw);

		// Get class ID
        lstrcat(pszClass, TEXT("\\CLSID"));
        dw=50 * sizeof(TCHAR);
        lRet = RegQueryValue(hKey, pszClass, szID, (LONG*)&dw);

        if ((LONG)ERROR_SUCCESS!=lRet)
            continue;   // CLSID subkey not found

        lstrcpy(pszClsid, TEXT("CLSID\\"));
        lstrcat(pszClsid, szID);
		lstrcat(pszClsid, TEXT("\\"));
		cch = lstrlen(pszClsid);
		lstrcpy(pszClsid+cch, TEXT("Control"));

		hKeyTemp = NULL;
        lRet=RegOpenKey(hKey, pszClsid, &hKeyTemp);
		if (hKeyTemp != NULL)
			RegCloseKey(hKeyTemp);

        if ((LONG)ERROR_SUCCESS!=lRet)
            continue;   // Control NOT found--skip this class

		// Look for InprocServer[32] or LocalServer[32] key
		lstrcpy(pszClsid+cch, _T("InprocServer32"));

		hKeyTemp = NULL;
        lRet=RegOpenKey(hKey, pszClsid, &hKeyTemp);
		if (hKeyTemp != NULL)
			RegCloseKey(hKeyTemp);

        if ((LONG)ERROR_SUCCESS!=lRet)	// InprocServer[32] not found
		{
			lstrcpy(pszClsid+cch, _T("LocalServer32"));

			hKeyTemp = NULL;
    	    lRet=RegOpenKey(hKey, pszClsid, &hKeyTemp);
			if (hKeyTemp != NULL)
				RegCloseKey(hKeyTemp);

	        if ((LONG)ERROR_SUCCESS!=lRet)	// LocalServer[32] not found
    	        continue;
		}

        //We got through all the conditions, add the string.
        lstrcat(pszKey, _T("\t"));

        // only add to listbox if not a duplicate
        if (LB_ERR == pBox->FindString(-1, pszKey)) 
        {
            lstrcat(pszKey, szID);
            pBox->AddString(pszKey);
        }
    }

	//set the tab stop to be about 1/3 of the box

	CRect rectBox;
	int	nThird;
	pBox->GetWindowRect(rectBox);
	nThird = rectBox.Width()/12;		// dialog units
	pBox->SetTabStops(nThird);

    //Select the first item by default
    pBox->SetCurSel(0);
    RegCloseKey(hKey);
    delete pszClass;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COleInsertCtlDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	COleInsertDialog::OnOK();
}

int COleInsertCtlDlg::DoModal()
{
        m_io.hInstance = AfxGetResourceHandle();
        m_io.lpszTemplate = MAKEINTRESOURCE(IDD_INSERTCTL);

        return COleInsertDialog::DoModal();
}


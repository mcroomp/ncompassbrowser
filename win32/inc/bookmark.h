#define _BOOKMARK_H_

class CBookmarkLibrary
	{
public:
	CBookmarkLibrary();
	~CBookmarkLibrary();

	CBookmarkLibrary& operator = (const CBookmarkLibrary& source );

	void StoreBookmarks(LPCTSTR filename);
	void LoadBookmarks(LPCTSTR filename);

	void AddBookmark( LPCSTR title, LPCSTR url );
	void DeleteBookmark( INT index );
	void ChangeBookmark( INT index, LPCSTR new_title, LPCSTR new_url );

	void MoveBookmarkUp( INT index );
	void MoveBookmarkDown( INT index );

	INT GetBookmarkCount() const;

	CString GetBookmarkURL(INT index);
	CString GetBookmarkTitle(INT index);
private:
	CStringArray m_title_array, m_url_array;	
	};
/////////////////////////////////////////////////////////////////////////////
// CDialogViewBookmarks dialog

class CDialogViewBookmarks : public CDialog
{
// Construction
public:
	CDialogViewBookmarks(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDialogViewBookmarks)
	enum { IDD = IDD_BOOKMARKS };
	CStatic	m_current_url;
	CListBox	m_list_control;
	//}}AFX_DATA

	CBookmarkLibrary m_bookmark_library;
	CString m_input_url;
	CString m_input_title;
	CString m_output_url;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogViewBookmarks)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void UpdateUI();

	// Generated message map functions
	//{{AFX_MSG(CDialogViewBookmarks)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeBookmarkList();
	afx_msg void OnButtonDeleteItem();
	afx_msg void OnButtonEditItem();
	afx_msg void OnButtonMoveDown();
	afx_msg void OnButtonMoveUp();
	afx_msg void OnButtonNewItem();
	afx_msg void OnButtonOpenUrl();
	afx_msg void OnDblclkBookmarkList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CDialogEditBookmark dialog

class CDialogEditBookmark : public CDialog
{
// Construction
public:
	CDialogEditBookmark(CWnd* pParent = NULL);   // standard constructor

	BOOL m_new_url;

// Dialog Data
	//{{AFX_DATA(CDialogEditBookmark)
	enum { IDD = IDD_EDIT_BOOKMARK };
	CString	m_edit_title;
	CString	m_edit_url;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogEditBookmark)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogEditBookmark)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

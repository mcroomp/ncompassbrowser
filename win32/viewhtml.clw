; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CViewhtmlDoc
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "viewhtml.h"
ODLFile=viewhtml.odl
LastPage=0

ClassCount=15
Class1=CViewhtmlApp
Class2=CViewhtmlDoc
Class3=CViewhtmlView
Class4=CMainFrame

ResourceCount=15
Resource1=IDD_ABOUTBOX
Resource2=IDD_LOCATION_BAR
Resource4=IDD_OPEN_URL
Class5=CAboutDlg
Resource3=IDR_CNTR_INPLACE
Resource5=IDD_INSERTCTL
Class6=CDialogOpenURL
Resource6=IDR_MENU1
Class7=CLocationDialogBar
Resource7=IDD_PREF_NETWORK
Resource8=IDD_CNTL_DLG
Class8=CCntlDlg
Resource9=IDD_BOOKMARKS
Resource10=IDD_EDIT_BOOKMARK
Resource11=IDD_SAVE_URL
Class9=CDialogSendMail
Class10=CDialogEditBookmark
Resource12=IDD_PREF_USERINFO
Class11=CDialogPref
Class12=CDialogPrefUserInfo
Resource13=IDR_MAINFRAME
Resource14=IDD_PREF_OLE_CONTROLS
Class13=CDialogPrefNetworkAndCache
Class14=CDialogPrefOleControls
Resource15=IDD_SEND_MAIL
Class15=CDialogViewBookmarks

[CLS:CViewhtmlApp]
Type=0
HeaderFile=inc\viewhtml.h
ImplementationFile=viewhtml.cpp
LastObject=ID_GO_BACK
Filter=N
VirtualFilter=AC

[CLS:CViewhtmlDoc]
Type=0
HeaderFile=inc\viewhdoc.h
ImplementationFile=viewhdoc.cpp
LastObject=ID_STOP_LOADING
Filter=N
VirtualFilter=ODC

[CLS:CViewhtmlView]
Type=0
HeaderFile=inc\viewhvw.h
ImplementationFile=viewhvw.cpp
Filter=C
VirtualFilter=VWC
LastObject=ID_EDIT_PASTE

[CLS:CMainFrame]
Type=0
HeaderFile=inc\mainfrm.h
ImplementationFile=mainfrm.cpp
LastObject=ID_EDIT_FIND
Filter=T
VirtualFilter=fWC

[CLS:CAboutDlg]
Type=0
HeaderFile=viewhtml.cpp
ImplementationFile=viewhtml.cpp
Filter=D
LastObject=CAboutDlg

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=23
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,static,1342308352
Control9=IDC_STATIC,static,1342308352
Control10=IDC_STATIC,static,1342308352
Control11=IDC_STATIC,static,1342308352
Control12=IDC_STATIC,static,1342308352
Control13=IDC_STATIC,static,1342308352
Control14=IDC_STATIC,static,1342308352
Control15=IDC_STATIC,static,1342308352
Control16=IDC_STATIC,static,1342308352
Control17=IDC_STATIC,static,1342308352
Control18=IDC_STATIC,static,1342308352
Control19=IDC_STATIC,static,1342308352
Control20=IDC_STATIC,static,1342308352
Control21=IDC_STATIC,static,1342308352
Control22=IDC_STATIC,static,1342308352
Control23=IDC_STATIC,static,1342308352

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_OPEN
Command2=ID_FILE_OPEN_URL
Command3=ID_FILE_PRINT
Command4=ID_FILE_PRINT_PREVIEW
Command5=ID_FILE_PRINT_SETUP
Command6=ID_APP_EXIT
Command7=ID_EDIT_CUT
Command8=ID_EDIT_COPY
Command9=ID_EDIT_PASTE
Command10=ID_EDIT_FIND
Command11=ID_VIEW_TOOLBAR
Command12=ID_VIEW_STATUS_BAR
Command13=ID_VIEW_LOCATION_BAR
Command14=ID_RELOAD
Command15=ID_GO_BACK
Command16=ID_GO_FORWARD
Command17=ID_GO_HOME
Command18=ID_STOP_LOADING
Command19=ID_ADD_BOOKMARK
Command20=ID_EDIT_BOOKMARKS
Command21=ID_BOOKMARK00
Command22=ID_OPTIONS_PREFERENCES
Command23=ID_APP_ABOUT
CommandCount=23

[MNU:IDR_CNTR_INPLACE]
Type=1
Class=CViewhtmlView
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_SAVE_AS
Command5=ID_FILE_PRINT
Command6=ID_FILE_PRINT_PREVIEW
Command7=ID_FILE_PRINT_SETUP
Command8=ID_FILE_MRU_FILE1
Command9=ID_APP_EXIT
CommandCount=9

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_EDIT_UNDO
Command6=ID_EDIT_CUT
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_NEXT_PANE
Command14=ID_PREV_PANE
Command15=ID_CANCEL_EDIT_CNTR
CommandCount=15

[ACL:IDR_CNTR_INPLACE]
Type=1
Class=CViewhtmlView
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_NEXT_PANE
Command6=ID_PREV_PANE
Command7=ID_CANCEL_EDIT_CNTR
CommandCount=7

[DLG:IDD_OPEN_URL]
Type=1
Class=CDialogOpenURL
ControlCount=3
Control1=IDC_URL_EDIT,edit,1350631552
Control2=IDOK,button,1342242817
Control3=IDCANCEL,button,1342242816

[DLG:IDD_INSERTCTL]
Type=1
ControlCount=4
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=65535,static,1342308352
Control4=IDC_CONTROLS,listbox,1352728963

[CLS:CDialogOpenURL]
Type=0
HeaderFile=inc\dopenurl.h
ImplementationFile=dopenurl.cpp
Filter=D
VirtualFilter=dWC
LastObject=IDC_URL_EDIT

[MNU:IDR_MENU1]
Type=1
CommandCount=0

[DLG:IDD_LOCATION_BAR]
Type=1
Class=CLocationDialogBar
ControlCount=3
Control1=IDC_LOCATION,edit,1350631552
Control2=IDC_STATIC,static,1342308352
Control3=ID_OPEN_LOCATION,button,1073807361

[CLS:CLocationDialogBar]
Type=0
HeaderFile=inc\mainfrm.h
ImplementationFile=mainfrm.cpp
Filter=D
LastObject=CLocationDialogBar
VirtualFilter=dWC

[DLG:IDD_CNTL_DLG]
Type=1
Class=CCntlDlg
ControlCount=20
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308353
Control4=IDC_STATIC,static,1342177283
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,static,1342308352
Control9=IDC_STATIC,static,1342308352
Control10=IDC_STATIC,static,1342308352
Control11=IDC_STATIC,static,1342308352
Control12=IDC_STATIC,static,1342308352
Control13=IDC_PRODUCT_NAME,edit,1350633600
Control14=IDC_COMPANY_NAME,edit,1350633600
Control15=IDC_FILE_DESCRIPTION,edit,1350633600
Control16=IDC_PRODUCT_VERSION,edit,1350633600
Control17=IDC_LEGAL_COPYRIGHT,edit,1350633600
Control18=IDC_LEGAL_TRADEMARKS,edit,1350633600
Control19=IDC_HTTP_SITE,edit,1350633600
Control20=IDC_STATIC,static,1342308353

[CLS:CCntlDlg]
Type=0
HeaderFile=inc\cntlitem.h
ImplementationFile=cntlitem.cpp
Filter=D
LastObject=IDCANCEL
VirtualFilter=dWC

[DLG:IDD_EDIT_BOOKMARK]
Type=1
Class=CDialogEditBookmark
ControlCount=6
Control1=IDC_EDIT_TITLE,edit,1350631552
Control2=IDC_EDIT_URL,edit,1350631552
Control3=IDOK,button,1342242817
Control4=IDCANCEL,button,1342242816
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352

[DLG:IDD_SAVE_URL]
Type=1
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308353

[DLG:IDD_PREF_NETWORK]
Type=1
Class=CDialogPrefNetworkAndCache
ControlCount=15
Control1=IDC_STATIC,static,1342308352
Control2=IDC_SMTP_MAIL_SERVER,edit,1350631552
Control3=IDC_STATIC,static,1342308352
Control4=IDC_HTTP_PROXY,edit,1350631552
Control5=IDC_STATIC,static,1342308352
Control6=IDC_NUM_CONNECTIONS,edit,1350631552
Control7=IDC_STATIC,static,1342308352
Control8=IDC_CACHE_DIRECTORY,edit,1350631552
Control9=IDC_STATIC,static,1342308352
Control10=IDC_CACHE_MEGABYTES,static,1342308354
Control11=IDC_CHANGE_MEGABYTE,msctls_updown32,1342177456
Control12=IDC_STATIC,static,1342308352
Control13=IDC_CLEAR_DISK_CACHE,button,1342242816
Control14=IDC_PORT_NUMBER,edit,1350631552
Control15=IDC_PORT_TEXT,static,1342308352

[CLS:CDialogPref]
Type=0
HeaderFile=inc\dlgpref.h
ImplementationFile=dlgpref.cpp
LastObject=CDialogPref

[DB:CDialogPref]
DB=1
ColumnCount=0
LastClass=CDialogPref

[CLS:CDialogEditBookmark]
Type=0
HeaderFile=inc\bookmark.h
ImplementationFile=bookmark.cpp
LastObject=CDialogEditBookmark

[DB:CDialogEditBookmark]
DB=1
ColumnCount=-1
LastClass=CDialogViewBookmarks
ClassCount=1
Class1=CDialogViewBookmarks

[DB:CDialogViewBookmarks]
DB=1
ColumnCount=-1
LastClass=CDialogPrefNetwork

[DLG:IDD_PREF_USERINFO]
Type=1
Class=CDialogPrefUserInfo
ControlCount=7
Control1=IDC_STATIC,static,1342308352
Control2=IDC_USER_NAME,edit,1350631552
Control3=IDC_STATIC,static,1342308352
Control4=IDC_USER_EMAIL,edit,1350631552
Control5=IDC_STATIC,static,1342308352
Control6=IDC_HOME_PAGE,edit,1350631552
Control7=IDC_LOAD_HOME_PAGE,button,1342242819

[CLS:CDialogPrefUserInfo]
Type=0
HeaderFile=inc\dlgpref.h
ImplementationFile=dlgpref.cpp
Filter=D
LastObject=CDialogPrefUserInfo

[DLG:IDD_PREF_OLE_CONTROLS]
Type=1
Class=CDialogPrefOleControls
ControlCount=5
Control1=IDC_STATIC,static,1342308352
Control2=IDC_OCX_DIRECTORY,edit,1350631552
Control3=IDC_CONTROL_LIST,listbox,1352728835
Control4=IDC_STATIC,static,1342308352
Control5=IDC_UNREGISTER_CONTROL,button,1342242816

[CLS:CDialogPrefNetworkAndCache]
Type=0
HeaderFile=inc\dlgpref.h
ImplementationFile=dlgpref.cpp
Filter=D
LastObject=CDialogPrefNetworkAndCache

[CLS:CDialogPrefOleControls]
Type=0
HeaderFile=inc\dlgpref.h
ImplementationFile=dlgpref.cpp
Filter=D
LastObject=CDialogPrefOleControls

[DLG:IDD_SEND_MAIL]
Type=1
Class=CDialogSendMail
ControlCount=9
Control1=IDC_MESSAGE_TO,edit,1350631552
Control2=IDC_MESSAGE_SUBJECT,edit,1350631552
Control3=IDC_MESSAGE_CC,edit,1350631552
Control4=IDC_MESSAGE_BODY,edit,1352732676
Control5=IDCANCEL,button,1342242816
Control6=IDOK,button,1342242817
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,static,1342308352
Control9=IDC_STATIC,static,1342308352

[CLS:CDialogSendMail]
Type=0
HeaderFile=smtp.cpp
ImplementationFile=smtp.cpp
Filter=D
LastObject=CDialogSendMail

[DLG:IDD_BOOKMARKS]
Type=1
ControlCount=10
Control1=IDC_BOOKMARK_LIST,listbox,1352728577
Control2=IDC_BUTTON_NEW_ITEM,button,1342242816
Control3=IDC_BUTTON_EDIT_ITEM,button,1342242816
Control4=IDC_BUTTON_DELETE_ITEM,button,1342242816
Control5=IDC_BUTTON_MOVE_UP,button,1342242816
Control6=IDC_BUTTON_MOVE_DOWN,button,1342242816
Control7=IDOK,button,1342242817
Control8=IDCANCEL,button,1342242816
Control9=IDC_BUTTON_OPEN_URL,button,1342242816
Control10=IDC_LIST_URL,static,1342308352

[CLS:CDialogViewBookmarks]
Type=0
HeaderFile=inc\bookmark.h
ImplementationFile=bookmark.cpp
LastObject=IDC_BOOKMARK_LIST
Filter=D
VirtualFilter=dWC


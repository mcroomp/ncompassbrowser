#include "cross_p.h"

#include "MacMain.h"

#define new DEBUG_NEW

/* function prototypes to make MetroWerks happy */

void ToolBoxInit (void);
int WindowInit (void);
void EventLoop (void);
pascal void ScrollProc(ControlHandle theControl, short partCode);
int SetupControls(WindowPtr window);
void HandleMouseDown( EventRecord *eventPtr);
void DoEvent( EventRecord *eventPtr);
void TrackThumb(ControlHandle theControl, Point thePoint);
const char *GetString(const char *p, CString& store);
int ParseInputFile();
BOOL PtInButton(int x,int y, Point p);
BOOL TrackButtonDown(int x,int y);
int TrackAttachmentsPopup(ControlHandle theControl);
int TrackOverviewPopup(ControlHandle theControl, Point thePoint);
int TrackHeadingPopup(ControlHandle theControl, Point thePoint);
void MakeCurrent(WindowPtr window, CFileRecord *r);
BOOL FreeSomeMemory();
void RecalcLayout(WindowPtr window);

CFileRecord * CFileRecord::m_first = NULL;
CFileRecord * CFileRecord::m_last = NULL;

long gPageSize;
long cur_item_number = 1;

long gYOffset = 0;
long gMaxYOffset = 0;

CFileRecord *gCurrentFile;

Rect	gWindowRect;
Rect	gItemRect;
Rect 	gTopPictRect;
Rect	gButPictRect;

Boolean	gDone = false;
ControlHandle scrollbar_control, heading_control, overview_control, attach_control;
	
ControlActionUPP pScrollActionUPP;

static MenuHandle overview_menu;
static Str255 overview_title = "\pOverview";

BOOL FreeSomeMemory()
	{
	CFileRecord *walk = CFileRecord::m_first;
	while(walk)
		{
		if (walk != gCurrentFile)
			{
			if (walk->m_formathtml)
				{
				delete walk->m_formathtml;
				walk->m_formathtml = NULL;
				return TRUE;
				}	
			}
		walk = walk->m_next;
		}
		
	walk = CFileRecord::m_first;
	while(walk)
		{
		if (walk != gCurrentFile)
			{
			if (walk->m_parsehtml)
				{
				delete walk->m_parsehtml;
				walk->m_parsehtml = NULL;
				return TRUE;
				}	
			}
		walk = walk->m_next;
		}

	return FALSE;
	}

int main ()
{
	__catch_buffer = malloc(1024);	//	allocate bigger buffer for exceptions	
	
	try
		{
		gWindowRect.top = 0;
		gWindowRect.left = 0;
		gWindowRect.right = 498;
		gWindowRect.bottom = 382;
		
		gTopPictRect.left = 0;
		gTopPictRect.top = 0;
		gTopPictRect.right = 499;
		gTopPictRect.bottom = 74;
		
		gItemRect.left = 2;
		gItemRect.top = 96;
		gItemRect.right = 370;
		gItemRect.bottom = 261+92;
		
	 	gButPictRect.left = gWindowRect.right - 111;
		gButPictRect.top  = gWindowRect.bottom - 86;
		gButPictRect.right = gWindowRect.right;
		gButPictRect.bottom = gWindowRect.bottom;
		
		pScrollActionUPP = NewControlActionProc( ScrollProc );
	 	
		gPageSize = gItemRect.bottom - gItemRect.top;
		
		ToolBoxInit ();
	
		ParseInputFile();
		gCurrentFile = CFileRecord::m_first;
		
		if (WindowInit ())
			{
			ASSERT(FALSE);
			return -1;
			}
		EventLoop ();
	
		CFileRecord *p = CFileRecord::m_first;
		while(p)
			{
			CFileRecord *tmp = p->m_next;
			delete p;
			p = tmp;
			}
		
		ClearPictureCache();
	#ifndef NDEBUG
		DumpMemory();
	#endif
		}
	catch( CViewException e )
		{
		char buffer[256] = "Unhandled exception:\r";
		
		e.FormatErrorString(buffer + strlen(buffer) );
		ErrorExit(buffer);
		}
	ExitMovies();
	return 0;
}

void ToolBoxInit (void)
{
	InitGraf ( &qd.thePort );
	InitFonts ();
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (nil);
	InitCursor ();
	EnterMovies();
}

int SetupControls(WindowPtr window)
	{
	Rect vScrollRect;
	
	vScrollRect.left =  gItemRect.right + 2;
	vScrollRect.top  = gItemRect.top - 1;
	vScrollRect.right = gItemRect.right + 18;
	vScrollRect.bottom = gItemRect.bottom + 1;
	
	scrollbar_control = NewControl( window, &vScrollRect, "\p", true, 1, 1, 1, scrollBarProc, 1);
	
	Rect popupRect;
	
	popupRect.left = 10;
	popupRect.top = 358;
	popupRect.right = 10 + 150;
	popupRect.bottom = 358 + 20;
	
	overview_menu = GetMenu(MENU_OVERVIEW);
	InsMenuItem(overview_menu, overview_title, 0);
	CFileRecord *p = CFileRecord::m_first;
	int menusize = 1;
	while(p)
		{
		Str255 buffer;
		p->m_title.GetPString(buffer);		
		InsertMenuItem(overview_menu, buffer, menusize);
		menusize++;
		p = p->m_next;
		}		
	
	attach_control = NewControl(window, &popupRect, "\p", true, popupTitleLeftJust, MENU_ATTACHMENTS, 0, popupMenuProc, 2);
	
	popupRect.left = 140;
	popupRect.right = 140 + 110;
	
	overview_control = NewControl(window, &popupRect, "\p", true, popupTitleLeftJust, MENU_OVERVIEW, 0, popupMenuProc, 3);
	
	popupRect.left = 260;
	popupRect.right = 260 + 100;
	
	heading_control = NewControl(window, &popupRect, "\p", true, popupTitleLeftJust, MENU_HEADINGS, 0, popupMenuProc, 4);
	
	return 0;
	}

int WindowInit(void)
	{
	WindowPtr window;
	
	if (( window = GetNewCWindow( 128, nil, (WindowPtr)-1L) ) == nil)
		return -1;
	if (SetupControls(window))
		return -1;
	SetPort(window);
	ShowWindow(window);
	SetWTitle(window, "\pLoading...");
 	RgnHandle rgn = NewRgn();
 	SetRectRgn(rgn, 0,0, 498, 382);
 	UpdateWindow(rgn);
 	DisposeRgn(rgn);
 	
 	MakeCurrent(window, gCurrentFile);
 	return 0;
	}

const char *GetString(const char *p, CString& store)
	{
	const char *sbegin = strchr(p, '"');
	if (!sbegin)
		ERROREXIT("Bad input.txt");
	const char *send = strchr(sbegin+1, '"');
	if (!send)
		ERROREXIT("Bad input.txt");
		
	store = CString(sbegin+1, send - sbegin - 1);
	
	return send+1;
	}
	
void MakeCurrent(WindowPtr window, CFileRecord *r)
	{
	CFileRecord *old = gCurrentFile;
	
	if (old != NULL && old->m_formathtml)
		{
		POSITION walk = old->m_formathtml->GetFirstFormatItem();
		while(walk)
			{
			CFormatItem *f = old->m_formathtml->GetNextFormatItem(walk);
			f->OnShow(FALSE);
			}
		}
		
	if (!r->m_formathtml)
		{
		CParseHTML *parse = r->m_parsehtml;
		
		r->m_parsehtml = NULL;
		
		if (!parse)
			{
			parse = new CParseHTML;
			try {
				parse->LoadFile(r->m_filename);
				}
			catch( CViewException e )
				{
				delete parse;
				throw;
				}
			}
		
		CFormatHTML *format = new CFormatHTML;
		try
			{
			FORMAT_PARAMS fp;
			fp.window_width = gItemRect.right - gItemRect.left;
			format->Format( *parse, fp);
			}
		catch( CViewException e )
			{
			delete format;
			delete parse;
			throw;
			}
			
		r->m_parsehtml = parse;
		r->m_formathtml = format;
		}
	gCurrentFile = r;
	
	
	POSITION walk = gCurrentFile->m_formathtml->GetFirstFormatItem();
	while(walk)
		{
		CFormatItem *f = gCurrentFile->m_formathtml->GetNextFormatItem(walk);
		f->OnShow(TRUE);
		} 
		
	Rect w;
	
	w.left = 0;
	w.top = 55;
	w.right = gWindowRect.right;
	w.bottom = gItemRect.top;
	
	EraseRect(&gItemRect);
	EraseRect(&w);
	
	InvalRect(&gWindowRect);
	InvalRect(&gButPictRect);
	
	gYOffset = 0;
	gMaxYOffset = (gCurrentFile->m_formathtml->GetHeight() - gPageSize);
	if (gMaxYOffset < 0)
		gMaxYOffset = 0;

	Str255 buffer;
	buffer[0] =  gCurrentFile->m_title.GetLength();
	memcpy(buffer+1, (const char *)gCurrentFile->m_title, buffer[0]);
	SetWTitle(window, buffer);
	
	SetCtlMin(scrollbar_control, 1);
	if (gMaxYOffset > 0)
		SetCtlMax(scrollbar_control, SCROLL_SIZE);
	else
		SetCtlMax(scrollbar_control, 1);
		
	SetCtlValue(scrollbar_control, 1);
	
	if (gCurrentFile->m_formathtml->GetFirstHeadingPos() == NULL)
		HiliteControl(heading_control, 255);
	else
		HiliteControl(heading_control, 0);
	}

int ParseInputFile()
	{
	FILE * f = fopen("input.txt", "rt");
	char buffer[256];

	CFileRecord *filerecord = NULL;	
	
	buffer[0] = 0;
	
	while(fgets(buffer, 256, f))
		{
		if (strlen(buffer) < 3)
			continue;		// ignore lines that are too short
	
		if (strmatch(buffer, "file=")==0)
			{
			filerecord = new CFileRecord;				
			GetString(buffer+5, filerecord->m_filename);
			}
		else if (strmatch(buffer, "title=")==0)
			{
			GetString(buffer+6, filerecord->m_title);
			}
		else if (strmatch(buffer, "hdr1=")==0)
			{
			GetString(buffer+5, filerecord->m_hdr1);
			}
		else if (strmatch(buffer, "hdr2=")==0)
			{
			GetString(buffer+5, filerecord->m_hdr2);
			}
		else if (strmatch(buffer, "hdr3=")==0)
			{
			GetString(buffer+5, filerecord->m_hdr3);
			}
		else if (strmatch(buffer, "attach=")==0)
			{
			const char *p = buffer+7;
			while(1)
				{
				skipspaces(&p);
				if (!*p)
					break;
				CString a;
				p = GetString(p, a);
				filerecord->m_attach.Add(a);
				}
			}
		else
			{
			strcat(buffer, "is a bad line in input.txt");
			ERROREXIT(buffer);
			}
		}
	if (!filerecord)
		ERROREXIT("Input.txt is empty");
	fclose(f);
	return 0;
	}

	
pascal void ScrollProc(ControlRef theControl, short partCode)
	{
	WindowPtr window;
	
	window = (**theControl).contrlOwner;
	
	RgnHandle rgn = NewRgn();
	
	switch( partCode )
		{
		case inPageDown:
			Scroll(theControl, gYOffset + gPageSize, rgn, TRUE );
			break;
		case inPageUp:
			Scroll(theControl, gYOffset -gPageSize, rgn, TRUE );
			break;
		case inDownButton:
			Scroll(theControl, gYOffset + 15, rgn, TRUE );
			break;
		case inUpButton:
			Scroll(theControl, gYOffset - 15, rgn, TRUE );
			break;
			}
	UpdateWindow(rgn);
	DisposeRgn(rgn);
	}

void HandleMouseDown( EventRecord *eventPtr)
	{
	WindowPtr window;
	short thePart;
	Point thePoint;
	ControlHandle theControl;
	long newsize;
	Rect sizerect;
	CFileRecord *walk;
	
	thePart = FindWindow( eventPtr->where, &window);
	switch(thePart)
		{
		case inSysWindow:
			SystemClick( eventPtr, window );
			break;
		case inDrag:
			DragWindow( window, eventPtr->where, &qd.screenBits.bounds);
			break;
		case inGrow:
			sizerect.top = 382;
			sizerect.bottom = 10000;
			sizerect.left = 498;
			sizerect.right = 10000;
			newsize = GrowWindow(window, eventPtr->where, &sizerect);
			if (newsize == 0)
				break;
			SizeWindow( window, LoWord(newsize), HiWord(newsize), TRUE);
	
			gWindowRect.right = LoWord(newsize);
			gWindowRect.bottom = HiWord(newsize);
			RecalcLayout(window);
			
			
			// remove all 
			walk = CFileRecord::m_first;
			while(walk)
				{
				if (walk->m_formathtml)
					{
					delete walk->m_formathtml;
					walk->m_formathtml = NULL;
					}	
				walk = walk->m_next;
				}
			try
				{
				MakeCurrent(window, gCurrentFile);
				}
			catch( CViewException e )
				{
				ErrorExit("Could not re-format file");
				}
			break;
		case inContent:
			thePoint = eventPtr->where;
			GlobalToLocal( &thePoint );
			if (PtInButton(gButPictRect.left + 8, gButPictRect.top,thePoint))
				{
				if (!gCurrentFile->m_prev || !TrackButtonDown(gButPictRect.left + 8, gButPictRect.top))
					break;
				try 
					{		
					MakeCurrent(window, gCurrentFile->m_prev);
					}
				catch( CViewException e )
					{
					char buffer[256];
					e.FormatErrorString(buffer);
					ShowError(buffer);
					
					break;		// couldn't load file
					}
				cur_item_number --;
				}
			else if (PtInButton(gButPictRect.left +8, gButPictRect.top + 20,thePoint))
				{
				if (!gCurrentFile->m_next || !TrackButtonDown(gButPictRect.left + 8,gButPictRect.top + 20))
					break;		
				try 
					{
					MakeCurrent(window, gCurrentFile->m_next);
					}
				catch( CViewException e )
					{
					char buffer[256];
					e.FormatErrorString(buffer);
					ShowError(buffer);
					
					break;		// couldn't load file
					}
				cur_item_number ++;
				}
			else if (PtInButton(gButPictRect.left + 8, gButPictRect.top + 40, thePoint))
				{
				if (TrackButtonDown(gButPictRect.left + 8, gButPictRect.top + 40))
					gDone = TRUE;
				}
			else if ( (thePart = FindControl( thePoint, window, &theControl )) != 0)
				{
				switch ((*theControl)->contrlRfCon)
					{
					case 1:
						if (thePart == inThumb)
							{
							TrackThumb( theControl, thePoint );
							}
						else 	 
							thePart = TrackControl( theControl, thePoint, pScrollActionUPP);
						break;
					case 2:
	
						TrackAttachmentsPopup(theControl);
						break;	
					case 3:
						TrackOverviewPopup(theControl, thePoint);
						break;
					case 4:
						TrackHeadingPopup(theControl, thePoint);
						break;
					}
				}
			else
				{
				// maybe the user clicked on a hotlink
				CRect point_rect( thePoint.h - gItemRect.left, thePoint.v - gItemRect.top + gYOffset,
								  thePoint.h - gItemRect.left, thePoint.v - gItemRect.top + gYOffset);
		
				// find all items that intersect this point			
				CPtrList itemlist;			
				gCurrentFile->m_formathtml->GetFormatList(itemlist, point_rect);
				
				if (itemlist.GetCount() >0)
					{
					CFormatItem *fi = (CFormatItem *)itemlist.GetHead();
					fi->OnClick( point_rect.left, point_rect.top);
					}
				}
			break;
		case inGoAway:
			if (TrackGoAway(window, eventPtr->where) == TRUE)
				gDone = true;
			break;
		}
	}

void DoEvent( EventRecord *eventPtr)
	{
	WindowPtr window;
	RgnHandle rgn;
	switch(eventPtr->what)
		{
		case mouseDown:
			HandleMouseDown( eventPtr);
			break;
		case updateEvt:
			window = (WindowPtr)eventPtr->message;
			BeginUpdate(window);
			DrawControls(window);
			rgn = NewRgn();
			GetClip(rgn);
			UpdateWindow(rgn);
			DisposeRgn(rgn);
			EndUpdate(window);
			break;
		}
	}	

void EventLoop(void)
	{
	EventRecord event;
	gDone = false;
	
	BOOL hand_cursor = FALSE;
	Point last_point = {0,0};
	
	Handle h = (Handle)GetCursor(128);
	Cursor cursor_res;
	
	HLock(h);
	memcpy(&cursor_res, *h, sizeof(cursor_res));
	HUnlock(h);
	
	while(gDone ==  false)
		{
		if (WaitNextEvent( everyEvent, &event, 0x7ffffff, nil) )
			DoEvent(&event);
		MoviesTask(nil, 100);
		
		Point thePoint;
		GetMouse( &thePoint );
		
		if (last_point.h == thePoint.h &&
			last_point.v == thePoint.v)
			continue;
		
		// maybe the user has the mouse over a hotlink
		CRect point_rect( thePoint.h - gItemRect.left, thePoint.v - gItemRect.top + gYOffset,
						  thePoint.h - gItemRect.left, thePoint.v - gItemRect.top + gYOffset);

		// find all items that intersect this point			
		CPtrList itemlist;			
		gCurrentFile->m_formathtml->GetFormatList(itemlist, point_rect);
		
		BOOL hotlink = FALSE;
		
		POSITION walk = itemlist.GetHeadPosition();
		while(walk)
			{
			CFormatItem *fi = (CFormatItem *)itemlist.GetNext(walk);
			
			if (fi->IsHotlink())
				hotlink = TRUE;
			}
	
		if (hand_cursor == hotlink)
			continue;		// already in desired state
	
		if (hotlink)
			{
			SetCursor( &cursor_res  );
			}
		else
			{
			SetCursor( &qd.arrow );
			}
			
		hand_cursor = hotlink;
		}
	}

BOOL PtInButton(int x,int y, Point p)
	{
	Rect r;
	r.left = x;
	r.top  = y;
	r.right = x + 16;
	r.bottom = y + 16;
	return PtInRect(p, &r);
	}
	
int TrackHeadingPopup(ControlHandle theControl, Point thePoint)
	{
	long result;
	Point point;
	
	thePoint;
	
	Rect rect = (*theControl)->contrlRect;
	rect.left += 1;
	rect.top += 1;
	rect.bottom -= 2;
	rect.right -= 2;
	//InvertRect(&rect);
	
	point.h = (*theControl)->contrlRect.left + 1;
	point.v = (*theControl)->contrlRect.bottom - 2;
	LocalToGlobal(&point);
	
	MenuHandle heading_menu = NewMenu(MENU_HEADINGS_CONTENT, "\pTempMenu");
	POSITION walk = gCurrentFile->m_formathtml->GetFirstHeadingPos();
	int count = 1;
	while(walk)
		{
		const CHeading * h = gCurrentFile->m_formathtml->GetNextHeading(walk);
		Str255 text;
		CString t = CString(' ', h->m_heading_level - 1) + h->m_heading_name;	
		
		t.GetPString(text);
		InsMenuItem(heading_menu,"\pTemp", count);
		SetItem(heading_menu,count, text);
		count++;
		}
	InsertMenu(heading_menu, hierMenu);
	result = PopUpMenuSelect(heading_menu, point.v, point.h, 0);
	DeleteMenu(MENU_HEADINGS_CONTENT);
	DisposeMenu(heading_menu);	
	
	if (HiWord(result) == 0)
		return -1;
		
		
	int sel = LoWord(result);
	
	walk = gCurrentFile->m_formathtml->GetFirstHeadingPos();
	count = 1;
	while(walk)
		{
		const CHeading * h = gCurrentFile->m_formathtml->GetNextHeading(walk);
		if (count == sel)
			{	
			RgnHandle rgn = NewRgn();
			Scroll(scrollbar_control, h->m_y_offset, rgn, TRUE);
			UpdateWindow(rgn);
			DisposeRgn(rgn);
			return 0;
			}
		count++;
		}
	return 0;
	}
	
int TrackOverviewPopup(ControlHandle theControl, Point thePoint)
	{
	BOOL changed;
	
	DelMenuItem(overview_menu,1);	

	SetCtlValue(theControl, cur_item_number);
	
	CFileRecord *file;
	
	if (TrackControl( theControl, thePoint, (ControlActionUPP)-1))
		{
		file = CFileRecord::m_first;
		int i,ilen = GetCtlValue(theControl);
		for(i=1;i<ilen;i++)
			{
			file = file->m_next;
			ASSERT(file);
			}
		cur_item_number = ilen;
		if (file == gCurrentFile)
			changed = FALSE;
		else
			changed = TRUE;
		}
	else
		changed = FALSE;
	
	InsMenuItem(overview_menu, overview_title, 0);	
	SetCtlValue(theControl, 1);
	RgnHandle region = NewRgn();
	OpenRgn();
	FrameRect( &(*theControl)->contrlRect );
	CloseRgn(region);
	UpdateWindow(region);
	DisposeRgn(region);
	if (changed)
		{
		try 
			{
			MakeCurrent((*theControl)->contrlOwner, file);
			}
		catch( CViewException e )
			{
			char buffer[256];
			e.FormatErrorString(buffer);
			ShowError(buffer);
			}
		}
	return 0;
	}
int TrackAttachmentsPopup(ControlHandle theControl)
	{
	Point point;
	long result;
	
	Rect rect = (*theControl)->contrlRect;
	rect.left += 1;
	rect.top += 1;
	rect.bottom -= 2;
	rect.right -= 2;
	//InvertRect(&rect);
	
	point.h = (*theControl)->contrlRect.left + 1;
	point.v = (*theControl)->contrlRect.bottom - 2;
	LocalToGlobal(&point);
	
	MenuHandle menu = NewMenu(MENU_ATTACHMENTS_CONTENT, "\pTempMenu");
	int i,ilen = gCurrentFile->m_attach.GetSize();
	for(i=0;i<ilen;i++)
		{
		Str255 buffer;
		gCurrentFile->m_attach[i].GetPString(buffer);		
		InsertMenuItem(menu, buffer, i+1);
		}		
	InsertMenu(menu, hierMenu);
	result = PopUpMenuSelect(menu, point.v, point.h, 1);
	DeleteMenu(MENU_ATTACHMENTS_CONTENT);
	DisposeMenu(menu);	
	
	if (HiWord(result) == 0)
		return -1;
	else
		return LoWord(result);
	}

void TrackThumb(ControlHandle theControl, Point thePoint)
	{
	Point last_mouse;
	Rect contrlRect = (*theControl)->contrlRect;
	
	last_mouse = thePoint;
	
	int button_width = (contrlRect.right - contrlRect.left);
	int start_y = contrlRect.top + (button_width*3/2);

	int pixrange = contrlRect.bottom - contrlRect.top - 
			button_width *3;
	
	long original_y = gYOffset;
	
	while( Button() )
		{
		Point cur_mouse;
		GetMouse(&cur_mouse);
		
		if (cur_mouse.h == last_mouse.h && cur_mouse.v == last_mouse.v)
			continue;		// don't do anything when mouse isn't moving
			
		long p;
		
		// only scroll when the mouse is within the scrollbar
		if (cur_mouse.h >= contrlRect.left && cur_mouse.h < contrlRect.right)
			p = ((long)(cur_mouse.v - start_y)) * gMaxYOffset / pixrange;
		else
			p = original_y;
		
		if (p < 0)
			p = 0;
		if (p > gMaxYOffset)
			p = gMaxYOffset;
		
		RgnHandle rgn = NewRgn();
		Scroll(theControl, p, rgn, TRUE);
		UpdateWindow(rgn);
		DisposeRgn(rgn);
		
		MoviesTask(nil, 2000);
		}
	}


BOOL TrackButtonDown(int x,int y)
	{
	PicHandle down = GetPicture(131);
	PicHandle up = GetPicture(132);
	
	Rect r;
	r.left = x -2;
	r.top  = y -2;
	r.right = x + 18;
	r.bottom = y + 18;
	
	DrawPicture(down, &r);
	BOOL mouse_in = TRUE;
	while(Button())
		{
		Point point;
		GetMouse(&point);
		
		if (PtInRect(point,&r))
			{
			if (!mouse_in)
				{
				DrawPicture(down, &r);
				mouse_in = TRUE;
				}
			}
		else
			{
			if (mouse_in)
				{
				DrawPicture(up, &r);
				mouse_in = FALSE;
				}
			}
		}
	DrawPicture(up, &r);
	return mouse_in;
	}


void RecalcLayout(WindowPtr window)
	{
	Rect vScrollRect;
	
	gItemRect.bottom = gWindowRect.bottom - 29;
	gItemRect.right = gWindowRect.right - 128;
	
	gPageSize = gItemRect.bottom - gItemRect.top;
	
	EraseRect( &gWindowRect );

	vScrollRect.left =  gItemRect.right+2;
	vScrollRect.top  = gItemRect.top - 1;
	vScrollRect.right = gItemRect.right + 18;
	vScrollRect.bottom = gItemRect.bottom + 1;
	
	MoveControl(scrollbar_control, vScrollRect.left, vScrollRect.top);
	SizeControl(scrollbar_control, vScrollRect.right - vScrollRect.left, vScrollRect.bottom - vScrollRect.top);
	
	MoveControl(attach_control, 10, gWindowRect.bottom - 24);
	MoveControl(overview_control, 140, gWindowRect.bottom - 24);
	MoveControl(heading_control, 260, gWindowRect.bottom - 24);
	
	gButPictRect.left = gWindowRect.right - 111;
	gButPictRect.top  = gWindowRect.bottom - 86;
	gButPictRect.right = gWindowRect.right;
	gButPictRect.bottom = gWindowRect.bottom;
	
	InvalRect( &gWindowRect );
	}

CFileRecord::CFileRecord()
		{
		m_formathtml = NULL;
		m_parsehtml = NULL;
		if (m_first)
			{
			m_prev = m_last;
			m_next = NULL;
			m_last->m_next = this;
			m_last = this;
			}
		else
			{
			m_prev = NULL;
			m_next = NULL;
			m_first = this;
			m_last = this;
			}
		}
		
CFileRecord::~CFileRecord()
		{
		if (m_formathtml)
			delete m_formathtml;
		if (m_parsehtml)
			delete m_parsehtml;
		/* don't need to worry about removing from list, since they all get deleted at the
		   same time */
		}
		

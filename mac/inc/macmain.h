#define _MACMAIN_H_

#ifndef _FORMATHTML_H_
#include "formathtml.h"
#endif

/* resource definitions */

#define PICT_TITLE					128
#define	PICT_DISABLE_PREV	 		129
#define PICT_DISABLE_NEXT			130
#define PICT_BUTTON_DOWN			131
#define PICT_BUTTON_UP				132
#define PICT_CLOSE_WINDOW			133
#define PICT_ENABLE_PREV			134
#define PICT_ENABLE_NEXT			135

#define MENU_ATTACHMENTS			128
#define MENU_OVERVIEW				129
#define MENU_HEADINGS				130
#define MENU_ATTACHMENTS_CONTENT	200
#define MENU_HEADINGS_CONTENT		201

#define FIXED_FONT 	courier
#define PROP_FONT newYork

void UpdateWindow(RgnHandle update_rgn);
void Scroll(ControlHandle ctrl, long amount, RgnHandle update_rgn, BOOL setcontrol);
void LaunchNetscape(const char *URL);

class CFileRecord
	{
public:
	CFileRecord();
	~CFileRecord();

	CString m_filename, m_title, m_hdr1, m_hdr2, m_hdr3;
	CStringArray m_attach;
	CFormatHTML *m_formathtml;
	CParseHTML *m_parsehtml;
	
static CFileRecord *m_first, *m_last;
	CFileRecord *m_prev, *m_next;
	};
	
// number of ticks on the scroll bar (equal to the number of pixels on the scroll bar)
#define SCROLL_SIZE 200

// global variables
extern Rect	gWindowRect;
extern Rect	gItemRect;
extern Rect gTopPictRect;
extern Rect	gButPictRect;
extern long gYOffset, gMaxYOffset, gPageSize;

extern CFileRecord *gCurrentFile;

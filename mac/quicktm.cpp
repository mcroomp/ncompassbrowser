#include "cross_p.h"

#include "MacMain.h"
#include "quicktime.h"

CFormatQuickTimeItem::CFormatQuickTimeItem( LPCSTR filename, BOOL has_controller, const CRect& extent)
	: CFormatItem( ITEM_QUICKTIME, extent, 0)
	{
	short err, movieResFile;
	FSSpec fspec;
	
	Str255 buffer;
	buffer[0] = strlen(filename);
	memcpy(buffer+1, filename, buffer[0]);
	
	if (FSMakeFSSpec(0, 0, buffer, &fspec))
		{
		m_broken = TRUE;
		return;
		}
	
	err = OpenMovieFile( &fspec, &movieResFile, fsRdPerm);
	if (err == noErr)
		{
		short movieResID = 0;
		Str255 moviename;
		Boolean waschanged;
		
		NewMovieFromFile(&m_movie, movieResFile, &movieResID, moviename, newMovieActive, &waschanged);
		CloseMovieFile( movieResFile );
		m_broken = FALSE;
		
		Rect rect;
		GetMovieBox(m_movie, &rect);
		
		if (extent.right == 0)
			m_extent.right = rect.right - rect.left;
		if (extent.bottom == 0)
			m_extent.bottom = rect.bottom - rect.top;	
		}
	else
		{
		m_movie = nil;
		m_broken = TRUE;
		}
	}
	
CFormatQuickTimeItem::~CFormatQuickTimeItem()	
	{
	if (!m_broken)
		DisposeMovie(m_movie);
	}

void CFormatQuickTimeItem::OnRedraw(const char *plain_text, BOOL hilight)
	{	
	UpdateMovie(m_movie);
	MoviesTask(m_movie, 1000);
	
	Rect r;

	r.left = m_extent.left + gItemRect.left;
	r.top = m_extent.top + gItemRect.top  - gYOffset;
	r.right = m_extent.right + gItemRect.left;
	r.bottom = m_extent.bottom + gItemRect.top - gYOffset;
	
	ForeColor(blackColor);
	
	PenSize(1,1);
	FrameRect(&r);
	}
	
void CFormatQuickTimeItem::OnClick(long x,long y)
	{
	
	}
	
void CFormatQuickTimeItem::OnScroll()
	{
	if (!m_broken)
		{
		Rect r;
		
		r.left = m_extent.left + gItemRect.left;
		r.top = m_extent.top + gItemRect.top - gYOffset;
		r.right = m_extent.right + gItemRect.left;
		r.bottom = m_extent.bottom + gItemRect.top - gYOffset;
			
		SetMovieBox(m_movie, &r);	
		}
	}
	
void CFormatQuickTimeItem::OnShow(BOOL flag)
	{
	if (m_broken)
		return;
		
	if (flag)
		{
		RgnHandle rgn = NewRgn();
		SetRectRgn(rgn, gItemRect.left, gItemRect.top, gItemRect.right, gItemRect.bottom);
		
		SetMovieDisplayClipRgn(m_movie, rgn);
		
		Rect r;
		r.left = m_extent.left + gItemRect.left;
		r.top = m_extent.top + gItemRect.top + gYOffset;
		r.right = m_extent.right + gItemRect.left;
		r.bottom = m_extent.bottom + gItemRect.top + gYOffset;
		
		SetMovieBox(m_movie, &r);
		SetMovieGWorld(m_movie,  (CGrafPtr)qd.thePort, nil);
		GoToBeginningOfMovie(m_movie);
		TimeBase timebase = GetMovieTimeBase(m_movie);
		SetTimeBaseFlags(timebase, loopTimeBase );
		
		StartMovie(m_movie);
		
		
		DisposeRgn(rgn);
		}
	else
		{
		StopMovie(m_movie);
		// give non-visible movie a NULL clip region
		RgnHandle rgn = NewRgn();
		SetMovieDisplayClipRgn(m_movie, rgn);
		DisposeRgn(rgn);
		}
	}


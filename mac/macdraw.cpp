#include "cross_p.h"

#include "MacMain.h"

static long	fontsizetbl[8] =
	{ 0,		
	  8,		// fontsize 1
	  10,		//			2
	  12,		//			3
	  14,		//			4
	  16,		//			5
	  18,		//			6
	  24 };		//			7
	  
	
CFormatTextItem::CFormatTextItem(long font_size, long font_descent, long text_ofs, long text_len, long color,
	 long flags, const CRect& extent) : CFormatItem(ITEM_TEXT, extent, font_descent)
	{
	m_font_size = font_size;
	m_text_ofs = text_ofs;
	m_text_len = text_len;
	m_color = color;
	m_flags = flags;
	}	
	
void CFormatTextItem::OnRedraw(BOOL hilight, REDRAW_PARAMS& params)
	{
	if (m_flags & FORMAT_FIXEDFONT)
		TextFont(FIXED_FONT);
	else
		TextFont(PROP_FONT);
		
	Style st = 0;
	if (m_flags & FORMAT_BOLD)
		st |= bold;
	if (m_flags & FORMAT_ITALIC)
		st |= italic;
	if (m_flags & FORMAT_UNDERLINE)
		st |= underline;
	
	ForeColor(blackColor);		
		
	TextFace(st);	
	TextSize(fontsizetbl[m_font_size]);
	FontInfo fi;
	GetFontInfo(&fi);
	MoveTo(m_extent.left + gItemRect.left , m_extent.top + gItemRect.top + fi.ascent - gYOffset);	
	DrawText(params.plain_text + m_text_ofs , 0, m_text_len);	
	}

CFormatHotlinkTextItem::CFormatHotlinkTextItem(long font_size, long font_descent, long text_ofs, long text_len, long color, long flags, const CRect& extent, LPCSTR hotlink)
	: CFormatTextItem(font_size, font_descent, text_ofs, text_len, color, flags, extent)
	{
	m_item_type = ITEM_HOTLINK_TEXT;
	
	m_hotlink.m_parent = this;
	m_hotlink.m_url = hotlink;
	m_hotlink.m_next_part = NULL;
	m_hotlink.m_prev_part = NULL;
	}

void CFormatHotlinkTextItem::OnRedraw(BOOL hilight, REDRAW_PARAMS& params)
	{
	if (m_flags & FORMAT_FIXEDFONT)
		TextFont(FIXED_FONT);
	else
		TextFont(PROP_FONT);
		
	Style st = 0;
	if (m_flags & FORMAT_BOLD)
		st |= bold;
	if (m_flags & FORMAT_ITALIC)
		st |= italic;
	
	st |= underline;
	if (hilight)
		ForeColor(redColor);
	else
		ForeColor(blueColor);
		
	TextFace(st);	
	TextSize(fontsizetbl[m_font_size]);
	FontInfo fi;
	GetFontInfo(&fi);
	MoveTo(m_extent.left + gItemRect.left , m_extent.top + gItemRect.top + fi.ascent - gYOffset);	
	DrawText(params.plain_text + m_text_ofs , 0, m_text_len);	
	}

void CFormatHotlinkTextItem::OnClick(long x, long y)
	{
	m_hotlink.OnClick(x,y);
	}
	
void CHotlink::OnClick(long x, long y)
	{
	RgnHandle save_rgn = NewRgn();
	RgnHandle item_rgn = NewRgn();
	
	SetRectRgn(item_rgn, gItemRect.left, gItemRect.top, gItemRect.right, gItemRect.bottom);
	
	GetClip(save_rgn);
	SetClip(item_rgn);

	CHotlink *start_walk = this, *walk;
	while(start_walk->m_prev_part)
		start_walk = start_walk->m_prev_part;
		
	walk = start_walk;
	
	CBigString plain_text = gCurrentFile->m_formathtml->GetPlainText();
	
	plain_text.Lock();
	
	const char *p_plain_text = plain_text.GetBuffer();
	
	REDRAW_PARAMS rparams;
	
	rparams.plain_text = p_plain_text;
	
	// highlight the hotlink items
	while(walk)
		{
		walk->m_parent->OnRedraw(TRUE, rparams);
	
		walk = walk->m_next_part;
		}
	while(Button())
		;
	
	walk = start_walk;
	
	// re-invert the hotlink items
	while(walk)
		{
		walk->m_parent->OnRedraw(FALSE, rparams);
	
		walk = walk->m_next_part;
		}
	
	plain_text.Unlock();
	
	SetClip(save_rgn);
	
	LaunchNetscape(m_url);
	}

CFormatPictureItem::CFormatPictureItem(const CInlinePicture& p, const CRect& extent)
	: CFormatItem(ITEM_PICTURE, extent,0) , m_picture(p)
	{
	CRect r = p.GetBoundRect();
	if (extent.right == 0)
		m_extent.right = r.right;
		
	if (extent.bottom == 0)
		m_extent.bottom = r.bottom;	
	}

void CFormatPictureItem::OnRedraw( BOOL hilight, REDRAW_PARAMS& rparams )
	{
	CRect r;
	
	r.left = m_extent.left + gItemRect.left;
	r.top = m_extent.top + gItemRect.top  - gYOffset;
	r.right = m_extent.right + gItemRect.left;
	r.bottom = m_extent.bottom + gItemRect.top - gYOffset;
	
	RGBColor bk;
	bk.red = 0xffff;
	bk.green = 0xffff;
	bk.blue = 0xcccc;
	
	DRAW_PICTURE_PARAMS dparams;
	
	dparams.extent = r;
	dparams.backcolor = bk;
	
	m_picture.DrawPicture( dparams );
	}

CFormatHotlinkPictureItem::CFormatHotlinkPictureItem(const CInlinePicture& p, const CRect& extent, LPCSTR url, long border_size)
	: CFormatPictureItem(p, extent) 
	{
	m_border_size = border_size;
	m_hotlink.m_parent = this;
	m_hotlink.m_url = url;
	m_hotlink.m_next_part = NULL;
	m_hotlink.m_prev_part = NULL;
	m_item_type = ITEM_HOTLINK_PICTURE;
	
	if (extent.right == 0)
		m_extent.right += m_border_size * 2;
	if (extent.bottom == 0)
		m_extent.bottom += m_border_size * 2;	
	}

void CFormatHotlinkPictureItem::OnClick(long x, long y)
	{
	m_hotlink.OnClick(x,y);
	}

void CFormatHotlinkPictureItem::OnRedraw(BOOL hilight, REDRAW_PARAMS& rparams)
	{
	CRect r;
	
	r.left = m_extent.left + gItemRect.left + m_border_size;
	r.top = m_extent.top + gItemRect.top  - gYOffset + m_border_size;
	r.right = m_extent.right + gItemRect.left - m_border_size;
	r.bottom = m_extent.bottom + gItemRect.top - gYOffset - m_border_size;
	
	RGBColor bk;
	bk.red = 0xffff;
	bk.green = 0xffff;
	bk.blue = 0xcccc;
	
	DRAW_PICTURE_PARAMS dparams;
	
	dparams.extent = r;
	dparams.backcolor = bk;

	m_picture.DrawPicture( dparams );
	
	if (m_border_size > 0)
		{
		if (hilight)
			ForeColor(redColor);
		else
			ForeColor(blueColor);
		
		Rect frame;
		
		frame.left = r.left - m_border_size;
		frame.top = r.top - m_border_size;
		frame.right = r.right + m_border_size;
		frame.bottom = r.bottom + m_border_size;
	
		PenSize(m_border_size,m_border_size);
	
		FrameRect(&frame);
		}
	}

CFormatRuleItem::CFormatRuleItem(const CRect& extent)
	: CFormatItem(ITEM_RULE, extent, 0)
	{
	}
	
void CFormatRuleItem::OnRedraw( BOOL hilight, REDRAW_PARAMS& params)
	{
	Rect r;
	
	r.left = m_extent.left + gItemRect.left;
	r.right = m_extent.right + gItemRect.left;
	r.top = m_extent.top + gItemRect.top  - gYOffset;
	r.bottom = m_extent.bottom + gItemRect.top - gYOffset;
	
	RGBColor color;
	color.red = color.green = color.blue = 187*256;
	RGBForeColor(&color);
	PaintRect(&r);
		
	PenSize(1,1);
	
	color.red = color.green = color.blue = 0xffff;
	RGBForeColor(&color);
	MoveTo( r.right, r.top );
	LineTo( r.left, r.top);
	LineTo( r.left, r.bottom);
	
	color.red = color.green = color.blue = 221*256 ;
	RGBForeColor(&color);
	
	MoveTo( r.right-1, r.top+1);
	LineTo( r.left+1, r.top+1);
	LineTo( r.left+1, r.bottom-1);
	
	color.red = color.green = color.blue = 136*256;
	RGBForeColor(&color);
	MoveTo( r.right, r.top );
	LineTo( r.right, r.bottom);
	LineTo( r.left, r.bottom);
	
	color.red = color.green = color.blue =  170*256;
	RGBForeColor(&color);
	
	MoveTo( r.right-1, r.top+1);
	LineTo( r.right-1,r.bottom-1);
	LineTo( r.left+1, r.bottom-1);
	}

void UpdateWindow( RgnHandle update_rgn)
	{
	RgnHandle save_rgn = NewRgn();
	GetClip(save_rgn);
	
	SetClip(update_rgn);
		
	if (RectInRgn(&gTopPictRect, update_rgn))
		{
		PicHandle picture = GetPicture(PICT_TITLE);
		DrawPicture(picture, &gTopPictRect);
		}
	if (RectInRgn(&gTopPictRect, update_rgn))
		{
		/* draw the three buttons */
		
		PicHandle picture;
		Rect prect;
		
		prect.left = gButPictRect.left; prect.right = gButPictRect.right;
		
		if (!gCurrentFile->m_prev)
			picture = GetPicture(PICT_DISABLE_PREV);
		else
			picture = GetPicture(PICT_ENABLE_PREV);
			
		prect.top = gButPictRect.top; prect.bottom = gButPictRect.top+16;
		DrawPicture(picture, &prect);
		
		if (!gCurrentFile->m_next)
			picture = GetPicture(PICT_DISABLE_NEXT);
		else
			picture = GetPicture(PICT_ENABLE_NEXT);
			
		prect.top = gButPictRect.top + 20; prect.bottom = gButPictRect.top + 20 + 16;
		DrawPicture(picture, &prect);		
		
		picture = GetPicture(PICT_CLOSE_WINDOW);
		prect.top = gButPictRect.top + 40; prect.bottom = gButPictRect.top + 40 + 16;
		DrawPicture(picture, &prect);
		}
	
	Rect sizebox;
	sizebox.top = gWindowRect.bottom - 15;
	sizebox.bottom = gWindowRect.bottom;
	sizebox.left = gWindowRect.right - 15;
	sizebox.right = gWindowRect.right;
	if (RectInRgn(&gTopPictRect, update_rgn))
		{
		ClipRect( &sizebox);
		DrawGrowIcon( (WindowPtr)qd.thePort );
		SetClip(update_rgn);
		}
		
	ForeColor(blackColor);
	
	MoveTo(499,42);
	LineTo( gWindowRect.right, 42);
	PenSize(2,2);
	/*
	MoveTo(0, 353);
	LineTo(387, 353);
	MoveTo(387, 74);
	LineTo(387, 382);
	*/
	
	if (!gCurrentFile->m_formathtml)	
		{
		SetClip(save_rgn);
		DisposeRgn(save_rgn);
		return;
		}
		
	/* Draw the three header lines */
	
	TextFont(helvetica);
	TextFace(bold);	
	TextSize(12);
	FontInfo fi;
	GetFontInfo(&fi);
	
	MoveTo(gItemRect.left, 55 + fi.ascent);	
	DrawText(gCurrentFile->m_hdr1 , 0, gCurrentFile->m_hdr1.GetLength());
	MoveTo(gItemRect.left, 55 + 13 + fi.ascent);	
	DrawText(gCurrentFile->m_hdr2 , 0, gCurrentFile->m_hdr2.GetLength());
	MoveTo(gItemRect.left, 55 + 26 + fi.ascent);	
	DrawText(gCurrentFile->m_hdr3 , 0, gCurrentFile->m_hdr3.GetLength());
	
	Rect frame;
	frame.left = gItemRect.left -1;
	frame.top = gItemRect.top -1;
	frame.right = gItemRect.right + 1;
	frame.bottom = gItemRect.bottom + 1;					
	
	RgnHandle rect_rgn = NewRgn();
	SetRectRgn(rect_rgn, gItemRect.left, gItemRect.top, gItemRect.right, gItemRect.bottom);
	SectRgn(update_rgn, rect_rgn, rect_rgn);
	
	SetClip(rect_rgn);
	
 	Rect updateRect = (**rect_rgn).rgnBBox;
	
	CPtrList itemlist;
	CRect _updateRect;
	_updateRect.SetRect(updateRect.left - gItemRect.left , updateRect.top - gItemRect.top  + gYOffset, 
					updateRect.right - gItemRect.left , updateRect.bottom - gItemRect.top  + gYOffset);
					
	CBigString plain_text = gCurrentFile->m_formathtml->GetPlainText();
	
	plain_text.Lock();
	
	const char *p_plain_text = plain_text.GetBuffer();
	
	REDRAW_PARAMS rparams;
	
	rparams.plain_text = p_plain_text;
	
	gCurrentFile->m_formathtml->GetFormatList(itemlist, _updateRect);
	POSITION walk = itemlist.GetHeadPosition();
	while(walk)
		{
		CFormatItem *item = (CFormatItem *)itemlist.GetNext(walk);
		
		item->OnRedraw( FALSE, rparams );
		} 
	
	plain_text.Unlock();
	
	MoviesTask(nil, 1000);
	
	SetClip(save_rgn);
		
	DisposeRgn(save_rgn);
	DisposeRgn(rect_rgn);
	}

#define MEASURE_MAX 100

void CFormatHTML::GetFontHeight(long& fontsize, long& baseline)
	{
	short save_font = qd.thePort->txFont;
	Style save_face = qd.thePort->txFace;
	short save_size = qd.thePort->txSize;
	
	if (m_font_type > 0)
		TextFont(FIXED_FONT);
	else
		TextFont(PROP_FONT);
		
	Style st = 0;
	if (m_bold > 0)
		st |= bold;
	if (m_italic > 0)
		st |= italic;
	if (m_underline > 0)
		st |= underline;		
		
	TextFace(st);	
	TextSize(fontsizetbl[m_font_size]);
	FontInfo fi;
	GetFontInfo(&fi);
	
	fontsize = fi.ascent + fi.descent;
	baseline = fi.descent;

	TextFace(save_face);
	TextSize(save_size);
	TextFont(save_font);
	}

BOOL CFormatHTML::GetLineBreak(const char *text, long maxlen, long &store_len, long& store_width, 
	long& y_ascent, long& y_descent)
	{
static short width_tbl[MEASURE_MAX+1];
	
	short save_font = qd.thePort->txFont;
	Style save_face = qd.thePort->txFace;
	short save_size = qd.thePort->txSize;
	
	if (m_font_type > 0)
		TextFont(FIXED_FONT);
	else
		TextFont(PROP_FONT);
		
	Style st = 0;
	if (m_bold > 0)
		st |= bold;
	if (m_italic > 0)
		st |= italic;
	if (m_underline > 0)
		st |= underline;		
		
	TextFace(st);	
	TextSize( fontsizetbl[m_font_size]);
	FontInfo fi;
	GetFontInfo(&fi);
	
	y_ascent = fi.ascent;
	y_descent = fi.descent;
	
	BOOL retval;
	long len;
	long maxwidth = m_right_margin - m_cur_x;

	if (maxwidth < 0)
		{
		ASSERT(!m_line_empty);
		
		//refuse to put anything more on this line since it is already full
		store_len = 0;
		store_width = 0;
		retval = TRUE;
		}

	// limit the amount of text we will search
	if (maxlen > MEASURE_MAX)
		len = MEASURE_MAX;
	else
		len = maxlen;
	
	MeasureText(len, text, width_tbl);
	
	if (width_tbl[len] <= maxwidth)
		{
		store_len = len;
		store_width = width_tbl[len];
		retval = FALSE;
		goto exit;
		}
		
	while(len > 0 && width_tbl[len] > maxwidth)
		{
		len--;
		}		
	
	for(long i=len-1;i>=0;i--)
		{
		if ( *(text+i) == ' ')
			{
			store_len = i;
			store_width = width_tbl[i];
			retval = TRUE;
			goto exit;
			}
		}
	
	if (m_line_empty)
		{
		// we're on a blank line, so we *have* to get some text on this line	
		store_len = len;
		store_width = width_tbl[len];
		retval = TRUE;
		}
	else
		{
		//refuse to put anything more on this line
		store_len = 0;
		store_width = 0;
		retval = TRUE;
		}	
exit:
	TextFace(save_face);
	TextSize(save_size);
	TextFont(save_font);
	return retval;
	}
	
void Scroll(ControlHandle ctrl, long target, RgnHandle update_rgn, BOOL setcontrol)
	{
// to make sure we don't scroll too far
	if (target < 0)
		target = 0;
	else if (target > gMaxYOffset )
		target = gMaxYOffset;
	
	if (setcontrol)
		SetCtlValue( ctrl, target * SCROLL_SIZE / gMaxYOffset);
	
	long shift = gYOffset - target;
	
	if ( shift > gPageSize || shift < -gPageSize)
		{
		SetRectRgn(update_rgn, gItemRect.left, gItemRect.top, gItemRect.right, gItemRect.bottom);
		EraseRect(&gItemRect);
		}
	else
		{
		ScrollRect(&gItemRect, 0, shift, update_rgn);
		} 
	if (target != gYOffset)
		{
		gYOffset = target;
		
		POSITION walk = gCurrentFile->m_formathtml->GetFirstFormatItem();
		while(walk)
			{
			CFormatItem *f = gCurrentFile->m_formathtml->GetNextFormatItem(walk);
			f->OnScroll();
			}
		}
	}
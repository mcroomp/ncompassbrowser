#include "cross_p.h"

#include "fmthtml.h"


class CTmpFormatItem
	{
public:
	CTmpFormatItem( CFormatHTML *format_html, INT32 width, INT32 height, INT32 ascent, ALIGN_TYPE align, BOOL canbreak);
	virtual ~CTmpFormatItem();

	CFormatHTML * m_format_html;

	INT32 m_ascent;
	INT32 m_height;				// height of this thing
	BOOL m_can_break;			// TRUE if the item can be broken up into two smaller items
	INT32 m_width;
	ALIGN_TYPE m_align;

	virtual BOOL IsEmpty();		// returns TRUE if the item is empty and will take up no horizonal space
	virtual void AddToContainer(CFormatFrame *frame, CRect extent)  = 0;
	virtual CTmpFormatItem *Break( INT32 request_width, BOOL force);
	};

class CTmpFormatConditionalBreakItem : public CTmpFormatItem
	{
public:
	CTmpFormatConditionalBreakItem( CFormatHTML *format_html, BOOL canbreak = TRUE );

	virtual BOOL IsEmpty();
	virtual void AddToContainer(CFormatFrame *frame, CRect extent);
	virtual CTmpFormatItem *Break( INT32 request_width, BOOL force);
	};

class CTmpFormatBulletItem : public CTmpFormatItem
	{
public:
	CTmpFormatBulletItem( CFormatHTML *format_html, BULLET_TYPE bullet, INT32 numeric );

	BULLET_TYPE m_bullet_type;
	INT32 m_bullet_numeric;

	virtual void AddToContainer( CFormatFrame *frame, CRect extent);
	};

class CTmpFormatTextItem : public CTmpFormatItem
	{
public:
	CTmpFormatTextItem(CHotlink *hotlink, INT32 textstart, INT32 textsize, INT32 flags, CFormatHTML *html);

	INT32 m_textstart, m_textsize, m_flags;
	CHotlink *m_hotlink;

	virtual BOOL IsEmpty();
	virtual void AddToContainer( CFormatFrame *frame, CRect extent);
	virtual CTmpFormatItem *Break( INT32 request_width, BOOL force);
	};

class CTmpFormatPictureItem : public CTmpFormatItem
	{
public:
	CTmpFormatPictureItem( CHotlink *hotlink, INT32 bordersize, BOOL is_mapped, LPCSTR url, CSize requested_size, CFormatHTML *html, ALIGN_TYPE valign);

	CString m_url;
	CHotlink *m_hotlink;
	INT32 m_bordersize;
	BOOL m_is_mapped;

	virtual void AddToContainer( CFormatFrame *frame, CRect extent);
	};

#ifdef USES_OLE_CONTROLS
class CTmpFormatOLEControlItem : public CTmpFormatItem
	{
public:
	CTmpFormatOLEControlItem( const GUID& guid, LPCSTR ocx_url, INT32 parseid, LPCSTR version, CDynLoadMemFile *memfile, CFormatHTML *html, INT32 width, INT32 height, ALIGN_TYPE valign );

	CDynLoadMemFile *m_memfile;
	INT32 m_parseid;
	CLSID m_clsid;
	CString m_ocx_url;
	CString m_version;

	virtual void AddToContainer( CFormatFrame *frame, CRect extent);
	};
#endif
	
CTmpFormatItem::CTmpFormatItem( CFormatHTML *html, INT32 width, INT32 height, INT32 ascent, ALIGN_TYPE align, BOOL canbreak)
	{
	m_format_html = html;
	m_ascent = ascent;
	m_align = align;
	m_height = height;
	m_can_break = canbreak;
	m_width = width;
	}

CTmpFormatItem::~CTmpFormatItem()
	{
	}

BOOL CTmpFormatItem::IsEmpty()
	{
	return FALSE;
	}

CTmpFormatItem *CTmpFormatItem::Break( INT32 request_width, BOOL force)
	{
	return NULL;
	}

CTmpFormatConditionalBreakItem::CTmpFormatConditionalBreakItem( CFormatHTML *format_html, BOOL canbreak )
	: CTmpFormatItem( format_html, 0, 0, 0, ALIGN_BASELINE, canbreak )
	{
	}

void CTmpFormatConditionalBreakItem::AddToContainer(CFormatFrame *frame, CRect extent)
	{
	// do nothing
	}

BOOL CTmpFormatConditionalBreakItem::IsEmpty()
	{
	return !m_can_break;
	}

CTmpFormatItem *CTmpFormatConditionalBreakItem::Break( INT32 request_width, BOOL force)
	{
	if (!m_can_break)
		return NULL;
	else
		return new CTmpFormatConditionalBreakItem( m_format_html, FALSE );
	}

CTmpFormatBulletItem::CTmpFormatBulletItem( CFormatHTML *format_html, BULLET_TYPE bullet, INT32 numeric )
	: CTmpFormatItem( format_html, format_html->m_tab_size, 
			bullet == BULLET_NUMERIC ? format_html->m_format_font->GetAscent(3) + format_html->m_format_font->GetDescent(3) : 8,
			bullet == BULLET_NUMERIC ? format_html->m_format_font->GetAscent(3) : 8,
			ALIGN_BASELINE, FALSE)
	{
	m_bullet_type = bullet;
	m_bullet_numeric = numeric;
	}

void CTmpFormatBulletItem::AddToContainer( CFormatFrame *frame, CRect extent)
	{
	CFormatItem *i;

	if (m_bullet_type != BULLET_NUMERIC)
		{
		INT32 w = (extent.Width() - extent.Height())/2;
		extent.left += w;
		extent.right = extent.left + extent.Height();
		}

	i = new (m_format_html->m_format_items) CFormatBulletItem( m_bullet_type, m_bullet_numeric,
		extent, frame );

	if (m_format_html->m_first_new_item == NULL)
		m_format_html->m_first_new_item = i;
	}


CTmpFormatTextItem::CTmpFormatTextItem( CHotlink *hotlink, INT32 textstart, INT32 textsize, INT32 flags, CFormatHTML *html)
	: CTmpFormatItem( html, 0,0,0, ALIGN_BASELINE, (flags & FONTFLAG_NO_WRAP) ? FALSE : TRUE)
	{
	m_ascent = m_format_html->m_format_font->GetAscent(flags);
	m_height = m_ascent + m_format_html->m_format_font->GetDescent(flags);
	m_flags = flags;
	m_textstart = textstart;
	m_textsize = textsize;
	m_hotlink = hotlink;

	CString tstring;
	m_format_html->m_plain_text.GetString(tstring, m_textstart, m_textsize);

	m_width = m_format_html->m_format_font->GetStringWidth(flags, tstring, m_textsize );
	}

BOOL CTmpFormatTextItem::IsEmpty()
	{
	return m_textsize == 0;
	}

void CTmpFormatTextItem::AddToContainer( CFormatFrame *frame, CRect extent )
	{
	if (m_textsize == 0)
		return;
	
	CFormatItem *i;

	if (m_hotlink)
		{
		i = new(m_format_html->m_format_items) CFormatHotlinkTextItem(m_hotlink, m_textstart, m_textsize, m_flags, extent, frame);
		m_hotlink->AddItem(i);
		}
	else
		{
		i = new(m_format_html->m_format_items) CFormatTextItem( m_textstart, m_textsize, m_flags, extent, frame);
		}
	if (m_format_html->m_first_new_item == NULL)
		m_format_html->m_first_new_item = i;
	}

CTmpFormatItem *CTmpFormatTextItem::Break( INT32 request_width, BOOL force )
	{
	CString str;

	if ( (m_flags & FONTFLAG_NO_WRAP) || m_textsize == 0)
		return NULL;		// can't split an empty string

	m_format_html->m_plain_text.GetString(str, m_textstart, m_textsize);

	INT32 numchr;

	if (request_width != -1)
		{
		numchr = m_format_html->m_format_font->GetLineBreak( m_flags, force, str,
			m_textsize, request_width);
		}
	else
		{
		numchr = m_textsize;
		}

	if (force)
		{
		CTmpFormatTextItem *item = DEBUG_NEW CTmpFormatTextItem(m_hotlink, m_textstart + numchr, m_textsize - numchr, m_flags, m_format_html );

		m_textsize = numchr;
		m_width = m_format_html->m_format_font->GetStringWidth( m_flags, str, numchr);
		return item;
		}

	if (numchr == 0)
		return NULL;			// string is too short to fit in this space

	while(numchr > 0)
		{
		if (str[numchr-1] == ' ')
			{
			// tack space on the end of the current item
			CTmpFormatTextItem *item = DEBUG_NEW CTmpFormatTextItem( m_hotlink, m_textstart + numchr, m_textsize - numchr, m_flags, m_format_html );

			m_textsize = numchr;
			m_width = m_format_html->m_format_font->GetStringWidth( m_flags, str, numchr);
			return item;
			}
		numchr--;
		}
	return NULL;
	}

CTmpFormatPictureItem::CTmpFormatPictureItem( CHotlink *hotlink, INT32 bordersize, BOOL is_mapped, LPCSTR url, CSize requested_size, CFormatHTML *html, ALIGN_TYPE valign )
	: CTmpFormatItem( html, 0, 0, 0, valign, FALSE)	
	{
	m_url = url;
	m_bordersize = bordersize;

	m_height = requested_size.cy;
	m_width = requested_size.cx;

	m_hotlink = hotlink;
	m_ascent = m_height;
	m_is_mapped = is_mapped;
	}

void CTmpFormatPictureItem::AddToContainer(CFormatFrame *frame, CRect extent )
	{
	CFormatItem *i;

	if (m_hotlink)
		{
		i = new(m_format_html->m_format_items) CFormatHotlinkPictureItem( m_hotlink, m_bordersize, m_is_mapped, m_url, extent, frame);
		}
	else
		{
		i = new(m_format_html->m_format_items) CFormatPictureItem( m_url, extent, frame);
		}
	if (m_format_html->m_first_new_item == NULL)
		m_format_html->m_first_new_item = i;
	}

#ifdef USES_OLE_CONTROLS
CTmpFormatOLEControlItem::CTmpFormatOLEControlItem(const CLSID& clsid, LPCSTR ocx_url, INT32 parseid, LPCSTR version, CDynLoadMemFile *memfile, CFormatHTML *html, INT32 width, INT32 height, ALIGN_TYPE valign )
	: CTmpFormatItem( html, width, height, height, valign, FALSE)
	{
	m_memfile = memfile;
	m_parseid = parseid;
	m_clsid = clsid;
	m_ocx_url = ocx_url;
	m_version = version;
	}

void CTmpFormatOLEControlItem::AddToContainer(CFormatFrame *frame, CRect extent )
	{
	CFormatOLEControlItem *i;

	i = new(m_format_html->m_format_items) CFormatOLEControlItem( m_clsid, m_ocx_url, m_parseid, m_version, m_memfile, extent, frame);

	m_format_html->m_control_list.AddTail(i);
	
	if (m_format_html->m_first_new_item == NULL)
		m_format_html->m_first_new_item = i;
	}	

#endif

BOOL IntersectRect(const CRect& r1, const CRect& r2)
	{
	return (r1.left < r2.right && r1.top < r2.bottom &&
			r1.right > r2.left && r1.bottom > r2.top);
	}

CFormatItem::~CFormatItem()
	{
	}

CRect CFormatItem::GetExtent() const
	{
	CRect extent(m_extent);

	CFormatFrame *frame = m_frame_parent;
	while(frame)
		{
		CPoint p = frame->GetOrigin();

		extent.left += p.x;
		extent.top += p.y;
		extent.right += p.x;
		extent.bottom += p.y;

		frame = frame->GetParent();
		}
	return extent;
	}

LPCSTR CFormatItem::GetPicture() const
	{
	return NULL;
	}
		
void CFormatItem::OnRedraw(REDRAW_PARAMS & rp)
	{
	}
	
void CFormatItem::OnScroll()
	{
	}

void CFormatItem::OnShow(BOOL flag)
	{
	}

void CFormatItem::OnClick(INT32 x, INT32 y)
	{
	}
	
CFormatItem::CFormatItem(ITEM_TYPE item_type, const CRect& extent, CFormatFrame *frame) : m_extent(extent)
	{
	m_item_type = item_type;
	m_frame_parent = frame;
	}
	
CHotlink * CFormatItem::GetHotlink() const
	{
	return NULL;
	}

CHotlink::CHotlink(LPCSTR url)
	{
	m_url = url;
	}

CHotlink *CFormatHotlinkTextItem::GetHotlink() const
	{
	return m_hotlink;
	}

CHotlink *CFormatHotlinkPictureItem::GetHotlink() const
	{
	return m_hotlink;
	}


CFormatFrame::CFormatFrame( CFormatHTML *format_html, CFormatFrame *parent, CPoint origin, CSize recommended_size ,
	INT32 left_border, INT32 top_border, INT32 right_border, INT32 bottom_border)
	{
	m_origin = origin;
	m_recommended_size = recommended_size;

	m_actual_size.cx = 0;
	m_actual_size.cy = top_border;

	m_left_border = left_border;
	m_right_border = right_border;
	m_top_border = top_border;
	m_bottom_border = bottom_border;


	m_format_html = format_html;
	m_parent = parent;

	m_line_empty = TRUE;

	m_cur_y = top_border;
	m_left_margin = left_border; 
	m_right_margin = m_recommended_size.cx - right_border;

	m_center_count = 0;

	m_previous_align = m_align = ALIGN_LEFT;
	m_indent = 0;
	m_first_indent = 0;
	m_first_line = TRUE;

	m_space_after_paragraph = 0;		
	m_max_y_ascent = 0;
	m_max_y_descent = 0;
	m_amount_on_line = 0;

	m_empty_lines = 100;

	m_table_inside = 0;

	RecalcMargins();
	}

CSize CFormatFrame::GetActualSize() const
	{
	return CSize( m_actual_size.cx + m_left_border + m_right_border, m_actual_size.cy + m_bottom_border );
	}

void CFormatFrame::StretchActualSizeX( INT32 size )
	{
	size += (m_left_margin - m_left_border);
	
	if (size > m_actual_size.cx)
		m_actual_size.cx = size;
	}

void CFormatFrame::StretchActualSizeY( INT32 size )
	{
	if (size > m_actual_size.cy)
		m_actual_size.cy = size;
	}


CFormatFrame::~CFormatFrame()
	{
	INT32 i, ilen = m_table_current_row.GetSize();
	for(i=0;i<ilen;i++)
		{
		delete (CFormatTableColumn *)m_table_current_row[i];
		}
	m_table_current_row.RemoveAll();

	m_indent_stack.RemoveAll();

	ilen = m_current_line.GetSize();
	for(i=0;i<ilen;i++)
		{
		delete (CTmpFormatItem *)m_current_line[i];
		}
	m_current_line.RemoveAll();

	ilen = m_contained_frames.GetSize();
	for(i=0;i<ilen;i++)
		{
		delete (CFormatFrame *)m_contained_frames[i];
		}
	m_contained_frames.RemoveAll();

	POSITION walk = m_cur_align_left_list.GetHeadPosition();
	while(walk)
		{
		CFormatFrame *p = (CFormatFrame *)m_cur_align_left_list.GetNext(walk);
		delete p;
		}
	m_cur_align_left_list.RemoveAll();

	walk = m_cur_align_right_list.GetHeadPosition();
	while(walk)
		{
		CFormatFrame *p = (CFormatFrame *)m_cur_align_right_list.GetNext(walk);
		delete p;
		}
	m_cur_align_right_list.RemoveAll();
	}

#define MIN_COLUMN_WIDTH 30


void CFormatFrame::EndTableRow(BOOL lastrow)
	{
	if (m_table_current_frame)
		{
		m_table_current_frame->BreakLine( break_soft );
		Assign_Max(m_table_max_y, m_table_current_frame->GetActualSize().cy);
		}
	

	INT32 i, ilen = m_table_current_row.GetSize(), x, colspan = 0;
	x = m_left_margin;

	m_table_max_y = 0;

	// Find the maximum y size of the row
	for(i=0;i<ilen;i++)
		{
		CFormatTableColumn *column = (CFormatTableColumn *)m_table_current_row[i];
	
		if (column->m_frame && column->m_row_span_left == 0)
			Assign_Max(m_table_max_y, column->m_frame->GetActualSize().cy - column->m_row_span_height);
		}

	if (m_table_max_y == 0)
		{
		m_table_max_y = m_format_html->m_format_font->GetAscent(m_format_html->m_font_flags) + m_format_html->m_format_font->GetDescent(m_format_html->m_font_flags);
		}

	for(i=0;i<ilen;i++)
		{
		CFormatTableColumn *column = (CFormatTableColumn *)m_table_current_row[i];

		if (colspan == 0)
			{
			if (column->m_frame)
				{
				if (lastrow)
					column->m_row_span_left = 0;	// last row cannot span columns

				if (column->m_row_span_left > 0)
					{
					column->m_row_span_height += m_table_max_y + m_table_border_width;
					}
				else
					{
					CFormatFrame *frame = column->m_frame;
					INT32 y_adjust = 0;
					INT32 max_y = m_table_max_y + column->m_row_span_height;

					switch(column->m_valign)
						{
						case ALIGN_CENTER:
						case ALIGN_MIDDLE:
							y_adjust = (max_y - frame->GetActualSize().cy)/2;
							break;
						case ALIGN_BOTTOM:
							y_adjust = (max_y - frame->GetActualSize().cy);
							break;
						case ALIGN_TOP:
							y_adjust = 0;
							break;
						}
					CPoint p =frame->GetOrigin();
					frame->MoveFrame( CPoint( p.x, p.y + y_adjust ) );
					column->m_frame = NULL;					
					}
				colspan = column->m_col_span;
				}
			else
				{
				colspan = 1;
				}

			if (m_table_border_width > 0)
				{
				new (m_format_html->m_format_items) CFormatRuleItem( 
					CRect( x, m_cur_y, x + m_table_border_width, m_cur_y + m_table_max_y + m_table_border_width), this);
				}
			}
		
		x += m_table_border_width + column->m_width;
		colspan--;
		}								

	if (m_table_border_width > 0)
		{
		// final border
		new (m_format_html->m_format_items) CFormatRuleItem( 
			CRect( x, m_cur_y, x + m_table_border_width, m_cur_y + m_table_max_y + m_table_border_width), this);
		}

	m_cur_y += m_table_max_y;
				

	ilen = m_table_current_row.GetSize();
	x = m_left_margin + m_table_border_width;
	colspan = 0;

	BOOL at_bottom = FALSE;

	for(i=0;i<ilen;i++)
		{
		CFormatTableColumn *column = (CFormatTableColumn *)m_table_current_row[i];
		INT32 width = column->m_width + m_table_border_width;

		if (colspan == 0)
			{
			if (column->m_row_span_left == 0)
				{
				at_bottom = TRUE;
				}
			else
				{
				at_bottom = FALSE;
				column->m_row_span_left--;
				}

			colspan = column->m_col_span;					
			}

		if (m_table_border_width > 0 &&	at_bottom)
			{
			new (m_format_html->m_format_items) CFormatRuleItem( 
				CRect( x, m_cur_y, x + width, m_cur_y + m_table_border_width ), this);
			}

		colspan--;
		x += width;
		}
	m_cur_y += m_table_border_width;
	}

// returns FALSE if the formatting should abort due to an unknown image or table size
// or TRUE if it should continue

BOOL CFormatFrame::ApplyTag( const CTag *ptag, FORMAT_PARAMS &fp)
	{	
	if (m_table_inside)
		{		
// continue table 
		if (m_table_inside > 1)
			{
			// keep track of tables within tables
			if (ptag->m_type == TAG_TABLE)
				m_table_inside++;
			else if (ptag->m_type == (TAG_TABLE|TAG_END) )
				m_table_inside--;					   

			return m_table_current_frame->ApplyTag( ptag, fp );
			}
		else
			{
			const CTagTableCell *ptagcell = (const CTagTableCell *)ptag;
			const CTagTableRow *ptagrow = (const CTagTableRow *)ptag;
			CFormatTableColumn *column;
			INT32 i,ilen, width;
			

			switch(ptag->m_type)
				{
				case TAG_TABLECELL:
					if (m_table_current_frame)
						{
						// empty out current frame
						column = (CFormatTableColumn *)m_table_current_row[m_table_current_column-1];
						
						m_table_current_frame->BreakLine( break_soft );
						m_table_x_offset += m_table_current_frame->m_recommended_size.cx +
							m_table_border_width;
						}

					// Find the first free column 
					while(TRUE)
						{
						column = (CFormatTableColumn *)m_table_current_row[m_table_current_column];
						if (column->m_frame == NULL)
							break;

						ilen = column->m_col_span;
						for (i=0;i<ilen;i++)
							{
							column = (CFormatTableColumn *)m_table_current_row[m_table_current_column+i];
							m_table_x_offset += column->m_width + m_table_border_width;
							}
						m_table_current_column += ilen;
						}

					width = 0;
					ilen = ptagcell->m_col_span;
					for(i=0;i<ilen;i++)
						{
						CFormatTableColumn * t= (CFormatTableColumn *)m_table_current_row[m_table_current_column + i];
						width += t->m_width;
						}
					width += (ilen-1)*m_table_border_width;

					m_table_current_frame = new CFormatFrame( m_format_html, this, 
						CPoint( m_table_x_offset, m_cur_y),
						CSize( width, 0),
						m_table_cell_padding,m_table_cell_padding,m_table_cell_padding,m_table_cell_padding);

					m_table_current_frame->m_align = ptagcell->m_align;
					
					column->m_frame = m_table_current_frame;
					column->m_valign =  ptagcell->m_valign;
					column->m_col_span = ptagcell->m_col_span;
					column->m_row_span_left = ptagcell->m_row_span-1;
					column->m_row_span_height = 0;

					m_contained_frames.Add( m_table_current_frame );
					m_table_current_column += ptagcell->m_col_span;
					break;
				case TAG_TABLEROW:
					if (!m_table_first_row)
						EndTableRow(FALSE);
					else
						m_table_first_row = FALSE;

					m_table_x_offset = m_left_margin + m_table_border_width;
					m_table_current_column = 0;					
					m_table_max_y = 0;
					m_table_current_frame = NULL;
					break;
				case TAG_TABLE|TAG_END:
						{
						EndTableRow(TRUE);
					
						INT32 i, ilen = m_table_current_row.GetSize();
						for(i=0;i<ilen;i++)
							{
							delete (CFormatTableColumn *)m_table_current_row[i];
							}
						m_table_current_row.RemoveAll();
						m_table_inside = 0;
						m_empty_lines = 0;
						// establish space after table
						BreakLine(break_hard);

						RecalcMargins();

						StretchActualSizeY( m_cur_y );
						}
					break;
				case TAG_TABLE:
					if (!m_table_current_frame->ApplyTag( ptag, fp ))
						return FALSE;
					m_table_inside++;
					break;
				default:
					if (m_table_current_frame)
						{
						if (!m_table_current_frame->ApplyTag( ptag, fp ))
							return FALSE;
						}
					break;
				}
			}
		return TRUE;
		}
		
// Ignore tag if it was a global formatting change (like bold or italics)
	if ( m_format_html->ApplyTag( ptag, fp ) )
		return TRUE;

	// helper pointers (not necessarily valid)
	const CTagINT32 *ptagint32 = (const CTagINT32 *)ptag;
	const CTagCString *ptagcstring = (const CTagCString *)ptag;
	const CTagImage * ptagimage = (const CTagImage *)ptag;
	const CTagTable * ptagtable = (const CTagTable *)ptag;
	const CTagBeginParagraph * ptagpara = (const CTagBeginParagraph *)ptag;
	const CTagBullet *pbullet = (const CTagBullet *)ptag;
	
#ifdef USES_OLE_CONTROLS
	const CTagOLEControl * ptagole = (const CTagOLEControl *)ptag;
#endif
		
	switch(ptag->m_type)
		{
		case TAG_CENTER:
			m_center_count++;
			if (m_center_count > 0)
				{
				m_previous_align = m_align = ALIGN_CENTER;
				}
			break;
		case TAG_CENTER|TAG_END:
			m_center_count--;
			if (m_center_count == 0)
				{
				m_previous_align = m_align = ALIGN_LEFT;
				}
			break;
		case TAG_PARAGRAPH:
			while(m_empty_lines < ptagpara->m_space_before)
				BreakLine(break_hard);

			if (ptagpara->m_align != ALIGN_NONE)
				{
				m_previous_align = m_align;
				m_align = ptagpara->m_align;
				}

			m_space_after_paragraph = ptagpara->m_space_after;
			m_indent = ptagpara->m_indent_amount * m_format_html->m_tab_size;
			m_first_indent = ptagpara->m_indent_first_line * m_format_html->m_tab_size;
			m_first_line = TRUE;
			RecalcMargins();
			break;
		case TAG_PARAGRAPH|TAG_END:
			while(m_empty_lines < m_space_after_paragraph)
				BreakLine(break_hard);
		
			m_first_indent = 0;
			m_indent = 0;
			m_space_after_paragraph = 1;
			m_first_line = TRUE;

			m_align = m_previous_align;
			
			RecalcMargins();
			break;

#ifdef USES_OLE_CONTROLS
		case TAG_OLECONTROL:
#endif
		case TAG_IMAGE:
				{
				INT32 hspace,vspace, bordersize;
				CSize picture_size;
				ALIGN_TYPE align;

#ifdef USES_OLE_CONTROLS
				CDynLoadMemFile *memfile;
#endif
				
				if (ptag->m_type  == TAG_IMAGE)
					{
					bordersize = ptagimage->m_border;

					if (ptagimage->m_height && ptagimage->m_width)
						{
						picture_size.cy = ptagimage->m_height;
						picture_size.cx = ptagimage->m_width;
						}
					else
						{
						if (!fp.m_picture_info.GetPictureSize(ptagimage->m_url, picture_size))
							return FALSE;
						}

					if (fp.m_pDC->IsPrinting() )
						{
						// strech the bitmap to be 72dpi when printing
						picture_size.cx = picture_size.cx * fp.m_pDC->GetDeviceCaps(  LOGPIXELSX ) / 72;
						picture_size.cy = picture_size.cy * fp.m_pDC->GetDeviceCaps(  LOGPIXELSY ) / 72;
						bordersize = bordersize * fp.m_pDC->GetDeviceCaps(  LOGPIXELSX ) / 72; 
						}

					if (m_format_html->m_hotlink)
						{
						picture_size.cx += bordersize*2;
						picture_size.cy += bordersize*2;
						}

					hspace = ptagimage->m_hspace;
					vspace = ptagimage->m_vspace;

					align = ptagimage->m_align ;
					}

#ifdef USES_OLE_CONTROLS
				else if (ptag->m_type == TAG_OLECONTROL)
					{
					if (!ptagole->m_url.IsEmpty())
						{
						memfile = new CDynLoadMemFile(fp.m_notify_object,ptagole->m_url, METHOD_GET);
						memfile->StartLoading();
						}
					else
						memfile = NULL;
				
					ASSERT( ptagole->m_height > 0);
					ASSERT( ptagole->m_width > 0);

					picture_size.cx = ptagole->m_width;
					picture_size.cy = ptagole->m_height;

					align = ptagole->m_align ;						
					hspace = ptagole->m_hspace;
					vspace = ptagole->m_vspace;
					bordersize = 0;
					}
#endif				
				CTmpFormatItem *titem;
				ALIGN_TYPE titem_align;

				if (align == ALIGN_LEFT ||
					align == ALIGN_RIGHT)
					{
					titem_align = ALIGN_BASELINE;
					}
				else
					{
					titem_align = align;
					}

				if (ptag->m_type == TAG_IMAGE)
					{
					// the picture is inline
					titem = DEBUG_NEW CTmpFormatPictureItem( (m_format_html->m_hotlink ? m_format_html->m_current_hotlink : NULL), ptagimage->m_border, ptagimage->m_is_mapped, ptagimage->m_url, picture_size, m_format_html, titem_align ) ;
					}
#ifdef USES_OLE_CONTROLS
				else if (ptag->m_type == TAG_OLECONTROL)
					{
					titem = DEBUG_NEW CTmpFormatOLEControlItem(ptagole->m_clsid, ptagole->m_ocx_url, ptagole->m_parse_id, ptagole->m_version, memfile, m_format_html, picture_size.cx, picture_size.cy,  titem_align);
					}
#endif
				
				if (align == ALIGN_LEFT)
					{
					CFormatFrame *frame = new CFormatFrame( m_format_html, this, CPoint(0,0), CSize(picture_size.cx + hspace,picture_size.cy + 2*vspace),
						0, vspace, hspace, vspace );
					m_new_align_left_list.AddTail( frame );
					frame->AddTmpFormatItem( titem );
					frame->BreakLine( CFormatFrame::break_soft);
					RecalcMargins();
					}
				else if (align == ALIGN_RIGHT)
					{
					CFormatFrame *frame = new CFormatFrame( m_format_html, this, CPoint(0,0), CSize(picture_size.cx + hspace,picture_size.cy + 2*vspace),
						hspace, vspace, 0, vspace);
					m_new_align_right_list.AddTail( frame );
					frame->AddTmpFormatItem( titem );
					frame->BreakLine( break_soft);
					RecalcMargins();
					}
				else
					{
					if (!IsLineEmpty() && ptagimage->m_can_break)
						AddTmpFormatItem( new CTmpFormatConditionalBreakItem(m_format_html) );

					AddTmpFormatItem( titem );

					if (ptagimage->m_can_break)
						AddTmpFormatItem( new CTmpFormatConditionalBreakItem(m_format_html) );
					}
				}
			break;
		case TAG_BULLET:
			AddTmpFormatItem( new CTmpFormatBulletItem( m_format_html, pbullet->m_bullet_type, pbullet->m_bullet_number ) );
			break;
		case TAG_NEWLINE:
			BreakLine( CFormatFrame::break_soft);
			break;
		case TAG_NEWLINE_HARD_BREAK:
			BreakLine( CFormatFrame::break_hard);			
			break;
		case TAG_CONDITIONAL_BREAK:
			AddTmpFormatItem( new CTmpFormatConditionalBreakItem(m_format_html) );					
			break;
		case TAG_NEWLINE_CLEAR:
			BreakLine(break_soft);

			ASSERT(ptagint32->m_int32 > 0 && ptagint32->m_int32 <= 3);
			Clear( (CLEAR_TYPE)ptagint32->m_int32);			// requested a clear on a newline
			break;
		case TAG_NEWLINE_CLEAR_EN:
			BreakLine(break_soft);
			Clear(unit_en, ptagint32->m_int32);			// requested a clear on a newline
			break;
		case TAG_NEWLINE_CLEAR_PIXEL:
			BreakLine(break_soft);
			Clear(unit_pixel, ptagint32->m_int32);			// requested a clear on a newline
			break;
		case TAG_HORZRULE:
				{
				INT32 w = (m_right_margin - m_left_margin) * (100 - ptagint32->m_int32) / 200;

				CFormatItem *item = new(m_format_html->m_format_items) CFormatRuleItem(CRect(m_left_margin + w, m_cur_y, m_right_margin - w, m_cur_y+ 3), this);
				m_cur_y += 3;
				m_empty_lines = 0;

				if (m_format_html->m_first_new_item == NULL)
					m_format_html->m_first_new_item = item;
				}
			break;
		case TAG_TEXT:
				{
				const CTagText *ptagtext = (const CTagText *)ptag;
				m_format_html->m_font_flags = ptagtext->m_font_flags;

				AddTmpFormatItem( DEBUG_NEW CTmpFormatTextItem(m_format_html->m_hotlink ? m_format_html->m_current_hotlink : NULL, ptagtext->m_startpos, ptagtext->m_textlen, m_format_html->m_font_flags, m_format_html));
				}									
			break;
		case TAG_HEADING:
			/*
			This code isn't quite working right...

			if (m_format_html->m_last_heading)
				delete m_format_html->m_last_heading;
			m_format_html->m_last_heading = DEBUG_NEW CHeading;
			m_format_html->m_last_heading->m_y_offset = m_cur_y;
			m_format_html->m_last_heading->m_heading_level = ptagint32->m_int32;*/
			break;
		case TAG_HEADING|TAG_END:
			/*
			if (!m_format_html->m_last_heading)
				break;

			m_format_html->m_plain_text.GetString(m_format_html->m_last_heading->m_heading_name ,
				m_format_html->m_last_heading_pos, m_format_html->m_last_plain_pos - m_format_html->m_last_heading_pos);

			m_format_html->m_headinglist.AddTail(m_format_html->m_last_heading);
			m_format_html->m_last_heading = NULL;	
			*/
			break;
		case TAG_ANCHOR_NAME:
			if (m_format_html->m_last_anchor)
				delete m_format_html->m_last_anchor;
			m_format_html->m_last_anchor = DEBUG_NEW CAnchor;
			m_format_html->m_last_anchor_pos = m_format_html->m_last_plain_pos;
			m_format_html->m_last_anchor->m_y_offset = m_cur_y;
			break;
		case TAG_ANCHOR_NAME|TAG_END:
			if (!m_format_html->m_last_anchor)
				break;

			m_format_html->m_plain_text.GetString(m_format_html->m_last_anchor->m_anchor_name, 
				m_format_html->m_last_anchor_pos, m_format_html->m_last_plain_pos - m_format_html->m_last_anchor_pos);

			m_format_html->m_anchorlist.AddTail(m_format_html->m_last_anchor);
			m_format_html->m_last_anchor = NULL;
			break;
		case TAG_TABLE:
				{
				if (!ptagtable->m_parsed_whole_table)
					{
					return FALSE;		// insist on whole table being parsed before continuing
					}

				// scan forward to make sure all the pictures in this table have been read
				// to the point of knowing how big they are

				INT32 table_count = 1;

				m_format_html->m_parse->DEBUG_LOCK();
				POSITION tagstart = m_format_html->m_parse->FindTagPos( ptag );

				POSITION walk = tagstart;
				
				m_format_html->m_parse->GetNextTag( walk );
				m_format_html->m_parse->Unlock();

				CMapStringToPtr picture_map;

				while(table_count > 0 && walk)
					{
					m_format_html->m_parse->DEBUG_LOCK();
					CTag * scan = (CTag *) m_format_html->m_parse->GetNextTag( walk );
					m_format_html->m_parse->Unlock();
					
					switch (scan->m_type)
						{
						case TAG_TABLE:
							table_count++;
							break;
						case TAG_TABLE|TAG_END:
							table_count--;
							break;
						case TAG_IMAGE:
								{
								CTagImage *ptagimage = (CTagImage *) scan;
								fp.m_picture_info.UpdatePicture(ptagimage->m_url);

								CSize dummy;
	
								if ((!ptagimage->m_width || !ptagimage->m_height) &&
									!fp.m_picture_info.GetPictureSize(ptagimage->m_url, dummy))
									return FALSE;
								}
							break;
						}
					}
				ASSERT( table_count == 0); 
				
				m_table_border_width = ptagtable->m_border_width;

				INT32 i, ilen = ptagtable->m_num_columns;
				if (ilen == 0)
					ilen = 1;

				walk = tagstart;
				
				CInt32Array max_column_widths, min_column_widths;
				INT32 max_table_size, min_table_size;

				GetTableSize( fp, m_format_html,  m_format_html->m_parse, walk, 
					max_column_widths, min_column_widths, min_table_size, max_table_size);

				if (max_table_size <= m_line_width)
					{
					m_table_width = max_table_size;
					m_table_cell_padding = ptagtable->m_cell_padding;
				
					ASSERT( m_table_current_row.GetSize() == 0);

					for(i=0;i<ilen;i++)
						{
						CFormatTableColumn *column = new CFormatTableColumn;
						m_table_current_row.Add( column );

						column->m_width = max_column_widths[i];
						column->m_align = ALIGN_LEFT;
						column->m_valign = ALIGN_TOP;
						column->m_frame = NULL;
						column->m_col_span = 1;
						column->m_row_span_left = 0;
						column->m_row_span_height = 0;					
						}
					}
				else if (min_table_size >= m_line_width)
					{
					m_table_width = min_table_size;
				
					ASSERT( m_table_current_row.GetSize() == 0);

					for(i=0;i<ilen;i++)
						{
						CFormatTableColumn *column = new CFormatTableColumn;
						m_table_current_row.Add( column );

						column->m_width = min_column_widths[i];
						column->m_align = ALIGN_LEFT;
						column->m_valign = ALIGN_TOP;
						column->m_frame = NULL;
						column->m_col_span = 1;
						column->m_row_span_left = 0;
						column->m_row_span_height = 0;					
						}
					}
				else
					{
					INT32 tbl_diff = max_table_size - min_table_size;
					INT32 remain = m_line_width - min_table_size;

					m_table_width = m_line_width;
				
					INT32 table_width = m_table_border_width;

					CFormatTableColumn *want_remainder = NULL;
					/* pointer to a column that can get the remainder of space after it has been
					   given its share of space */
					
					for(i=0;i<ilen;i++)
						{
						CFormatTableColumn *column = new CFormatTableColumn;
						m_table_current_row.Add( column );
					
						INT32 col_diff = max_column_widths[i] - min_column_widths[i];

						column->m_width = min_column_widths[i] + (col_diff * remain / tbl_diff);
						if (column->m_width < max_column_widths[i])
							want_remainder = column;

						table_width += column->m_width + m_table_border_width;
						
						column->m_align = ALIGN_LEFT;
						column->m_valign = ALIGN_TOP;
						column->m_frame = NULL;
						column->m_col_span = 1;
						column->m_row_span_left = 0;
						column->m_row_span_height = 0;					
						}

					want_remainder->m_width += m_line_width - table_width;
					table_width += m_line_width - table_width;
					
					ASSERT( m_line_width == table_width);
					}

				m_table_cell_padding = ptagtable->m_cell_padding;
				
				m_table_current_frame = NULL;
				m_table_inside = 1;
				m_table_max_y = 0;
				m_table_first_row = TRUE;

				// establish space before table
				while(m_empty_lines < 1)
					BreakLine(break_hard);

				// clear until we have enough space to show the table
				Clear( unit_pixel, m_table_width );

				if ( m_center_count > 0 && m_table_width < m_line_width)
					{
					// center the table if requested
					m_left_margin += (m_line_width - m_table_width)/2;
					}

				if (m_table_border_width > 0)
					{
					CFormatItem *item = new (m_format_html->m_format_items) CFormatRuleItem( 
						CRect( m_left_margin, m_cur_y, m_left_margin + m_table_width, m_cur_y + m_table_border_width ), this);

					if (m_format_html->m_first_new_item == NULL)
						m_format_html->m_first_new_item = item;
			
					m_cur_y += m_table_border_width;
					}

				StretchActualSizeX( m_table_width );
				}
			break;
		}
	return TRUE;
	}

void CFormatFrame::MoveFrame( CPoint new_origin )
	{
	m_origin = new_origin;
	}
		
void CFormatFrame::Clear( CLEAR_TYPE clear_type )
	{
	if (clear_type == clear_left ||
		clear_type == clear_all)
		{
		while( !m_cur_align_left_list.IsEmpty() ||
			   !m_new_align_left_list.IsEmpty() ) 			
			{
			BreakLine(break_hard);
			}
		}

	if (clear_type == clear_right ||
		clear_type == clear_all)
		{
		while( !m_cur_align_right_list.IsEmpty() ||
			   !m_new_align_right_list.IsEmpty() ) 			
			{
			BreakLine(break_hard);
			}
		}
	}

void CFormatFrame::Clear( TABLE_UNITS unit, INT32 amount )
	{
	if (unit == unit_en)
		{
		amount = (m_format_html->m_dots_per_inch * amount / 140);
		}

	// amount is now in pixels

	while( (m_right_margin - m_left_margin < amount) &&
		(!m_cur_align_right_list.IsEmpty() ||!m_new_align_right_list.IsEmpty()) )
		{
		BreakLine(break_hard);
		}
	}

void CFormatFrame::BreakLine(BREAK_TYPE breaktype)
	{
	CPtrArray next_line;

	if ( IsLineEmpty() )
		{
		if (breaktype == break_hard)
			{
			m_cur_y += m_format_html->m_format_font->GetAscent(m_format_html->m_font_flags) + m_format_html->m_format_font->GetDescent(m_format_html->m_font_flags);

			StretchActualSizeY(m_cur_y);
			m_first_line = TRUE;
			RecalcMargins();
			m_empty_lines++;
			}
		return;
		}

	do
		{
		INT32 width = 0;

		INT32 i,ilen = m_current_line.GetSize();
		INT32 j,k;

		CTmpFormatItem *tmpf;
		BOOL found_break = FALSE;
		INT32 break_width = 0;
		INT32 break_index = 0;

		for(i=0;i<ilen;i++)
			{
			tmpf = (CTmpFormatItem *)m_current_line[i];
			if (tmpf->m_can_break)
				{
				found_break = TRUE;
				break_index = i;
				break_width = width;
				}

			if (found_break && (width + tmpf->m_width > m_line_width) )
				{
				i = break_index;
				width = break_width;
				tmpf = (CTmpFormatItem *)m_current_line[i];
				break;
				}

			width += tmpf->m_width;
			}

		if (i == ilen)
			{
			if (breaktype == break_lineonly)
				{
				m_amount_on_line = width;
				m_empty_lines = -1;
				return;
				}
			goto shrunk_current_line;		// everything fits...
			}

		if (m_line_width < m_format_html->m_minimum_text_width)
			{
			m_line_width = m_format_html->m_minimum_text_width;
			}
					
		// first pass through.. we break before or after non-breaking items 
		// and non-forced on breaking items

		if (tmpf->m_can_break)
			{
			// most common case... the item that goes over the boundary is breakable
			CTmpFormatItem *newpart = tmpf->Break( m_line_width - width, FALSE);
			
			if (newpart)
				{
				// add the new part and the rest of the line to the next line
				next_line.Add(newpart);
				for(j = i + 1; j < ilen; j++)
					next_line.Add( m_current_line[j] );

				m_current_line.RemoveAt(i+1, ilen - i - 1);		// remove the elements from the current line
				goto shrunk_current_line;
				}
			}

		// last item wouldn't wrap volunterily, so find another volunteer
		for(j = i - 1; j >= 0; j--)
			{
			CTmpFormatItem *p = (CTmpFormatItem *)m_current_line[j];

			if (p->m_can_break == TRUE)
				{
				// see if this item will be kind enough to break
				CTmpFormatItem *newpart = p->Break(-1, FALSE);
				if (newpart)
					{
					// we found someone who would break
					// add the new part and the rest of the line to the next line

					if (newpart->IsEmpty() )
						delete newpart;
					else
						next_line.Add(newpart);

					for(k = j + 1; k < ilen; k++)
						next_line.Add( m_current_line[k] );

					m_current_line.RemoveAt(j+1, ilen - j - 1);		// remove the elements from the current line
					goto shrunk_current_line;
					}
				// otherwise continue in our search
				}
			}

		if (tmpf->m_can_break)
			{
			// we've tried to be nice, it didn't work, so just force the last item to wrap
			CTmpFormatItem *newpart = tmpf->Break( m_line_width - width, TRUE);
			if (newpart != NULL)
				{
				// add the new part and the rest of the line to the next line
				if (newpart->IsEmpty() )
					delete newpart;
				else
					next_line.Add(newpart);

				for(j = i + 1; j < ilen; j++)
					next_line.Add( m_current_line[j] );

				m_current_line.RemoveAt(i+1, ilen - i - 1);		// remove the elements from the current line
				}
			}
		
shrunk_current_line:
		// m_current_line is now sized to fit the current screen, so dump it out
		
		INT32 max_ascent = 0, max_descent = 0;
		width = 0;

		// find width and maximum ascent and descent
		ilen = m_current_line.GetSize();
		for(i=0;i<ilen;i++)
			{
			tmpf = (CTmpFormatItem *)m_current_line[i];

			INT32 ascent, descent;

			switch(tmpf->m_align)
				{
				case ALIGN_TOP:
					ascent = 0;
					descent = tmpf->m_height;
					break;
				case ALIGN_CENTER:
				case ALIGN_MIDDLE:
					ascent = (tmpf->m_height+1)/2;
					descent = tmpf->m_height/2;
					break;
				case ALIGN_BASELINE:
				case ALIGN_BOTTOM:
					ascent = tmpf->m_ascent;
					descent = tmpf->m_height - tmpf->m_ascent;
					break;
				/*case ALIGN_BOTTOM:
					ascent = 0;
					descent = tmpf->m_height;
					break; */
				default:
					ASSERT(FALSE);
				}

			if (ascent > max_ascent)
				max_ascent = ascent;
			if (descent > max_descent )
				max_descent = descent;
			width += tmpf->m_width;
			}

		INT32 x_adjust = 0;
		// align this line
		if (m_align == ALIGN_LEFT)
			x_adjust = 0;	
		else if (m_align == ALIGN_RIGHT) 
			x_adjust = m_line_width - width;
		else if (m_align == ALIGN_CENTER)
			x_adjust = (m_line_width - width)/2;
		else
			{
			TRACE1("Bad m_align (%d)\n", (int)m_align);
			x_adjust = 0;
			}

		INT32 cur_x = m_left_margin + x_adjust;
		// send it to m_formatlist
		for(i=0;i<ilen;i++)
			{
			tmpf = (CTmpFormatItem *)m_current_line[i];

			INT32 top;

			switch(tmpf->m_align)
				{
				case ALIGN_TOP:
					top = 0;
					break;
				case ALIGN_CENTER:
				case ALIGN_MIDDLE:
					top = max_ascent - (tmpf->m_height+1)/2;
					break;
				case ALIGN_BASELINE:
				case ALIGN_BOTTOM:
					top = max_ascent - tmpf->m_ascent;
					break;
				//case ALIGN_BOTTOM:
				//	top = max_ascent + max_descent - tmpf->m_height;
				//	break;
				default:
					ASSERT(FALSE);
				}
			

			tmpf->AddToContainer( this, CRect(cur_x, m_cur_y + top,
					cur_x + tmpf->m_width, m_cur_y + top + tmpf->m_height));
			
			cur_x += tmpf->m_width;
			delete tmpf;
			}

		if (max_ascent == 0 && max_descent == 0)
			{
			m_cur_y += 8;			// increment an arbitrary amount if we didn't fit anything on the line
			}
		else
			m_cur_y += max_ascent + max_descent;

		// adjust actual size to fit the line we are adding

		StretchActualSizeY(m_cur_y);
		StretchActualSizeX( width );
				
		m_current_line.RemoveAt(0, ilen);
		m_first_line = FALSE;
		RecalcMargins();

		m_current_line.InsertAt(0, &next_line);
		next_line.RemoveAll();
		}
	while ( ! IsLineEmpty() );

	m_amount_on_line = 0;
	m_empty_lines = 0;
	if (breaktype == break_hard )
		{
		m_first_line = TRUE;
		RecalcMargins();
		}
	}

void CFormatFrame::RecalcMargins()
	{
	// adjust the margins to take into account any new left or right aligned images
	// or changes in margins/ indents
top:

	m_left_margin = m_left_border; 
	m_right_margin = m_recommended_size.cx - m_right_border;
	
	POSITION walk = m_cur_align_left_list.GetHeadPosition();
	while(walk)
		{
		POSITION oldwalk = walk;
		CFormatFrame *frame = (CFormatFrame *)m_cur_align_left_list.GetNext(walk);
		if (frame->m_origin.y + frame->GetActualSize().cy < m_cur_y)
			{
			m_cur_align_left_list.RemoveAt(oldwalk);
			m_contained_frames.Add(frame);
			goto top;
			}
		else
			{
			m_left_margin = frame->m_origin.x + frame->GetActualSize().cx;
			}
		}

	walk = m_cur_align_right_list.GetHeadPosition();
	while(walk)
		{
		POSITION oldwalk = walk;
		CFormatFrame *frame = (CFormatFrame *)m_cur_align_right_list.GetNext(walk);
		if (frame->m_origin.y + frame->GetActualSize().cy < m_cur_y)
			{
			m_cur_align_right_list.RemoveAt(oldwalk);
			m_contained_frames.Add(frame);
			goto top;
			}
		else
			{
			ASSERT(m_right_margin >= frame->m_origin.x);
			m_right_margin = frame->m_origin.x;
			}
		}

	// add left indent
	m_left_margin += m_indent;
	if (m_first_line)
		m_left_margin += m_first_indent;

	m_line_width = m_right_margin - m_left_margin;

	if (!m_new_align_left_list.IsEmpty())
		{
		CFormatFrame *frame = (CFormatFrame *)m_new_align_left_list.GetHead();

		if (frame->GetActualSize().cx >= m_line_width - m_format_html->m_minimum_text_width)
			{
			if (m_cur_align_right_list.IsEmpty()  &&
				m_cur_align_left_list.IsEmpty() )
				{
				// extend the actual size of this frame to contain the new item
				m_new_align_left_list.RemoveHead();
				
				frame->MoveFrame( CPoint( m_left_margin, m_cur_y) );
				m_contained_frames.Add( frame );

				CSize s = frame->GetActualSize();
				m_cur_y += s.cy;

				StretchActualSizeX( s.cx );
				StretchActualSizeY(m_cur_y);
				goto top;
				}
			// wait until we have more space to put the picture in
			}
		else
			{
			frame->MoveFrame( CPoint( m_left_margin, m_cur_y) );
			m_new_align_left_list.RemoveHead();
			m_cur_align_left_list.AddTail( frame );
			goto top;
			}			
		}

	if (!m_new_align_right_list.IsEmpty())
		{
		CFormatFrame *frame = (CFormatFrame *)m_new_align_right_list.GetHead();

		if (frame->GetActualSize().cx >= m_line_width - m_format_html->m_minimum_text_width)
			{
			if (m_cur_align_right_list.IsEmpty()  &&
				m_cur_align_left_list.IsEmpty() )
				{
				m_new_align_right_list.RemoveHead();

				if (frame->GetActualSize().cx >= m_line_width)
					frame->MoveFrame( CPoint( m_left_margin, m_cur_y) );
				else
					frame->MoveFrame( CPoint( m_right_margin - frame->GetActualSize().cx, m_cur_y) );
					
				m_contained_frames.Add( frame );
				CSize s = frame->GetActualSize();
				m_cur_y += s.cy;
				StretchActualSizeX( s.cx );
				StretchActualSizeY(m_cur_y);
				}
			// wait until we have more space to put the picture in			
			}
		else
			{
			frame->MoveFrame( CPoint( m_right_margin - frame->GetActualSize().cx, m_cur_y) );
			m_new_align_right_list.RemoveHead();
			m_cur_align_right_list.AddTail( frame );
			goto top;
			}		
		}
	}

BOOL CFormatFrame::IsLineEmpty() const
	{
	return m_current_line.GetSize() == 0;
	}

void CFormatFrame::AddTmpFormatItem( CTmpFormatItem *item)
	{
	m_current_line.Add(item);
	m_amount_on_line += item->m_width;
	m_empty_lines = -1;

	if (m_amount_on_line > m_line_width)
		BreakLine(break_lineonly);
	}

BOOL CFormatHTML::ApplyTag( const CTag * ptag, FORMAT_PARAMS& fp )
	{
	// helper pointers (not necessarily valid)
	const CTagINT32 *ptagint32 = (const CTagINT32 *)ptag;
	const CTagCString *ptagcstring = (const CTagCString *)ptag;
	const CTagImage * ptagimage = (const CTagImage *)ptag;

#ifdef USES_OLE_CONTROLS
	const CTagOLEControl * ptagole = (const CTagOLEControl *)ptag;
#endif
		
	switch(ptag->m_type)
		{
		case TAG_ANCHOR_HREF:
			m_hotlink = TRUE;
			m_current_hotlink = DEBUG_NEW CHotlink( ptagcstring->m_string );
			m_hotlinklist.AddTail(m_current_hotlink);
			break;
		case TAG_ANCHOR_HREF|TAG_END:
			m_hotlink = FALSE;
			m_current_hotlink = NULL;
			break;
		default:
			return FALSE;
		}
	return TRUE;
	}


CFormatHTML::CFormatHTML()
	{
	m_last_anchor = NULL;
	m_last_heading = NULL;
	m_frame = NULL;
	}
	
CFormatHTML::~CFormatHTML()
	{
	DeleteContents();
	}


void CFormatHTML::Format( CMimeObject *mime_object, FORMAT_PARAMS& fp)
	{
	ASSERT_UI_THREAD();

#ifdef _WINDOWS
	m_pDC = fp.m_pDC;
	CFormatFont format_font( fp.m_pDC );
#endif
#ifdef _MACINTOSH
	CFormatFont format_font;
#endif

	CPicture *picture = NULL;

	m_parse = mime_object;
	
	POSITION walk = NULL, oldwalk, oldoldwalk;

	m_first_new_item = NULL;

	m_format_font = DEBUG_NEW CFormatFont(fp.m_pDC);
	
	if (fp.m_start_position == NULL)
		{
		DeleteContents();

		mime_object->DEBUG_LOCK();
		m_plain_text = mime_object->GetPlainText();
		mime_object->Unlock();
	
		m_indent_stack_index = 0;
		
		m_last_hotlink = NULL;
	
		m_frame = new CFormatFrame( this, NULL, CPoint(0,0), CSize( fp.m_window_width, 0), 
			5,0,5,0);

		m_hotlink = 0;

		m_last_plain_pos = 0;
	
		m_font_flags = 3;

		m_last_anchor = NULL;
		m_last_heading = NULL;
	
		m_minimum_text_width = m_format_font->GetMaxCharWidth( FONTFLAG_BOLD|FONTFLAG_ITALIC|7);

		m_dots_per_inch = fp.m_pDC->GetDeviceCaps(  LOGPIXELSX );
		m_tab_size = m_dots_per_inch/2;

		oldwalk = BEFORE_START_POSITION;

		mime_object->DEBUG_LOCK();
		walk = mime_object->GetFirstTagPos();
		mime_object->Unlock();
		}
	else
		{
		if (fp.m_start_position == BEFORE_START_POSITION)
			{
			oldwalk = BEFORE_START_POSITION;
			mime_object->DEBUG_LOCK();
			walk = mime_object->GetFirstTagPos();
			mime_object->Unlock();
			}
		else
			{
			walk = oldwalk= fp.m_start_position;
			mime_object->DEBUG_LOCK();
			mime_object->GetNextTag(walk);
			mime_object->Unlock();
			}
		}
	
	while(walk)
		{
		oldoldwalk = oldwalk;
		oldwalk = walk;

		mime_object->DEBUG_LOCK();
		const CTag *ptag = mime_object->GetNextTag(walk);
		mime_object->Unlock();
		
		if (ptag->m_type == TAG_IMAGE)
			{
			fp.m_picture_info.UpdatePicture( ((CTagImage *)ptag)->m_url );
			}

		if (!m_frame->ApplyTag( ptag, fp ))
			{
			// rewind one item back, so next time we start on the item we couldn't do last time
			oldwalk = oldoldwalk;
			break;
			}
		}

	if (m_first_new_item)
		fp.m_first_new_format_item_pos = m_format_items.Find( m_first_new_item );
	else			 
		fp.m_first_new_format_item_pos = NULL;

	mime_object->DEBUG_LOCK();
	LOAD_STATE l = mime_object->GetLoadState();
	mime_object->Unlock();

	if (l != LOAD_STATE_LOADING)
		{
		m_frame->BreakLine( CFormatFrame::break_soft );
		fp.m_start_position = NULL;
		}
	else
		{
		fp.m_start_position = oldwalk;
		}

	delete m_format_font;
	m_format_font = NULL;

	m_parse = NULL;
	}


void CFormatHTML::GetFormatList(CPtrList &list, const CRect& r) const
	{
	POSITION walk = m_format_items.GetHeadPosition();

	while(walk)
		{
		CFormatItem *p = (CFormatItem *)m_format_items.GetNext(walk);
		if (IntersectRect(r, p->GetExtent() ))
			list.AddTail(p);;
		}
	}
	
void CFormatHTML::DeleteContents()
	{
	if (m_frame)
		delete m_frame;

#ifdef USES_OLE_CONTROLS
	m_control_list.RemoveAll();
#endif

	if (m_last_anchor)
		{
		delete m_last_anchor;
		m_last_anchor = NULL;
		}
	if (m_last_heading)
		{
		delete m_last_heading;
		m_last_heading = NULL;
		}

	POSITION walk = m_anchorlist.GetHeadPosition();
	while(walk)
		{
		CAnchor *p = (CAnchor *)m_anchorlist.GetNext(walk);
		delete p;
		}
	m_anchorlist.RemoveAll();

	walk = m_headinglist.GetHeadPosition();
	while(walk)
		{
		CHeading *p = (CHeading *)m_headinglist.GetNext(walk);
		delete p;
		}	
	m_headinglist.RemoveAll();

	walk = m_hotlinklist.GetHeadPosition();
	while(walk)
		{
		CHotlink *p = (CHotlink *)m_hotlinklist.GetNext(walk);
		delete p;
		}	
	m_hotlinklist.RemoveAll();

	m_format_items.RemoveAll();
	}
	
INT32 CFormatHTML::FindAnchorHeight(const char *name)
	{
	POSITION walk = m_anchorlist.GetHeadPosition();
	while(walk)
		{
		CAnchor *a = (CAnchor *)m_anchorlist.GetNext(walk);
		if (a->m_anchor_name == name)
			return a->m_y_offset;
		}
	return -1;
	}

POSITION CFormatHTML::GetFirstHeadingPos() const
	{
	return m_headinglist.GetHeadPosition();
	}

const CHeading * CFormatHTML::GetNextHeading(POSITION& walk) const
	{
	return (const CHeading *)m_headinglist.GetNext(walk);
	}
	
INT32 CFormatHTML::GetTextPositionAtHeight( INT32 height ) const
	{
	POSITION walk = m_format_items.GetHeadPosition();

	while(walk)
		{
		CFormatTextItem *p = (CFormatTextItem *)m_format_items.GetNext(walk);

		if (p->m_item_type == ITEM_TEXT ||
			p->m_item_type == ITEM_HOTLINK_TEXT)
			{
			if (p->m_extent.bottom > height)
				return p->m_text_ofs;
			}
		}
	return 0;
	}

INT32 CFormatHTML::GetHeightAtTextPosition( INT32 textpos ) const
	{
	POSITION walk = m_format_items.GetHeadPosition();

	while(walk)
		{
		CFormatTextItem *p = (CFormatTextItem *)m_format_items.GetNext(walk);

		if (p->m_item_type == ITEM_TEXT ||
			p->m_item_type == ITEM_HOTLINK_TEXT)
			{
			if (p->m_text_ofs + p->m_text_len > textpos)
				return p->m_extent.top;
			}
		}
	return 0;
	}
	
	

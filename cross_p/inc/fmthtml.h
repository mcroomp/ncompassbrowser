#define _FMTHTML_H_

#ifndef _READHTML_H_
#include "readhtml.h"
#endif

class CPicture;

#ifdef USES_OLE_CONTROLS

#ifndef _DMEMFILE_H_
#include "dmemfile.h"
#endif

#endif

class CFormatFrame;
class CTmpFormatItem;

// inline utility function
inline void Assign_Max( INT32& old, INT32 newvalue )
	{ 
	if (old < newvalue) 
		old = newvalue; 
	}

/* generalized cross-platform font formating and drawing wrapper */

class CFormatFont
	{
public:

#ifdef _WINDOWS
	CFormatFont(CDC *pDC);
#endif
#ifdef _MACINTOSH
	CFormatFont();
#endif
	~CFormatFont();

	BOOL GetLineBreak(INT32 flags, BOOL line_empty, const char *text, INT32 string_len, INT32 string_width);
	INT32 GetStringWidth( INT32 flags, LPCSTR string, INT32 string_len);

	INT32 GetAscent(INT32 flags);
	INT32 GetDescent(INT32 flags);
	INT32 GetMaxCharWidth(INT32 flags);

	void TextOut(INT32 flags, INT32 color, INT32 x, INT32 y, LPCSTR string, INT32 string_len );

private:
	void UpdateChanged(INT32 flags);

	INT32 m_flags;
	
#ifdef _WINDOWS
	CDC	*m_pDC;
	CFont *m_last_font;
	TEXTMETRIC m_last_text_metrics;
#endif

#ifdef _MACINTOSH
	short m_save_font;
	Style m_save_face;
	short m_save_size;
	FontInfo m_font_info;
#endif

	};

class CPictureInfo
	{
public:
	CPictureInfo();
	~CPictureInfo();

	void Reset( CNotifyObject *notify );
	void UpdatePicture( LPCSTR url );
	BOOL GetPictureSize( LPCSTR url, CSize& store_size );
	BOOL GetPicture(LPCSTR url, CPicture *& store_pointer );
	CString FindPicture( CMimeDynamicLoad *dlobject );

private:
	CMapStringToPtr m_picture_map;
	CNotifyObject *m_notify;
	};

// extra parameters for the OnRedraw function
class REDRAW_PARAMS
	{
public:
#ifdef _WINDOWS
	REDRAW_PARAMS( const CBigString& plain_text, CDC *pDC, BOOL hilight, LPBYTE frame_buffer,
		CPoint frame_buffer_ofs, CSize frame_buffer_size, INT32 text_color,
			INT32 hotlink_color, INT32 old_hotlink_color, INT32 back_color, CPictureInfo& picture_info);
#endif

#ifdef _MACINTOSH
	REDRAW_PARAMS( const CBigString& plain_text, BOOL hilight);
#endif

	~REDRAW_PARAMS();

	CBigString m_plain_text;
	CFormatFont m_format_font;
	BOOL m_hilight;

	LPBYTE m_frame_buffer;
	CPoint m_frame_buffer_ofs;
	CSize m_frame_buffer_size;
	INT32 m_text_color, m_hotlink_color, m_back_color, m_old_hotlink_color;

	CPictureInfo& m_picture_info;
	
#ifdef _WINDOWS
	CDC *m_pDC;
#endif
	};


class FORMAT_PARAMS
	{
public:

// input parameters
	INT32 m_window_width;
	INT32 m_page_height;
	CString m_base_url;
	POSITION m_start_position;

	CPictureInfo& m_picture_info;
	CNotifyObject *m_notify_object;
	
// output
	POSITION m_first_new_format_item_pos;

#ifdef _WINDOWS
	FORMAT_PARAMS( INT32 window_width, INT32 page_height, const char *base_url, CPictureInfo &picture_info,
		POSITION start_position, CDC *pDC, CNotifyObject *m_notify_object);

	CDC *m_pDC;

#endif

#ifdef _MACINTOSH
	FORMAT_PARAMS( INT32 window_width, INT32 page_height, const char *m_base_url );
#endif

	};


typedef enum {
		ITEM_NONE,
		ITEM_TEXT,
		ITEM_HOTLINK_TEXT,
		ITEM_RULE,
		ITEM_PICTURE,
		ITEM_HOTLINK_PICTURE,
		ITEM_QUICKTIME,

#ifdef USES_OLE_CONTROLS
		ITEM_OLECONTROL,
#endif

		ITEM_BULLET
		} ITEM_TYPE;

class CAnchor 
	{
public:
	INT32 m_y_offset;
	CString m_anchor_name;
	};
	
class CHeading
	{
public:
	INT32 m_y_offset;
	CString m_heading_name;
	INT32 m_heading_level;
	};

class CFormatItem;

class CHotlink
	{
public:
	CHotlink( LPCSTR url );
	
	inline POSITION GetFirstItemPosition() const
		{ return m_hotlink_items.GetHeadPosition(); }
	inline CFormatItem * GetNextItem( POSITION& walk) const
		{ return (CFormatItem *)m_hotlink_items.GetNext(walk); }
	inline void AddItem( CFormatItem *fi)
		{ m_hotlink_items.AddTail(fi); }
	const char *GetURL() const
		{ return m_url; }
protected:
	CPtrList m_hotlink_items;
	CString m_url;
	};

class CFormatItem : public CContainerItem
	{
public:

// platform dependent
	virtual void OnRedraw(REDRAW_PARAMS& params);
	virtual void OnScroll();
	virtual void OnClick(INT32 x, INT32 y);
	virtual void OnShow(BOOL flag);

// information on the item

// hotlink info
	virtual CHotlink *GetHotlink() const;
		// returns NULL if it is not a hotlink, otherwise a pointer to the
		//	CHotlink object this item is part of

// picture info
	virtual LPCSTR GetPicture() const;
		// returns NULL if the item does not have a dynamic loading picture
		// associated with it, a pointer to the URL

// data retrieval functions
	inline ITEM_TYPE GetItemType() const
		{ return m_item_type; }
	inline CFormatFrame *GetFrame() const
		{ return m_frame_parent; }

	CRect GetExtent() const;

protected:
	CFormatItem(ITEM_TYPE item_type, const CRect& extent, CFormatFrame *frame_parent);

	virtual ~CFormatItem();
	
	ITEM_TYPE m_item_type;			// item type
	CFormatFrame *m_frame_parent;
private:
	CRect m_extent;					// extent rect of the object relative to the frame

	friend class CFormatHTML;
	};

class CFormatTextItem : public CFormatItem
	{
public:
	CFormatTextItem(INT32 text_ofs, INT32 text_len, INT32 flags, const CRect& extent, CFormatFrame *frame);

	//platform dependent
	virtual void OnRedraw(REDRAW_PARAMS& params);
protected:
	INT32 m_text_ofs;
	INT32 m_text_len;
	INT32 m_flags;

	friend class CFormatHTML;
	};

	
class CFormatHotlinkTextItem : public CFormatTextItem
	{
public:
	CFormatHotlinkTextItem(CHotlink *hotlink, INT32 text_ofs, INT32 text_len, INT32 flags, const CRect& extent, CFormatFrame *frame);
	
	virtual CHotlink *GetHotlink() const;

// platform dependent
	virtual void OnRedraw(REDRAW_PARAMS& params);
	virtual void OnClick(INT32 x,INT32 y);

protected:
	CHotlink *m_hotlink;
	};

class CFormatRuleItem : public CFormatItem
	{
public:
	CFormatRuleItem(const CRect& extent, CFormatFrame *frame);
	
// platform dependent
	virtual void OnRedraw(REDRAW_PARAMS& params);
	};

class CFormatBulletItem : public CFormatItem
	{
public:
	CFormatBulletItem(BULLET_TYPE bullet_type, INT32 numeric, const CRect& extent, CFormatFrame *frame);
	
// platform dependent
	virtual void OnRedraw(REDRAW_PARAMS& params);
protected:
	BULLET_TYPE m_bullet_type;
	INT32 m_numeric;
	};


class CFormatPictureItem : public CFormatItem
	{
public:
	CFormatPictureItem(LPCSTR url, const CRect& extent, CFormatFrame *frame_parent);

	virtual LPCSTR GetPicture() const;
	
// platform dependent
	virtual void OnRedraw(REDRAW_PARAMS& params);
protected:
	CString m_url;
	};
	
class CFormatHotlinkPictureItem : public CFormatPictureItem
	{
public:
	CFormatHotlinkPictureItem( CHotlink *hotlink, INT32 border_size, BOOL is_mapped, LPCSTR url, const CRect& extent, CFormatFrame *frame_parent);
	virtual CHotlink *GetHotlink() const;
	
// platform dependent
	virtual void OnRedraw(REDRAW_PARAMS& params);
	virtual void OnClick(INT32 x,INT32 y);

	inline BOOL IsMapped() const
		{ return m_is_mapped; }

protected:
	CHotlink *m_hotlink;
	INT32 m_border_size;
	BOOL m_is_mapped;
	};

#ifdef USES_OLE_CONTROLS

class CFormatOLEControlItem : public CFormatItem
	{
public:
	CFormatOLEControlItem(const CLSID& clsid, LPCSTR ocx_url, INT32 parse_id, LPCSTR version, CDynLoadMemFile * dyn_mem, const CRect& extent, CFormatFrame *frame_parent);

	virtual void OnRedraw(REDRAW_PARAMS& params);

	inline INT32 GetParseID() const 
		{ return m_parse_id; }
	inline void GetClassID( CLSID& clsid) const	
		{ clsid = m_clsid; }
	inline CString GetOCX_URL() const
		{ return m_ocx_url; }
	inline CDynLoadMemFile *GetMemFile() const
		{ return m_memfile; }
	inline CString GetVersion() const
		{ return m_version; }	

protected:
	INT32			m_parse_id;
	CLSID 			m_clsid;
	CString 		m_ocx_url;
	CDynLoadMemFile *m_memfile;	
	CString 		m_version;	
	};

#endif

class CFormatTableColumn
	{
public:
	INT32 m_width;
	ALIGN_TYPE m_align, m_valign;
	CFormatFrame *m_frame;
	INT32 m_row_span_left, m_col_span;
	INT32 m_row_span_height;
	};

class CFormatForm
	{
public:
	CFormatForm();


private:
	CPtrList m_form_item_list;
	};

class CFormatFrame 
	{
public:
	typedef enum
		{
		clear_left = 1,
		clear_right = 2,
		clear_all = 3
		} CLEAR_TYPE;

	typedef enum
		{
		break_hard,
		break_soft,
		break_lineonly 
		} BREAK_TYPE;
	
	CFormatFrame(  CFormatHTML *format_html, CFormatFrame *parent, CPoint origin, CSize recommended_size,
	   		INT32 left_border, INT32 top_border, INT32 right_border, INT32 bottom_border);
	
	~CFormatFrame();

	BOOL ApplyTag( const CTag *ptag, FORMAT_PARAMS &fp );

	CSize GetActualSize() const;
		
	void MoveFrame( CPoint new_origin );
	
	inline CPoint GetOrigin() const
		{ return m_origin; }

	inline CFormatFrame *GetParent() const
		{ return m_parent; }
		
private:
	void StretchActualSizeX( INT32 size );
	void StretchActualSizeY( INT32 size );

	void Clear( CLEAR_TYPE clear_type );
	void Clear( TABLE_UNITS unit, INT32 amount );

	void BreakLine(BREAK_TYPE parabreak);
	void RecalcMargins();

	void AddTmpFormatItem( CTmpFormatItem *item);

	BOOL IsLineEmpty() const;

	void EndTableRow(BOOL lastrow);

	CFormatHTML *m_format_html;
	CFormatFrame *m_parent;

	INT32 m_top_border, m_bottom_border;
	INT32 m_left_border, m_right_border;  // size of surrounding border
	
	CPoint m_origin;
	CSize m_recommended_size;
	CSize m_actual_size;
	
	BOOL m_line_empty;

	INT32 m_cur_y;
	INT32 m_left_margin, m_right_margin, m_line_width;

	CPtrArray m_current_line;

	CPtrList	m_new_align_left_list, m_new_align_right_list,
				m_cur_align_left_list, m_cur_align_right_list;

	CPtrArray	m_contained_frames;

	CInt32Array m_indent_stack, m_align_stack;

	INT32 m_center_count;

	INT32 m_table_inside, m_table_x_offset, m_table_width, m_table_border_width, m_table_cell_padding;
	BOOL m_table_first_row;
	CPtrArray m_table_current_row;

	CFormatFrame *m_table_current_frame;
	INT32 m_table_max_y, m_table_current_column;
	
	ALIGN_TYPE m_align, m_previous_align;
	INT32 m_indent;
	INT32 m_first_indent;
	INT32 m_space_after_paragraph;
	BOOL m_first_line;
		
	INT32 m_amount_on_line;
	INT32 m_max_y_ascent;
	INT32 m_max_y_descent;

	INT32 m_empty_lines;

	friend class CFormatHTML;
	friend class CTmpFormatItem;
	friend class CTmpFormatTextItem;
	friend class CTmpFormatPictureItem;
#ifdef USES_OLE_CONTROLS
	friend class CTmpFormatOLEControlItem;
#endif
	};

class CFormatHTML 
	{
public:
	CFormatHTML();
	~CFormatHTML();
	
	void Format( CMimeObject *parsehtml, FORMAT_PARAMS& formatparams);

	POSITION GetFirstFormatItem() const
		{ return m_format_items.GetHeadPosition(); }
	CFormatItem * GetNextFormatItem(POSITION& walk)
		{ return (CFormatItem *)m_format_items.GetNext(walk); }

	INT32 GetTextPositionAtHeight( INT32 height ) const;
	INT32 GetHeightAtTextPosition( INT32 textpos ) const;
	
	void GetFormatList(CPtrList& list, const CRect& extent) const;
	
	inline CSize GetActualSize() const 
		{ return m_frame->GetActualSize(); }
	inline CBigString GetPlainText() const
		{ return m_plain_text; }	
	
#ifdef USES_OLE_CONTROLS
	inline POSITION GetOLEControlHeadPosition() const
		{ return m_control_list.GetHeadPosition(); }
	inline CFormatOLEControlItem *GetOLEControlNext(POSITION& walk) const
		{ return (CFormatOLEControlItem *)m_control_list.GetNext(walk); }
#endif

	INT32 FindAnchorHeight(const char *anchor_name);
	POSITION GetFirstHeadingPos() const;
	const CHeading * GetNextHeading(POSITION& walk) const;
private:
	// applies a text formatting tag, returns TRUE if the tag could be processes
	BOOL ApplyTag( const CTag *tag, FORMAT_PARAMS &fp );

	void Clear( INT32 flags);
	void DeleteContents();	
	
	BOOL m_hotlink;
	CHotlink *m_current_hotlink;

	INT32 m_minimum_text_width;
	
	CAnchor *m_last_anchor;
	INT32 m_last_anchor_pos;

	CHeading *m_last_heading;
	INT32 m_last_heading_pos;

	CBigString m_plain_text;
	INT32 m_last_plain_pos;

	CPtrList m_headinglist;
	
	CPtrList m_anchorlist;
	CPtrList m_hotlinklist;

#ifdef USES_OLE_CONTROLS
	CPtrList m_control_list;
#endif

	INT32 m_font_flags;

	INT32 m_indent_stack_index;
	
	CContainer m_format_items;

	CFormatFont *m_format_font;
	CHotlink *m_last_hotlink;

	INT32 m_dots_per_inch, m_tab_size;

	CFormatItem *m_first_new_item;

	CFormatFrame *m_frame;

	CMimeObject *m_parse;

// platform dependent members
#ifdef _WINDOWS
	CDC *m_pDC;
#endif

	friend class CFormatFrame;
	friend class CTmpFormatItem;
	friend class CTmpFormatTextItem;
	friend class CTmpFormatBulletItem;
	friend class CTmpFormatPictureItem;

#ifdef USES_OLE_CONTROLS
	friend class CTmpFormatOLEControlItem;
#endif

	friend void GetTableSize( FORMAT_PARAMS& fp, CFormatHTML *format_html, 
		CMimeObject *parsetags, POSITION& walk, 
		CInt32Array& i_max_column_widths, CInt32Array& i_min_column_widths,
		INT32& i_min_table_size, INT32& i_max_table_size);

	friend void GetColumnSize( FORMAT_PARAMS& fp, CFormatHTML *format_html, CMimeObject *parsetags, POSITION& walk, 
		INT32& min_size, INT32& max_size);
	};


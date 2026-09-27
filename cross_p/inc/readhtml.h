#define _READHTML_H_

#ifndef _CONTAIN_H_
#include "contain.h"
#endif

#ifndef _BIGSTR_H_
#include "bigstr.h"
#endif

#ifndef _MIMELOAD_H_
#include "mimeload.h"
#endif

//#define CHANGEFLAG_MIMEHEADER_READ 	0x00000001
//#define CHANGEFLAG_CACHED 		   	0x00000002
#define CHANGEFLAG_ADD_TEXT				0x00000010
#define CHANGEFLAG_TITLE 				0x00000020
#define CHANGEFLAG_BACK_COLOR 			0x00000040
#define CHANGEFLAG_TEXT_COLOR			0x00000080
#define CHANGEFLAG_HOTLINK_COLOR 		0x00000100
#define CHANGEFLAG_OLD_HOTLINK_COLOR	0x00000200
#define CHANGEFLAG_BACK_PICTURE			0x00000400

enum
	{
	TAG_TEXT,				// piece of text CTagText (no end)
	TAG_ANCHOR_NAME,		// begin=CTagCString  end=CTag
	TAG_ANCHOR_HREF,		// begin=CTagCString  end=CTag
	TAG_PARAGRAPH,			// begin=CTagBeginParagraph	  end=CTag
	TAG_HEADING,			// begin=CTagINT32	  end=CTag
	/*	m_ldata = heading level */
	TAG_HORZRULE,			// begin=CTagINT32	
	TAG_IMAGE,				// CTagImage
	/* 	m_sdata = filename
	   	m_ldata = align:
			0 = inline
			1 = left
			2 = right
		m_vspace = additional vertical space
		m_hspace = additional horizontal space
	*/
	TAG_NEWLINE,

	TAG_NEWLINE_CLEAR,
		/* 	int32 = clear flag
			1 = clear left
			2 = clear right
			3 = clear both
		*/
	
	TAG_NEWLINE_CLEAR_EN,
		/* int32 amount */

	TAG_NEWLINE_CLEAR_PIXEL,
		/* int32 amount */

	TAG_NEWLINE_HARD_BREAK,
		// causes as a new line with a hard break

	TAG_CONDITIONAL_BREAK,
	
	TAG_QUICKTIME,

#ifdef USES_OLE_CONTROLS
	TAG_OLECONTROL,
#endif
	
	TAG_TABLE,
	TAG_TABLECAPTION,
	TAG_TABLEROW,
	TAG_TABLECELL,

	TAG_BULLET,				// begin = CTagBullet

	TAG_CENTER,				// begin = CTag, end = CTag

// flag to signify end tag ( OR with normal tag)
	TAG_END = 0x1000
	};
	

class CTag : public CContainerItem
	{
public:		
	CTag(int type)
		{ m_type = type; }

	int m_type;
	};

#define FONTFLAG_BOLD 			0x0010
#define FONTFLAG_ITALIC			0x0020
#define FONTFLAG_UNDERLINE		0x0040
#define FONTFLAG_HOTLINK		0x0080
#define FONTFLAG_FIXED			0x0100
#define FONTFLAG_NO_WRAP		0x0200
#define FONTFLAG_SIZE			0x000f

class CTagText : public CTag
	{
public:
	CTagText(INT32 startpos, INT32 textlen, INT32 font_flags) : CTag( TAG_TEXT)
		{ m_startpos = startpos; m_textlen = textlen; m_font_flags = font_flags; }

	INT32 m_startpos, m_textlen;
	INT32 m_font_flags;
	};

// A tag with one integer parameter
class CTagINT32 : public CTag
	{
public:
	CTagINT32(int type, INT32 int32) : CTag(type)
		{ m_int32 = int32; }

	INT32 m_int32;
	};
	
class CTagCString : public CTag
	{
public:
	CTagCString(int type, const char *string) : CTag(type)
		{ m_string = string; }

	CString m_string;
	};

typedef enum
	{
	ALIGN_NONE = 0,
	ALIGN_TOP = 1,
	ALIGN_BOTTOM,
	ALIGN_BASELINE,
	ALIGN_MIDDLE,
	ALIGN_LEFT,
	ALIGN_RIGHT,
	ALIGN_CENTER,
	ALIGN_JUSTIFY,
	ALIGN_DECIMAL
	} ALIGN_TYPE;

class CTagImage : public CTag
	{
public:
	CTagImage(ALIGN_TYPE align, const char *url, INT32 width, INT32 height, INT32 hspace, INT32 vspace, INT32 border, BOOL is_mapped, BOOL can_break) : CTag(TAG_IMAGE)
		{ m_align = align; m_url = url; m_hspace = hspace; m_vspace = vspace; m_border = border; 
		  m_height = height; m_width = width; m_can_break = can_break; m_is_mapped = is_mapped; }
		
	ALIGN_TYPE m_align;
	INT32 m_hspace, m_vspace, m_border, m_height, m_width;

	CString m_url;
	BOOL m_is_mapped;
	BOOL m_can_break;
	};


class CTagBeginParagraph : public CTag
	{
public:
	CTagBeginParagraph( ALIGN_TYPE align, INT32 space_after, INT32 space_before, INT32 indent_amount, 
		INT32 indent_first_line ) : CTag( TAG_PARAGRAPH )
		{ m_align = align; m_space_after = space_after; m_space_before = space_before; m_indent_amount = indent_amount;
			m_indent_first_line = indent_first_line; }

	INT32 m_space_after, m_space_before;
	ALIGN_TYPE m_align;
	INT32 m_indent_amount, m_indent_first_line;
	};

typedef enum	
	{
	unit_en,
	unit_relative,
	unit_pixel
	} TABLE_UNITS;

class CTagTableRow;

class CSaveCellFormat
	{
public:
	INT32 m_font_flags, m_italic, m_bold, m_underline;
	CInt32Array m_font_size_stack;
	};

class CTagTable : public CTag
	{
public:
	CTagTable();

	ALIGN_TYPE m_table_align;

 	CInt32Array m_column_align;
	CInt32Array m_column_width;
	CInt32Array m_row_span_add;

	char m_decimal_point;
	TABLE_UNITS m_units;

	INT32 m_table_width, m_cell_padding;
	INT32 m_border_width;
	BOOL m_no_wrap;

	INT32 m_num_columns, m_cur_column, m_cur_row;

	BOOL m_parsed_whole_table;
		// True if the entire table has been parsed to the /TABLE tag
	CSaveCellFormat *m_save_format;
		// Information to restore the formatting to the state it was before 
		// the cell began. If the pointer is null, then this implies we are still in a cell
	CTagTableRow *m_cur_table_row;
	};

class CTagTableCaption : public CTag
	{
public:
	CTagTableCaption();

	CString m_caption;
	ALIGN_TYPE m_align;
	};

class CTagTableRow : public CTag
	{
public:
	CTagTableRow( CTagTable *current_table );
	
	ALIGN_TYPE m_align;
	char m_decimal_point;
	ALIGN_TYPE m_valign;
	BOOL m_no_wrap;
	};

class CTagTableCell : public CTag
	{
public:
	CTagTableCell( CTagTableRow *current_row);

	INT32 m_col_span, m_row_span;
	ALIGN_TYPE m_align;
	char m_decimal_point;
	ALIGN_TYPE m_valign;
	BOOL m_no_wrap;

	INT32 m_specified_width;
	};

typedef enum {
	BULLET_NONE,
	BULLET_NUMERIC,
	BULLET_HOLLOW_SQUARE,
	BULLET_FILLED_SQUARE,
	BULLET_HOLLOW_CIRCLE,
	BULLET_FILLED_CIRCLE
	} BULLET_TYPE;

class CTagBullet : public CTag
	{
public:
	CTagBullet( BULLET_TYPE bullet_type, INT32 bullet_number ) : CTag(TAG_BULLET)
		{ m_bullet_type = bullet_type; m_bullet_number = bullet_number; }

	BULLET_TYPE m_bullet_type;
	INT32 m_bullet_number;
	};

class CParseListRecord
	{
public:
	typedef enum
		{
		list_unordered,
		list_ordered,
		list_definition
		} LIST_TYPE;
	
	typedef enum
		{
		item_none,
		item_dl,
		item_dt,
		item_li
		} LIST_ITEM;

	CParseListRecord( LIST_TYPE list_type, BULLET_TYPE bullet_type)
		{ m_list_type = list_type; m_bullet_type = bullet_type; m_item_number = 1; m_current_item = item_none; }

	LIST_TYPE m_list_type;
	int m_item_number;
	BULLET_TYPE m_bullet_type;
	LIST_ITEM m_current_item;
	};

#ifdef _MACINTOSH
class CTagQuicktime : public CTag
	{
public:
	CTagQuicktime(INT32 pos, INT32 ldata, const char *sdata, INT32 height, INT32 width, INT32 hspace, INT32 vspace);

	INT32 m_height, m_width;
	INT32 m_hspace, m_vspace;
	};
#endif



#ifdef _WINDOWS

#ifdef USES_OLE_CONTROLS
class CTagOLEControl : public CTag
	{
public:
	CTagOLEControl(ALIGN_TYPE align, const char *url, INT32 width, INT32 height, INT32 hspace, INT32 vspace, const CLSID& clsid, LPCSTR ocx_url, LPCSTR version);
		
	ALIGN_TYPE m_align;
	INT32 m_hspace, m_vspace, m_height, m_width;
	INT32 m_parse_id;					// ID unique to this CParseHTML

	CLSID m_clsid;				// unique GUID of control

	CString m_version;
	CString m_ocx_url;				// URL of binary .OCX file
	CString m_url;						// URL of properties file
	};
#endif

#endif
					   
class CParseHTML : public CMimeObject
	{
public:
	CParseHTML( LPCSTR url, LPCSTR mime_type, BOOL start_plaintext );
	~CParseHTML();

// These functions require locking   		
	virtual POSITION FindTagPos( const CTag *tag) const;
	virtual CBigString GetPlainText() const;
	virtual POSITION GetFirstTagPos() const;
	virtual const CTag * GetNextTag(POSITION &walk) const;
	virtual CString GetTitle() const;
	virtual INT32 GetBackgroundColor() const;
	virtual INT32 GetTextColor() const;
	virtual INT32 GetHotlinkColor() const;
	virtual INT32 GetOldHotlinkColor() const;
	virtual CString GetBackgroundPicture() const;
			
protected:
	virtual LOAD_STATE OnReadData(LPCBYTE buffer, INT32 buffer_size);
		// Returns TRUE when the object has finished reading its data
	virtual LOAD_STATE OnEndOfFile();
		// Called at end of file
private:
// code to do with parsing
	inline void AppendChar(char c) 
		{ m_plain_text.AppendChar(c); }
	inline void AppendString(const char *source, INT32 len)
		{ m_plain_text.AppendString(source, len); }
		
	void append_html_text(const char *source, INT32 charcount);
	void append_pref_text(const char *source, INT32 charcount);
	void append_plain_text(const char *source, INT32 charcount);

	void ClearTagList();

  	CString m_title, m_background_url;
	INT32 m_back_color, m_text_color, m_hotlink_color, m_old_hotlink_color;

// current format information
	INT32 m_font_fixed;
	INT32 m_font_size;
	INT32 m_bold, m_italic, m_underline;
	CInt32Array m_font_size_stack;
	INT32 m_font_flags;
	BOOL m_is_plain_text;
		// TRUE if we should ignore all HTML tags and treat the rest of this file as plain text

// current parsing state
	BOOL m_inside_tag, m_inside_iso_code, m_inside_script;
	INT m_script_end_match;
	CString m_current_tag, m_current_iso_code;

	BOOL m_last_space;
	INT32 m_current_indent;

	INT m_bPreformatted, m_bInHead, m_bInTitle;
	INT m_BaseFontSize;
	CInt32Array m_BaseFontStack;

	CTagBeginParagraph *m_current_paragraph;
	CPtrList m_list_stack;
	CParseListRecord *m_current_list;

	int m_last_anchor_type;
	CContainer m_taglist;	
	
	CBigString m_plain_text;
	INT32 m_tab_size;	
	CString m_base_url;
	
	CPtrList m_table_stack;
	CTagTable *m_cur_table;

	ALIGN_TYPE m_default_align;

	CTag* AddBeginParagraph( ALIGN_TYPE align, INT32 space_after, INT32 space_before, INT32 indent_first_line, INT32 extra_indent = 0);
	CTag* AddEndParagraph();

	CTag* AddFormatTag(int type);
	CTag* AddFormatTagINT32(int type, INT32 int32);
	CTag* AddFormatTagCString(int type, const char *int32);
	CTag* AddFormatTagImage(ALIGN_TYPE align, const char *url, INT32 hspace, INT32 vspace, INT32 border, INT32 height, INT32 width, BOOL is_mapped);
	CTag* AddFormatTagText(INT32 startpos, INT32 textlen, INT32 font_flags);

	void PushFontSize(INT32 size);
	void PopFontSize();
	void PushBold();
	void PopBold();
	void PushUnderline();
	void PopUnderline();
	void PushItalic();
	void PopItalic();
	void PushFixed();
	void PopFixed();
	void ResetFont();

	void CParseHTML::BeginTableCell(int strid, const CPtrList& attrib_list );
	void BeginTableRow( const CPtrList& attrib_list );
	void EndTableCell( );
	};
	
int strnicmp(const char *string1, const char *string2, INT32 maxlen);
int strmatch(const char *source, const char *match);
void skipspaces(const char **text);

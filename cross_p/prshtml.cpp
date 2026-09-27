#include "cross_p.h"
#include "prshtml.h"

#include <ctype.h>

typedef enum 
	{
	STR_UNKNOWN = 1,
	
	STR_A,
	STR_B,
	STR_BASE,
	STR_BASEFONT,
	STR_BODY,
	STR_BR,
	STR_CAPTION,
	STR_CENTER,
	STR_CODE,
	STR_DD,
	STR_DL,
	STR_DT,
	STR_FONT,
	STR_H1,
	STR_H2,
	STR_H3,
	STR_H4,
	STR_H5,
	STR_H6,
	STR_HEAD,
	STR_HR,
	STR_HTML,
	STR_I,
	STR_IMG,
	STR_LI,
	STR_LISTING,
	STR_OL,
#ifdef USES_OLE_CONTROLS
	STR_OLECONTROL,
#endif
	STR_P,
	STR_PRE,
	STR_QUICKTIME,	
	STR_TABLE,
	STR_TD,
	STR_TH,
	STR_TR,
	STR_TITLE,
	STR_TT,
	STR_U,
	STR_UL,
	
	STR_END = 0x1000
	} STRING_ID;


typedef struct
	{
	const char *string;
	STRING_ID id;
	} STRING_ENTRY;

const STRING_ENTRY string_table[] = {
	{"a",			STR_A},
	{"b",			STR_B},
	{"base",		STR_BASE},
	{"basefont",	STR_BASEFONT},
	{"body",		STR_BODY},
	{"br",			STR_BR},
	{"caption",		STR_CAPTION},
	{"center",		STR_CENTER},
	{"code",		STR_CODE},
	{"dd",			STR_DD},
	{"dl",			STR_DL},
	{"dt",			STR_DT},
	{"font",		STR_FONT},
	{"h1",			STR_H1},
	{"h2",			STR_H2},
	{"h3",			STR_H3},
	{"h4",			STR_H4},
	{"h5",			STR_H5},
	{"h6",			STR_H6},
	{"head", 		STR_HEAD},
	{"hr",			STR_HR},
	{"html",		STR_HTML},
	{"i",			STR_I},
	{"img",			STR_IMG},
	{"li",			STR_LI},
	{"listing", 	STR_LISTING},
	{"ol",			STR_OL},
	{"p",			STR_P},
	{"pre",			STR_PRE},	
	{"quicktime",	STR_QUICKTIME},
	{"table",		STR_TABLE},
	{"td",			STR_TD},
	{"th",			STR_TH},
	{"title", 		STR_TITLE},
	{"tr",			STR_TR},
	{"tt",			STR_TT},
	{"u",			STR_U},
	{"ul",			STR_UL}

#ifdef USES_OLE_CONTROLS
	,{"xolecontrol",STR_OLECONTROL}
#endif
	};

 #define NUM_STRING (sizeof(string_table)/sizeof(string_table[0]))

#define MAX_ISO_LENGTH 8

typedef struct 
	{
	char string[MAX_ISO_LENGTH];
	int character;
	} ISO_8879;

/* this is a map of ISO 8879 character codes to the windows character set */

const ISO_8879 iso_8879_table[] = {
	{"quot", 	'"' },		{"amp",		'&' },	{"lt",		'<' },		{"gt",		'>' },
	{"nbsp",	0xa0},		{"iexcl",	0xa1},	{"cent",	0xa2},		{"pound",	0xa3},
	{"curren",	0xa4},		{"yen",		0xa5},	{"brvbar",	0xa6},		{"sect",	0xa7},
	{"die",		0xa8},		{"copy",	0xa9},	{"ordf",	0xaa},		{"laquo",	0xab},
	{"not",		0xac},		{"shy",		0xad},	{"reg",		0xae},		{"macron",	0xaf},
	{"degree",	0xb0},		{"plusmn",	0xb1},	{"sup2",	0xb2},		{"sup3",	0xb3},
	{"actute",	0xb4},		{"micro",	0xb5},	{"para",	0xb6},		{"middot",	0xb7},
	{"Cedilla",	0xb8},		{"sup1",	0xb9},	{"ordm",	0xba},		{"raquo",	0xbb},
	{"frac14",	0xbc},		{"frac12",	0xbd},	{"frac34",	0xbe},		{"iquest",	0xbf},
	{"Agrave",	0xc0},		{"Aacute",	0xc1},	{"Acirc",	0xc2},		{"Atilde",	0xc3},
	{"Auml",	0xc4},		{"Aring",	0xc5},	{"AElig",	0xc6},		{"Ccedil",	0xc7},
	{"Egrave",	0xc8},		{"Eacute",	0xc9},	{"Ecirc",	0xca},		{"Euml",	0xcb},
	{"Igrave",	0xcc},		{"Iacute",	0xcd},	{"Icirc",	0xce},		{"Iuml", 	0xcf},
	{"ETH",		0xd0},		{"Ntilde",	0xd1},	{"Ograve",	0xd2},		{"Oacute",	0xd3},
	{"Ocirc",	0xd4},		{"Otilde",	0xd5},	{"Ouml",	0xd6},		{"times",	0xd7},
	{"Oslash",	0xd8},		{"Ugrave",	0xd9},	{"Uacute",	0xda},		{"Ucirc",	0xdb},
	{"Uuml",	0xdc},		{"Yacute",	0xdd},	{"THORN",	0xde},		{"szlig",	0xdf},
	{"agrave",	0xe0},		{"aacute",	0xe1},	{"acirc",	0xe2},		{"atilde",	0xe3},
	{"auml",	0xe4},		{"aring",	0xe5},	{"aelig",	0xe6},		{"ccedil",	0xe7},
	{"egrave",	0xe8},		{"eacute",	0xe9},	{"ecirc",	0xea},		{"euml",	0xeb},
	{"igrave",	0xec},		{"iacute",	0xed},	{"icirc",	0xee},		{"iuml",	0xef},
	{"eth",		0xf0},		{"ntilde",	0xf1},	{"ograve",	0xf2},		{"oacute",	0xf3},
	{"ocirc", 	0xf4},		{"otilde",	0xf5},	{"ouml",	0xf6},		{"divide",	0xf7},
	{"oslash",	0xf8},		{"ugrave",	0xf9},	{"uacute",	0xfa},		{"ucirc",	0xfb},
	{"uuml",	0xfc},		{"yacute",	0xfd},	{"thorn",	0xfe},		{"yuml",	0xff } };

#define ISO_TABLE_SIZE (sizeof( iso_8879_table ) / sizeof( iso_8879_table[0] ) )

/* function prototypes to keep MetroWerks happy */

void ParseTag(CPtrList& attrib_list, const char *tagstart, INT32 &store_length, CString& store_name);
char ParseIsoCode( const char *tagstart );
STRING_ID GetStringID(const char *tagname);
CString ParseURL( const char *url );

class CAttribute
	{
public:
	CAttribute(const char *name, INT32 name_len, const char *value, INT32 value_len);
	~CAttribute();
		
	CString m_name;
	CString m_value;
	};
	
#define LIST_NONE 		0
#define LIST_UNORDERED 	1
#define LIST_ORDERED 	2
#define LIST_DEFINITION 3
	

INT32 StringToINT32( LPCSTR string, BOOL hex, LPCSTR* endofstring = NULL)
	{
	LPCSTR p = string;
	char *dummy;

	while(*p)
		{
		if (strchr("ABCDEFabcdef", *p))
			{
			hex = TRUE;
			break;
			}
		p++;
		}

	skipspaces( &string );
	if (*string == '#')
		string++;		// skip initial pound sign, if present

	if (hex)
		{
		// we are dealing with a hexdecimal number
		if (endofstring)
			return strtol(string, (char **)endofstring, 16);
		else
			return strtol(string, &dummy, 16);
		}
	else
		{
		// hopefully the number is in normal decimal notation
		if (endofstring)
			return strtol(string, (char **)endofstring, 10);
		else
			return strtol(string, &dummy, 10);
		}
	}

void ParseColspec(CTagTable *table, LPCSTR colspec)
	{
	skipspaces(&colspec);

	while(*colspec)
		{
		ALIGN_TYPE align = ALIGN_LEFT;
		INT32 amount = 0;

		switch(*colspec)
			{
			case 'L':
				align = ALIGN_LEFT;
				break;
			case 'R':
				align = ALIGN_RIGHT;
				break;
			case 'C':
				align = ALIGN_CENTER;
				break;

			}
		if ( !isdigit((unsigned char)*colspec) )
			colspec++;

		amount = StringToINT32( colspec, FALSE, &colspec );
		if (amount == 0)
			return;

		table->m_column_align.Add( align );
		table->m_column_width.Add( amount );			
		}
	}

ALIGN_TYPE ParseAlignType( LPCSTR string)
	{
	if (stricmp(string, "left")==0)
		return ALIGN_LEFT;
	else if (stricmp(string, "right")==0)
		return ALIGN_RIGHT;
	else if (stricmp(string, "center")==0)
		return ALIGN_CENTER;
	else if (stricmp(string, "top")==0)
		return ALIGN_TOP;
	else if (stricmp(string, "bottom")==0)
		return ALIGN_BOTTOM;
	else if (stricmp(string, "baseline")==0)
		return ALIGN_BASELINE;
	else if (stricmp(string, "middle")==0)
		return ALIGN_MIDDLE;
	else if (stricmp(string, "justify")==0)
		return ALIGN_JUSTIFY;
	else if (stricmp(string, "decimal")==0)
		return ALIGN_DECIMAL;
	else
		return ALIGN_NONE;
	}

int strnicmp(const char *string1, const char *string2, INT32 maxlen)
	{
	int retval,i;
	for(i=0;i<maxlen;i++)
		{
		if (!*string1)
			{
			if (!*string2)
				return 0;
			else
				return 1;
			}
		else
			{
			if (!*string2)
				return -1;
			}
		
		if ((retval = (toupper((unsigned char)*string1) - toupper((unsigned char)*string2)) ) != 0)
			return retval;
		*string1++;
		*string2++;
	}
	return 0;
	}


		
/* tries to match all the characters of the second string to the beginning of the first */

int strmatch(const char *source, const char *match)
	{
	return strnicmp(source, match, strlen(match));
	}

#define numformatcodes (sizeof(formatcodes)/sizeof(formatcodes[0]))		


CParseHTML::CParseHTML( CMimeDynamicLoad *parent )
	: CMimeObject( MIME_OBJECT_HTML, parent )
	{
	m_font_flags = 3;

	m_bold = 0;
	m_italic = 0;
	m_underline = 0;
	m_font_fixed = 0;	

	m_tab_size = 32;
	m_last_anchor_type = FALSE;

	m_back_color = -1;
	m_text_color = -1; 
	m_hotlink_color = -1;
	m_old_hotlink_color = -1;

	m_inside_tag = FALSE;
	m_inside_iso_code = FALSE;
	m_bPreformatted = 0;
	m_bInHead = 0;
	m_bInTitle = 0;
	m_BaseFontSize = 3;
	m_current_list = NULL;
	m_base_url = GetURL();
	m_last_space = TRUE;
	m_cur_table = NULL;
	m_current_indent = 0;
	m_default_align = ALIGN_NONE;
	m_current_paragraph = NULL;
	}
	
CParseHTML::~CParseHTML()
	{
	ClearTagList();
	}

/* does a copy from the source to the destination, removing multiple spaces and
   substituting cr & lfs & tabs with spaces */

void CParseHTML::append_html_text(const char *source, INT32 charcount)
	{
	INT32 startpos = m_plain_text.GetLength();

	if (m_inside_iso_code)
		{
		LPCSTR p = (LPCSTR)memchr(source, ';', charcount);

		if (p)
			{
			int length = p - source + 1;

			if (length > 10)
				goto bad_iso_code;

			m_current_iso_code += CString(source, length);
			AppendChar( ParseIsoCode( m_current_iso_code ) );
			m_current_iso_code.Empty();
			m_inside_iso_code = FALSE;
			source += length;
			charcount -= length;
			}
		else
			{
			if (m_current_iso_code.GetLength() + charcount > 10)
				{
bad_iso_code:
				m_inside_iso_code = FALSE;
				CString t = m_current_iso_code;
				AppendChar('&');

				m_current_iso_code.Empty();
				if (t.GetLength() > 0)
					AppendString(t, t.GetLength() );
				}
			else
				{
				m_current_iso_code += CString(source, charcount);
				return;
				}
			}
		}

	while(charcount > 0)
		{
		if (*source == '\r' || *source == '\n' || *source == ' ' || *source == '\t')
			{
			if (!m_last_space)
				{
				AppendChar(' ');
				m_last_space = TRUE;
				}
			}
		else if (*source == '&')
			{
			// find the length of the string
		
			LPCSTR p = (LPCSTR)memchr(source, ';', charcount);

			int length;

			if (!p)
				{
				// try to ignore badly formatted HTML with & in it
				if (charcount > 7)
					{
					length = 1;
					AppendChar('&');
					}
				else
					{
					m_inside_iso_code = TRUE;
					m_current_iso_code = CString(source, charcount);
					AddFormatTagText(startpos, m_plain_text.GetLength() - startpos, m_font_flags); 
					return;
					}
				}
			else
				{			
				length = p - source;
				if (length > 10)
					{
					length =1;
					AppendChar('&');
					}
				else
					{
					AppendChar( ParseIsoCode( source ) );
					}
				}
			
			source += length;
			charcount -= length;
			m_last_space = FALSE;
			}
		else
			{
			AppendChar(*source);
			m_last_space = FALSE;
			}
			
		source++;
		charcount--;
		}

	AddFormatTagText(startpos, m_plain_text.GetLength() - startpos, m_font_flags); 
	}	

void CParseHTML::append_pref_text(const char *source, INT32 charcount)
	{
	BOOL seen_cr = FALSE, seen_lf = FALSE;	
	INT32 startpos = m_plain_text.GetLength(), textlen = 0;

	if (m_inside_iso_code)
		{
		LPCSTR p = (LPCSTR)memchr(source, ';', charcount);

		if (p)
			{
			m_current_iso_code += CString(source, p - source + 1);
			AppendChar( ParseIsoCode( m_current_iso_code ) );
			}
		else
			{
			m_current_iso_code += CString(source, charcount);
			return;
			}
		}

	while(charcount > 0)
		{
		if (*source == '\r')
			{
			if (seen_lf == FALSE)
				{
				AddFormatTagText(startpos, textlen, m_font_flags|FONTFLAG_NO_WRAP);
				startpos = m_plain_text.GetLength(); textlen = 0;
				AddFormatTag(TAG_NEWLINE_HARD_BREAK);
				seen_cr = TRUE;
				}
			}
		else if (*source == '\n')
			{
			if (seen_cr == FALSE)
				{
				AddFormatTagText(startpos, textlen, m_font_flags|FONTFLAG_NO_WRAP);
				startpos = m_plain_text.GetLength(); textlen = 0;
				AddFormatTag(TAG_NEWLINE_HARD_BREAK);
				seen_lf = TRUE;
				}
			}
		else if (*source == '\t')
			{
			AppendString("    ",4);
			seen_cr = FALSE;
			seen_lf = FALSE;
			textlen+=4;
			}
		else if (*source == '&')
			{
			// find the length of the string
		
			LPCSTR p = (LPCSTR)memchr(source, ';', charcount);

			if (!p)
				{
				m_inside_iso_code = TRUE;
				m_current_iso_code = CString(source, charcount);
				AddFormatTagText(startpos, m_plain_text.GetLength() - startpos, m_font_flags|FONTFLAG_NO_WRAP); 
				return;
				}
			
			int length = p - source;

			AppendChar( ParseIsoCode( source ) );
			
			source += length;
			charcount -= length;
			textlen++;
			seen_cr = FALSE;
			seen_lf = FALSE;
			}
		else
			{
			AppendChar(*source);
			seen_cr = FALSE;
			seen_lf = FALSE;
			textlen++;
			}
		source++;
		charcount--;
		}

	AddFormatTagText(startpos, m_plain_text.GetLength() - startpos, m_font_flags|FONTFLAG_NO_WRAP); 
	}	


void skipspaces(const char **text)
	{
	const char *t = *text;
	while(*t && (*t == ' ' || *t == '\t' || *t == '\r' || *t == '\n') )
		t++;
	*text = t;
	} 


char ParseIsoCode( const char *source )
	{
	ASSERT(*source == '&');

	source++;

	if (*source == '#')
		{
		// we have a numeric code
		source++;
		int number = 0;
		while( isdigit( (unsigned char)*source ) )
			{
			number = number * 10 + (*source - '0' );
			source++;
			}
		return (char)number;
		}
	else
		{
		// find the length of the string
		LPSTR p = strchr(source, ';');
		if (p != NULL && (p - source) < MAX_ISO_LENGTH )
			{
			int length = (p - source);

			for( int i = 0; i< ISO_TABLE_SIZE; i++)
				{
				if (memcmp( iso_8879_table[i].string, source, length) == 0)
					return iso_8879_table[i].character;
				}
			source += length;
			}
		return '?';
		}
	}

void ParseTag(CPtrList& attrib_list, const char *tagstart, CString& store_name)
	{
	INT32 fieldlen =0, fieldnamelen = 0, vallen = 0;
	int bInQuote = 0;
	const char *p, *fieldstart, *valstart;
	
	p = tagstart;
	
	if (*p == '<')
		p++;
		
	skipspaces(&p);
	fieldstart = p;
	while(*p && isalnum((unsigned char)*p) || *p == '/')
		p++;
	
	store_name = CString(fieldstart, p - fieldstart);	
	store_name.MakeLower();

	while(*p)
		{
		skipspaces(&p);
		if (*p == '>')
			break;

		fieldstart = p;
		fieldnamelen = 0;
		while(isalnum((unsigned char)*p) || *p == '-' || *p == '_' )
			p++, fieldnamelen++;

		if (fieldnamelen == 0)
			{
			// some unknown or non-alphabetic characters was found.. skip it and try again
			// to guarantee progress
			p++;		
			continue;
			}

		skipspaces(&p);
		if (*p == '=')
			{
			p++;
			skipspaces(&p);
			vallen = 0;
			if (*p == '"')
				{
				p++;
				valstart = p;
				while(*p && *p!='"' && *p!='>')
					p++, vallen++;
				if (*p=='"')
					p++;		// skip closing quote
				}
			else
				{
				valstart = p;
				while(*p && *p!=' ' && *p!='\t' && *p!='\n' && *p!='\r' && *p!='>')
					p++, vallen++;
				}
			
			attrib_list.AddTail( DEBUG_NEW CAttribute(fieldstart, fieldnamelen, valstart, vallen));	
			}
			
		else if (*p=='"')
			{
			p++;
			// if we somehow managed to get to a quoted section, skip all of it
			while(*p && *p!='"')
				p++;
			}
		else if (*p=='>')
			{
			// there was no =, so add an attribute with a blank value field and terminate
			attrib_list.AddTail( DEBUG_NEW CAttribute(fieldstart, fieldnamelen, 0, 0));
			break;
			}
		else if (*p)
			{
			// there was no =, so add an attribute with a blank value field
			attrib_list.AddTail( DEBUG_NEW CAttribute(fieldstart, fieldnamelen, 0, 0));
			}
		}
	}

STRING_ID GetStringID(const char *tagname)
	{
	if (tagname[0] == '/')
		{	
		tagname++;
		
		int i,ilen = NUM_STRING;
		for(i=0;i<ilen;i++)
			{
			if (strcmp(string_table[i].string, tagname) == 0)
				return (STRING_ID)(string_table[i].id | TAG_END);
			}
		}
	else
		{
		int i,ilen = NUM_STRING;
		for(i=0;i<ilen;i++)
			{
			if (strcmp(string_table[i].string, tagname) == 0)
				return string_table[i].id;
			}
		}
	return STR_UNKNOWN;
	}

LOAD_STATE CParseHTML::OnReadData(LPCBYTE buffer, INT32 buffer_size)
	{
	m_access_mutex.LockWrite();

	LPCSTR pbuffer = (LPCSTR)buffer;
	LPCSTR tagend,p;

	INT32 amount_left = buffer_size;
	INT32 amount_plain;
	BOOL done;

	CPtrList attrib_list;
	CString store_name;

	done = FALSE;

	if (m_inside_tag)
		{
		p = (LPCSTR)memchr(pbuffer,'>', buffer_size);
		if (!p)
			{
			// we are still inside a tag... just add what we read to the current_tag string
			m_current_tag += CString( pbuffer, buffer_size);
			m_access_mutex.UnlockWrite();
			Notify( CHANGEFLAG_ADD_TEXT );
			return LOAD_STATE_LOADING;
			}
		else
			{
			// we have the end of tag string inside the buffer... copy
			// the remaining stuff to the m_current_tag and the continue as normal
			
			INT32 size = p - pbuffer + 1;
			m_current_tag += CString( pbuffer, size );
			
			pbuffer += size;
			amount_left -= size;
			m_inside_tag = FALSE;

			ParseTag(attrib_list, m_current_tag, store_name);
			m_current_tag.Empty();
			goto got_a_tag;
			}
		}
	
	while(!done)
		{
		// find the beginning of a tag
		p = (LPCSTR)memchr(pbuffer, '<', amount_left);

		// anount_plain is the amount of plain text between the beginning of pbuffer and the
		// next tag									  
				
		if (!p)
			{
			amount_plain = amount_left;
			}
		else
			{
			amount_plain = p - pbuffer;
			}
		
		if (m_bInTitle > 0)
			{
			// header text is handled specially
			m_title += CString(pbuffer, amount_plain);
			}				
    	else if (m_bInHead > 0)
   			{
			// ignore other header text
			}
		else if (m_bPreformatted > 0)
    		{
    		append_pref_text(pbuffer, amount_plain);
    		}    		
   		else 
			{
   			append_html_text(pbuffer, amount_plain);
			}
			
		if (!p)
			{
			// no more tags in the text, so quit for now

			Notify(CHANGEFLAG_ADD_TEXT);
			m_access_mutex.UnlockWrite();
			return LOAD_STATE_LOADING;
			}

		pbuffer = p;
		amount_left -= amount_plain;
				
		tagend = (LPCSTR)memchr(pbuffer, '>', amount_left );
		if (!tagend)
			{
			// this is the last tag in the block and it doesn't end before
			// the end of the block
			m_current_tag = CString(pbuffer, amount_left);
			m_inside_tag = TRUE;
			Notify(CHANGEFLAG_ADD_TEXT);
			return LOAD_STATE_LOADING;
			}
		
		ParseTag(attrib_list, pbuffer, store_name);

		amount_left -= (tagend - pbuffer) + 1;
		pbuffer = tagend+1;
got_a_tag:
		// if we are in the header, then set the flag to ignore subsequent text
 
 		STRING_ID strid = GetStringID(store_name);
 		
 		switch(strid)
 			{
			case STR_BODY:
					{
					// handle a body tag
		    		POSITION walk = attrib_list.GetHeadPosition();
		    		while(walk)
		    			{
		    			const CAttribute *a = (const CAttribute *)attrib_list.GetNext(walk);
		    			if (a->m_name == "bgcolor")
							{
							m_back_color = StringToINT32(a->m_value, TRUE);
							Notify( CHANGEFLAG_BACK_COLOR );
							}
						else if (a->m_name == "text")
							{
							m_text_color = StringToINT32(a->m_value, TRUE);
							Notify( CHANGEFLAG_TEXT_COLOR );
							}
						else if (a->m_name == "link")
							{
							m_hotlink_color = StringToINT32(a->m_value, TRUE);
							Notify( CHANGEFLAG_HOTLINK_COLOR );
							}
						else if (a->m_name == "vlink")
							{
							m_old_hotlink_color = StringToINT32(a->m_value, TRUE);
							Notify( CHANGEFLAG_OLD_HOTLINK_COLOR );
							}
						else if (a->m_name == "background")
							{
							m_background_url = CombineURL(m_base_url, a->m_value);
							Notify( CHANGEFLAG_BACK_PICTURE );
							}
						}
					}
				break;
			case STR_HTML|STR_END:
				done = TRUE;
				break;				
 			case STR_HEAD:
    			m_bInHead++;
    			break;
    		case STR_HEAD|STR_END:
    			m_bInHead--;
   				break;
   			case STR_TITLE:
				m_title.Empty();
   				m_bInTitle++;
   				break;
   			case STR_TITLE|STR_END:
   				m_bInTitle--;
				Notify(CHANGEFLAG_TITLE);
   				break;
			case STR_BASE:
					{
					POSITION walk = attrib_list.GetHeadPosition();
		    		while(walk)
		    			{
		    			const CAttribute *a = (const CAttribute *)attrib_list.GetNext(walk);
		    			if (a->m_name == "href")
							m_base_url = a->m_value;
						}
		    		}
				break;
   			case STR_LISTING:
   			case STR_PRE:
    			m_bPreformatted++;
				AddBeginParagraph( m_default_align, 1, 1, 0);
    			PushFixed();
   				break;
   			case STR_LISTING|STR_END:
   			case STR_PRE|STR_END:
   				m_bPreformatted--;
				PopFixed();
				AddEndParagraph();				   				
    			break;
    		case STR_CODE:
				PushFixed();
    			break;
    		case STR_CODE|STR_END:
				PopFixed();
    			break;
    		case STR_DL:
    				{
					m_current_list = new CParseListRecord( CParseListRecord::list_definition, BULLET_NONE);
					m_list_stack.AddTail( m_current_list );
					m_current_indent++;
    				}
    			break;
    		case STR_DT:
    			if (m_current_list && m_current_list->m_list_type == CParseListRecord::list_definition)
					{
					AddBeginParagraph( m_default_align, 0, 0, -1);
					}
				else
					{
					AddBeginParagraph( m_default_align, 0,0,0);
					}
				break;
			case STR_DD:
	   			if (m_current_list && m_current_list->m_list_type == CParseListRecord::list_definition)
					{
					AddBeginParagraph( m_default_align, 0, 0, 0);
					}
				else
					{
					// somebody's using DD to indent something outside of a definition list...
					AddBeginParagraph( m_default_align, 0,0,0, 1);
					}
				break;
	    	case STR_DL|STR_END:
		    	if (m_current_list && m_current_list->m_list_type == CParseListRecord::list_definition)
					{
					AddEndParagraph();

					m_current_indent--;

	    			delete m_current_list;
					m_list_stack.RemoveTail();

					if (!m_list_stack.IsEmpty() )
						{
						m_current_list = (CParseListRecord *) m_list_stack.GetTail();
						}
					else
						{
						m_current_list = NULL;
						}
		    		}
		    	break;
			case STR_HR:
					{
					AddBeginParagraph( m_default_align, 1, 1, 0);

					INT32 width = 100;
		    		POSITION walk = attrib_list.GetHeadPosition();
		    		while(walk)
		    			{
		    			const CAttribute *a = (const CAttribute *)attrib_list.GetNext(walk);
		    			if (a->m_name == "width")
		    				{
		    				width = StringToINT32( a->m_value, FALSE );
		    				}
		    			}
		    		if (width < 0)
		    			width = 0;
		    		else if (width > 100)
		    			width = 100;
		    			
		    		AddFormatTagINT32(TAG_HORZRULE, width);
		    		
					AddEndParagraph();
					}
	    		break;
	    	case STR_UL:
		    		{
					if (!m_current_list)
						AddBeginParagraph(m_default_align, 0, 1, 0);

					m_current_list = new CParseListRecord( CParseListRecord::list_unordered, BULLET_FILLED_CIRCLE );
					m_list_stack.AddTail( m_current_list );
					m_current_indent++;
    				}
	    		break;
	    	case STR_UL|STR_END:
		    	if (m_current_list && m_current_list->m_list_type == CParseListRecord::list_unordered)
					{
					AddEndParagraph();

					m_current_indent--;

	    			delete m_current_list;
					m_list_stack.RemoveTail();

					if (!m_list_stack.IsEmpty() )
						{
						m_current_list = (CParseListRecord *) m_list_stack.GetTail();
						}
					else
						{
						m_current_list = NULL;
						}

					if (!m_current_list)
						AddBeginParagraph(m_default_align, 1, 0, 0);
					}
				break;				
	    	case STR_OL:
		    		{
					if (!m_current_list)
						AddBeginParagraph(m_default_align, 0, 1, 0);

					m_current_list = new CParseListRecord( CParseListRecord::list_ordered, BULLET_FILLED_CIRCLE);
					m_list_stack.AddTail( m_current_list );
					m_current_indent++;
    				}
	    		break;
		    case STR_OL|STR_END:
	   	    	if (m_current_list && m_current_list->m_list_type == CParseListRecord::list_ordered)
					{
					AddEndParagraph();

					m_current_indent--;

	    			delete m_current_list;
					m_list_stack.RemoveTail();

					if (!m_list_stack.IsEmpty() )
						{
						m_current_list = (CParseListRecord *) m_list_stack.GetTail();
						}
					else
						{
						m_current_list = NULL;
						}

					if (!m_current_list)
						AddBeginParagraph(m_default_align, 1, 0, 0);
					}
				break;
	    	case STR_LI:
				if (m_current_list)
					{
					if (m_current_list->m_list_type == CParseListRecord::list_unordered)
						{
						AddBeginParagraph( m_default_align, 0, 0, -1);
						new (m_taglist) CTagBullet(m_current_list->m_bullet_type, 0);
						m_last_space = TRUE;
						}
					else if (m_current_list->m_list_type == CParseListRecord::list_ordered)
						{
						AddBeginParagraph( m_default_align, 0, 0, -1);
						new (m_taglist) CTagBullet(BULLET_NUMERIC, m_current_list->m_item_number);
	   					m_current_list->m_item_number++;
						m_last_space = TRUE;
	   					}
	   				}
	    		break;
	    	case STR_A:
		  			{
					if (m_last_anchor_type)
						{
						AddFormatTag( TAG_ANCHOR_HREF|TAG_END);
    					m_last_anchor_type = FALSE;
						m_font_flags &= ~FONTFLAG_HOTLINK;
						}
    				
		    		POSITION walk = attrib_list.GetHeadPosition();
		    		while(walk)
		    			{
		    			const CAttribute *a = (const CAttribute *)attrib_list.GetNext(walk);
		    			if (a->m_name == "href")
		    				{
		    				AddFormatTagCString(TAG_ANCHOR_HREF,		    				
		    					CombineURL(m_base_url, a->m_value));

							m_font_flags |= FONTFLAG_HOTLINK;
		    				m_last_anchor_type = TRUE;
		    				break;
		    				}
		    			else if (a->m_name == "name")
		    				{
		    				AddFormatTagCString(TAG_ANCHOR_NAME, a->m_value);
		    				break;
		    				}
		    			}
		    		}
		    	break;
		    case STR_A|STR_END:
    			if (m_last_anchor_type)
    				{
    				AddFormatTag( TAG_ANCHOR_HREF|TAG_END);
    				m_font_flags &= ~FONTFLAG_HOTLINK;
    				m_last_anchor_type = FALSE;
    				}
				break;
    		case STR_P:
					{
					POSITION walk = attrib_list.GetHeadPosition();
					ALIGN_TYPE align = m_default_align;
					while (walk)
						{
						const CAttribute *a = (const CAttribute *)attrib_list.GetNext(walk);

						if (a->m_name == "align")
							{
							align = ParseAlignType( a->m_value);
							}
						}

					AddBeginParagraph( align, 0, 1, 0, BULLET_NONE );
					}
		    	break;
			case STR_P|STR_END:
				AddEndParagraph();
				break;
		    case STR_BR:
	    			{
		    		POSITION walk = attrib_list.GetHeadPosition();
		    		INT32 ldata = 0;
		    		while(walk)
		    			{
		    			const CAttribute *a = (const CAttribute *)attrib_list.GetNext(walk);
		    			if (a->m_name == "clear")
		    				{
		    				CString param = a->m_value;
		    				param.MakeLower();
		    				
		    				if (param == "left")
		    					{
		    					ldata = 1;
		    					break;
		    					}
		    				if (param == "right")
		    					{
		    					ldata = 2;
		    					break;
		    					}
		    				if (param == "all" || a->m_value.IsEmpty())
		    					{
		    					ldata = 3;
		    					break;
		    					}
		    				}
		    			}
		    		
		    		if (ldata)	
	    				AddFormatTagINT32(TAG_NEWLINE_CLEAR, ldata);
					else
						AddFormatTag(TAG_NEWLINE);

	    			// reset last space flag, since a new paragraph may not start with a space
	    			m_last_space = TRUE;
	    			}
	    		break;
	    	case STR_B:
				PushBold();
	    		break;
	    	case STR_B|STR_END:
				PopBold();
				break;
	    	case STR_I:
				PushItalic();
	    		break;
	    	case STR_I|STR_END:
				PopItalic();
	    		break;
	    	case STR_U:
				PushUnderline();
	    		break;
	    	case STR_U|STR_END:
				PopUnderline();
	    		break;
	    	case STR_TT:
				PushFixed();
	    		break;
	    	case STR_TT|STR_END:
				PopFixed();
	    		break;
	    		
	// handle heading levels
	    	case STR_H1:
				AddBeginParagraph( m_default_align, 1, 1, 0, BULLET_NONE );
	    		AddFormatTagINT32( TAG_HEADING, 1);
				PushFontSize(6);
				PushBold();
    			break;	
	    	case STR_H1|STR_END:
				PopBold();
				PopFontSize();
				AddFormatTagINT32( TAG_HEADING|TAG_END, 1);
				AddEndParagraph();
	    		break;	
	    	case STR_H2:
				AddBeginParagraph( m_default_align, 1, 1, 0, BULLET_NONE );
	    		AddFormatTagINT32( TAG_HEADING, 2);				
				PushFontSize(5);
				PushBold();    			
    			break;				
	    	case STR_H2|STR_END:
				PopBold();
				PopFontSize();
    			AddFormatTagINT32( TAG_HEADING|TAG_END, 2);		
	    		AddEndParagraph();
	    		break;	
	    	case STR_H3:
	    		AddBeginParagraph( m_default_align, 1, 1, 0, BULLET_NONE );
	    		AddFormatTagINT32( TAG_HEADING, 3);		
    			PushFontSize(4);
				PushBold();
    			break;
    		case STR_H3|STR_END:
				PopBold();
				PopFontSize();
    			AddFormatTagINT32( TAG_HEADING|TAG_END, 3);		
	    		AddEndParagraph();
	    		break;		
	    	case STR_H4:
	    		AddBeginParagraph( m_default_align, 1, 1, 0, BULLET_NONE );
	    		AddFormatTagINT32( TAG_HEADING, 4);		
    			PushFontSize(3);
				PushBold();
    			break;
    		case STR_H4|STR_END:
				PopBold();
				PopFontSize();
    			AddFormatTagINT32( TAG_HEADING|TAG_END, 4);		
	    		AddEndParagraph();
	    		break;		
	    	case STR_H5:
	    		AddBeginParagraph( m_default_align, 1, 1, 0, BULLET_NONE );
	    		AddFormatTagINT32( TAG_HEADING, 5);		
    			PushFontSize(2);
				PushBold();
    			break;
    		case STR_H5|STR_END:
				PopBold();
				PopFontSize();
    			AddFormatTagINT32( TAG_HEADING|TAG_END, 5);		
	    		AddEndParagraph();
	    		break;		
	    	case STR_H6:
	    		AddBeginParagraph( m_default_align, 1, 1, 0, BULLET_NONE );
	    		AddFormatTagINT32( TAG_HEADING, 6);		
				PushFontSize(1);
				PushBold();
    			break;
    		case STR_H6|STR_END:
				PopBold();
				PopFontSize();
    			AddFormatTagINT32( TAG_HEADING|TAG_END, 6);		
	    		AddFormatTag( TAG_PARAGRAPH|TAG_END );
    			break;
	    	case STR_IMG:
#ifdef USES_OLE_CONTROLS
			case STR_OLECONTROL:
#endif
	    			{
	      			POSITION walk = attrib_list.GetHeadPosition();

					CString filename;
#ifdef USES_OLE_CONTROLS
					CString ctrl_url;
	      			CString version;
					GUID guid;
					memset(&guid,0,sizeof(GUID));
#endif
	      			
	      			ALIGN_TYPE align = ALIGN_BASELINE;
	      			INT32 hspace = 0, vspace = 0, border = -1;
					INT32 width = 0, height = 0;
					BOOL is_mapped = FALSE;
	      					
		    		while(walk)
		    			{
		    			const CAttribute *a = (const CAttribute *)attrib_list.GetNext(walk);
		    			if (a->m_name == "src")
		    				{
		    				filename = CombineURL(m_base_url, a->m_value);
		    				}
		    			else if (a->m_name == "align")
		    				{
		    				CString v = a->m_value;
		    				v.MakeLower();
		    				
							align = ParseAlignType( v );
							if (align == ALIGN_NONE)
								align = ALIGN_BASELINE;
		    				}
		    			else if (a->m_name == "hspace")
		    				{
		    				hspace = StringToINT32( a->m_value, FALSE);
		    				}
		    			else if (a->m_name == "vspace")
		    				{
		    				vspace = StringToINT32( a->m_value, FALSE);
		    				}
		    			else if (a->m_name == "border")
		    				{
		    				border = StringToINT32( a->m_value, FALSE);
		    				}
		    			else if (a->m_name == "height")
		    				{
							height = StringToINT32( a->m_value, FALSE);
		    				}
		    			else if (a->m_name == "width")
		    				{
		    				width = StringToINT32( a->m_value, FALSE);
		    				}		    		
						else if (a->m_name == "ismap")
							{
							is_mapped = TRUE;
							}
		    			
#ifdef USES_OLE_CONTROLS
						else if (a->m_name == "clsid")
							{
							CString t = a->m_value;

							// find some way of converting a->m_value to a guid
							LPTSTR v = t.GetBuffer( t.GetLength() );
						 	IIDFromString(v , &guid);
							t.ReleaseBuffer();
							}
						else if (a->m_name == "olesrc")
							{
							ctrl_url = a->m_value;
							}
						else if (a->m_name == "version")
							{
							version  = a->m_value;
							}
#endif
						}

#ifdef USES_OLE_CONTROLS
					if (strid ==  STR_OLECONTROL)
						{
		    			new(m_taglist) CTagOLEControl(align, filename, width, height, hspace, vspace, guid, ctrl_url, version);
						}		    			
					else
#endif
						{
		    			new(m_taglist) CTagImage(align, filename, width, height, hspace, vspace, border, is_mapped, m_bPreformatted <= 0);
						}
					
					if (align != ALIGN_LEFT && align != ALIGN_RIGHT)
						{
						m_last_space = FALSE;
						}
		    		break;
		    		}
		    	break;
		    case STR_FONT:
		    		{
		    		POSITION walk = attrib_list.GetHeadPosition();
	      			INT32 ldata = 0;
	      					
		    		while(walk)
		    			{
		    			const CAttribute *a = (const CAttribute *)attrib_list.GetNext(walk);
		    			if (a->m_name == "size")
		    				{
		    				if (a->m_value[0] == '+')
		    					ldata = m_BaseFontSize + StringToINT32( ((const char *)a->m_value) + 1, FALSE);
		    				else if (a->m_value[0] == '-')
		    					ldata = m_BaseFontSize - StringToINT32( ((const char *)a->m_value) + 1, FALSE);
		    				else
		    					ldata =  StringToINT32(a->m_value, FALSE);
		    				if (ldata == 0)
		    					ldata = m_BaseFontSize;	
		    				}
		    			}
					PushFontSize(ldata);
		    		}
		    	break;
		    case STR_FONT|STR_END:
				PopFontSize();
		    	break;
		    case STR_BASEFONT:
		    		{
		    		POSITION walk = attrib_list.GetHeadPosition();
	      			INT32 ldata = 0;
	      					
		    		while(walk)
		    			{
		    			const CAttribute *a = (const CAttribute *)attrib_list.GetNext(walk);
		    			if (a->m_name == "size")
		    				{
		    				ldata =  StringToINT32(a->m_value, FALSE);
		    				}
		    			}
		    		
		    		m_BaseFontStack.Push(m_BaseFontSize);
		    		if (ldata < 1 || ldata > 7)
		    			ldata = 3;
		    		m_BaseFontSize = ldata;	
		    		}
		    	break;
		    case STR_BASEFONT|STR_END:
		    	if (m_BaseFontStack.GetSize() > 0)
		    		m_BaseFontSize = m_BaseFontStack.Pop();
		    	break;
		    case STR_CENTER:
				AddFormatTag( TAG_NEWLINE );
				AddFormatTag( TAG_CENTER );
		    	break;
		    case STR_CENTER|STR_END:
				AddFormatTag( TAG_NEWLINE );
				AddFormatTag( TAG_CENTER|TAG_END );
		    	break;	
			case STR_TABLE:
					{
		    		POSITION walk = attrib_list.GetHeadPosition();
					CTagTable * table = new (m_taglist) CTagTable();

	      			while(walk)
						{
		    			const CAttribute *a = (const CAttribute *)attrib_list.GetNext(walk);
						if (a->m_name == "colspec")
							{
							ParseColspec( table, a->m_value);
							}
						else if (a->m_name == "units")
							{
							if (a->m_value == "pixel")
								table->m_units = unit_pixel;
							else if (a->m_value == "en")
								table->m_units = unit_en;
							else if (a->m_value == "relative")
								table->m_units = unit_relative;
							}
						else if (a->m_name == "width")
							{
							table->m_table_width = StringToINT32(a->m_value, FALSE);
							}
						else if (a->m_name == "cellpadding")
							{
							table->m_cell_padding = StringToINT32(a->m_value, FALSE);
							}
						else if (a->m_name == "border")
							{
							if (a->m_value.IsEmpty())
								table->m_border_width = 1;
							else
								table->m_border_width = StringToINT32(a->m_value, FALSE);
							}
						else if (a->m_name == "dp")
							{
							table->m_decimal_point = a->m_value[0];
							}
						}
					m_table_stack.AddTail( table );
					m_cur_table = table;
					}
				break;
			case STR_TABLE|STR_END:
				if (m_cur_table)
					{
					if (m_cur_table->m_save_format)
						EndTableCell();

					m_cur_table->m_parsed_whole_table = TRUE;
					m_table_stack.RemoveTail();
					if (!m_table_stack.IsEmpty())
						{
						m_cur_table = (CTagTable *)m_table_stack.GetTail();
						}
					else
						{
						m_cur_table = NULL;
						}

					AddFormatTag(TAG_TABLE|TAG_END);
					}
				break;
			case STR_TR:
				BeginTableRow( attrib_list );
				break;
			case STR_TD:
			case STR_TH:
				BeginTableCell( strid, attrib_list );
				break;
			case STR_TD|STR_END:
			case STR_TH|STR_END:
			case STR_TR|STR_END:
				m_last_space = TRUE;
				break;
#ifdef _MACINTOSH
		    case STR_QUICKTIME:
		    		{
		    		POSITION walk = attrib_list.GetHeadPosition();
	      			CString filename;
	      			
	      			INT32 ldata = 0;
	      			INT32 hspace = 0, vspace = 0;
	      			INT32 height = 0, width = 0;
	      					
		    		while(walk)
		    			{
		    			const CAttribute *a = (const CAttribute *)attrib_list.GetNext(walk);
		    			if (a->m_name == "src")
		    				{
		    				filename = CombineURL(m_base_url, a->m_value);
		    				}
		    			else if (a->m_name == "align")
		    				{
		    				CString v = a->m_value;
		    				v.MakeLower();
		    				
		    				if (v == "left")
		    					ldata = 1;
		    				else if (v == "right")
		    					ldata = 2;
		    				}
		    			else if (a->m_name == "hspace")
		    				{
		    				hspace = StringToINT32( a->m_value );
		    				}
		    			else if (a->m_name == "vspace")
		    				{
		    				vspace = StringToINT32( a->m_value );
		    				}
		    			else if (a->m_name == "height")
		    				{

		    				height = StringToINT32( a->m_value);
		    				}
		    			else if (a->m_name == "width")
		    				{
		    				width = StringToINT32( a->m_value );
		    				}
		    			}
		    			
		    		CTag * p = new(m_taglist) CTagQuicktime(m_plain_text.GetLength(), ldata, filename, height, width, hspace, vspace);
				
				}
#endif
		    	break;
			}

    	// remove everything from attrib_list
    	POSITION walk = attrib_list.GetHeadPosition();   
    	while(walk)
    		{
    		CAttribute * p = (CAttribute *)attrib_list.GetNext(walk);
    		delete p;
    		}
    	attrib_list.RemoveAll();	 		
     	}

	Notify(CHANGEFLAG_ADD_TEXT);
	m_access_mutex.UnlockWrite();
   	return LOAD_STATE_COMPLETE;
  	}

void CParseHTML::BeginTableCell(int strid, const CPtrList& attrib_list )
	{
	if (!m_cur_table )
		return;

	if (m_cur_table->m_save_format)
		EndTableCell();

	if (!m_cur_table->m_cur_table_row)
		{
		CPtrList blank;
		BeginTableRow(blank);
		}

	CSaveCellFormat *save = DEBUG_NEW CSaveCellFormat;

	save->m_font_flags = m_font_flags;
	save->m_italic = m_italic;
	save->m_underline = m_underline;
	save->m_bold = m_bold;
	INT32 i, ilen = m_font_size_stack.GetSize();
	for(i=0;i<ilen;i++)
		save->m_font_size_stack.Add( m_font_size_stack[i] );
	
	m_cur_table->m_save_format = save;

	CTagTableCell * cell = new (m_taglist) CTagTableCell( m_cur_table->m_cur_table_row );
	POSITION walk = attrib_list.GetHeadPosition();

	if (strid == STR_TH)
		{
		cell->m_align = ALIGN_CENTER;
		PushBold();
		}
	
	while(walk)
		{
		const CAttribute *a = (const CAttribute *)attrib_list.GetNext(walk);

		if (a->m_name == "colspan")
			{
			cell->m_col_span = StringToINT32( a->m_value, FALSE);
			}
		else if (a->m_name == "rowspan")
			{
			cell->m_row_span = StringToINT32( a->m_value, FALSE);
			}
		else if (a->m_name == "align")
			{
			cell->m_align = ParseAlignType(a->m_value);
			}
		else if (a->m_name == "dp")
			{
			cell->m_decimal_point = a->m_value[0];
			}
		else if (a->m_name == "valign")
			{
			cell->m_valign = ParseAlignType(a->m_value);
			}
		else if (a->m_name == "nowrap")
			{
			cell->m_no_wrap = TRUE;
			}
		else if (a->m_name == "width")
			{
			cell->m_specified_width = StringToINT32( a->m_value, FALSE);
			}
		}
	m_cur_table->m_cur_column+= cell->m_col_span;

	if (m_cur_table->m_num_columns < m_cur_table->m_cur_column)
		m_cur_table->m_num_columns = m_cur_table->m_cur_column;

	ilen = cell->m_row_span;	
	for (i=1;i<ilen;i++)
		{
		INT32 index = i + m_cur_table->m_cur_row;

		while (index > m_cur_table->m_row_span_add.GetUpperBound())
			m_cur_table->m_row_span_add.Add(0);

		(m_cur_table->m_row_span_add[index])+= cell->m_col_span;
		}
	}

void CParseHTML::EndTableCell( )
	{
	if (!m_cur_table)
		return;

	CSaveCellFormat *save = m_cur_table->m_save_format;
	if (!save)
		return;

	m_font_flags= save->m_font_flags;
	m_italic = save->m_italic;
	m_underline = save->m_underline;
	m_bold = save->m_bold;

	m_font_size_stack.RemoveAll();

	INT32 i, ilen = save->m_font_size_stack.GetSize();
	for(i=0;i<ilen;i++)
		m_font_size_stack.Add( save->m_font_size_stack[i] );
	
	delete save;

	m_cur_table->m_save_format = NULL;
	}

void CParseHTML::BeginTableRow( const CPtrList& attrib_list )
	{
	if (m_cur_table)
		{
		if (m_cur_table->m_save_format)
			EndTableCell();

		if (m_cur_table->m_cur_table_row)
			{
			// end the previous row
			m_cur_table->m_cur_table_row = NULL;
			}

		m_cur_table->m_cur_row++;

		if (m_cur_table->m_cur_row > m_cur_table->m_row_span_add.GetUpperBound())
			m_cur_table->m_cur_column = 0;
		else
			m_cur_table->m_cur_column = m_cur_table->m_row_span_add[m_cur_table->m_cur_row];

		CTagTableRow * row = new (m_taglist) CTagTableRow( m_cur_table );
		POSITION walk = attrib_list.GetHeadPosition();
		while(walk)
			{
			const CAttribute *a = (const CAttribute *)attrib_list.GetNext(walk);
			
			if (a->m_name == "align")
				{
				row->m_align = ParseAlignType( a->m_value );
				}
			else if (a->m_name == "dp")
				{
				row->m_decimal_point = a->m_value[0];
				}
			else if (a->m_name == "valign")
				{
				row->m_valign = ParseAlignType( a->m_value );
				}
			else if (a->m_name == "nowrap")
				{
				row->m_no_wrap = TRUE;
				}
			}
		m_cur_table->m_cur_table_row = row;
		}
	}

LOAD_STATE CParseHTML::OnEndOfFile()
	{
	AddFormatTag( TAG_NEWLINE );
	
	Notify(CHANGEFLAG_DONE);

	switch(GetLoadState())
		{
		case LOAD_STATE_LOADING:
			return LOAD_STATE_COMPLETE;
		case LOAD_STATE_ABORTED:
		case LOAD_STATE_NOT_LOADED:
			return LOAD_STATE_ABORTED;
		case LOAD_STATE_COMPLETE:
			return LOAD_STATE_COMPLETE;
		default:
			ASSERT(FALSE);
			return LOAD_STATE_ABORTED;
		}
	}

#ifdef USES_OLE_CONTROLS

CTagOLEControl::CTagOLEControl(ALIGN_TYPE align, const char *url, INT32 width, INT32 height, INT32 hspace, INT32 vspace, const CLSID& clsid, LPCSTR ocx_url, LPCSTR version)
	 : CTag(TAG_OLECONTROL)
	{ 
static INT32 parse_id = 0; 

	m_align = align; 
	m_url = url; 
	m_hspace = hspace; 
	m_vspace = vspace; 
	m_height = height; 
	m_width = width; 
	m_parse_id = parse_id++;
	m_clsid = clsid;
	m_ocx_url = ocx_url;
	m_version = version;
	}
	
#endif

CAttribute::CAttribute(const char *name, INT32 name_len, const char *value, INT32 value_len)
	: m_name(name, name_len), m_value(value, value_len)
	{
	m_name.MakeLower();
	}
	
CAttribute::~CAttribute()
	{
	}

void CParseHTML::ClearTagList()
	{
	m_taglist.RemoveAll();
	}
	
CTag* CParseHTML::AddFormatTag(int type)
	{
	return new(m_taglist) CTag(type);
	}		

CTag* CParseHTML::AddFormatTagINT32(int type, INT32 int32)
	{
	return new (m_taglist) CTagINT32(type, int32);
	}

CTag* CParseHTML::AddFormatTagCString(int type, const char *string)
	{
	return new (m_taglist) CTagCString(type, string);
	}

CTag* CParseHTML::AddFormatTagImage(ALIGN_TYPE align, const char *url, INT32 hspace, INT32 vspace, INT32 border, INT32 height, INT32 width, BOOL is_mapped)
	{
	return new (m_taglist) CTagImage(align, url, hspace, vspace, border, height, width, is_mapped, m_bPreformatted <= 0);
	}

CTag* CParseHTML::AddFormatTagText(INT32 startpos, INT32 textlen, INT32 font_flags)
	{
	if (textlen > 0)
		return new (m_taglist) CTagText(startpos, textlen, font_flags);
	else
		return NULL;
	}

CTag* CParseHTML::AddBeginParagraph( ALIGN_TYPE align, INT32 space_after, INT32 space_before, INT32 indent_first_line, INT32 extra_indent)
	{
	if (m_current_paragraph)
		{
		AddEndParagraph();
		}

	m_last_space = TRUE;
	return  (m_current_paragraph = new (m_taglist) CTagBeginParagraph( align, space_after, space_before, m_current_indent + extra_indent, indent_first_line));
	}

CTag* CParseHTML::AddEndParagraph()
	{
	if (m_current_paragraph)
		{
		m_last_space = TRUE;
		m_current_paragraph = NULL;
		return AddFormatTag( TAG_PARAGRAPH|TAG_END);
		}
	else
		return NULL;
	}

CString CParseHTML::GetTitle()
	{
	m_access_mutex.LockRead();
	CString title = m_title;
	m_access_mutex.UnlockRead();
	return title;
	}

CString CParseHTML::GetBackgroundPicture()
	{
	m_access_mutex.LockRead();
	CString title = m_background_url;
	m_access_mutex.UnlockRead();
	return title;
	}

INT32 CParseHTML::GetBackgroundColor()
	{
	m_access_mutex.LockRead();
	INT32 color = m_back_color;
	m_access_mutex.UnlockRead();
	return color;	
	}
		// returns the background color, -1 if the document does not specify a backgound color
INT32 CParseHTML::GetTextColor()
	{
	m_access_mutex.LockRead();
	INT32 color = m_text_color;
	m_access_mutex.UnlockRead();
	return color;	
	}
	
INT32 CParseHTML::GetHotlinkColor()
	{
	m_access_mutex.LockRead();
	INT32 color = m_hotlink_color;
	m_access_mutex.UnlockRead();
	return color;	
	}
	
INT32 CParseHTML::GetOldHotlinkColor()
	{
	m_access_mutex.LockRead();
	INT32 color = m_old_hotlink_color;
	m_access_mutex.UnlockRead();
	return color;	
	}

CTagTable::CTagTable()
	: CTag( TAG_TABLE )
	{
	m_table_align = ALIGN_NONE; 
	m_decimal_point = '.';

	m_units = unit_relative;

	m_cur_column = 0;
	m_num_columns = 0;
	m_table_width = 0;
	m_border_width = 0;
	m_no_wrap = FALSE;
	m_cur_row = 0;
	m_cell_padding = 1;

	m_save_format = NULL;
	m_parsed_whole_table = FALSE;
	m_cur_table_row = NULL;
	}

CTagTableCaption::CTagTableCaption() : CTag( TAG_TABLECAPTION )
	{
	m_align = ALIGN_LEFT;
	}

CTagTableRow::CTagTableRow( CTagTable *current_table ) : CTag( TAG_TABLEROW)
	{
	m_align = ALIGN_LEFT;
	m_decimal_point = current_table->m_decimal_point;
	m_valign = ALIGN_CENTER;
	m_no_wrap = current_table->m_no_wrap;
	}

CTagTableCell::CTagTableCell( CTagTableRow *current_row ) : CTag( TAG_TABLECELL)
	{
	m_col_span = 1;
	m_row_span = 1;
	m_align = current_row->m_align;
	m_no_wrap = current_row->m_no_wrap;
	m_valign = current_row->m_valign;
	m_specified_width = 0;
	}


void CParseHTML::PushBold()
	{
	m_bold++;
	if (m_bold == 1)
		m_font_flags |= FONTFLAG_BOLD;
	}

void CParseHTML::PopBold()
	{
	m_bold--;
	if (m_bold == 0)
		m_font_flags &= ~FONTFLAG_BOLD;
	}

void CParseHTML::PushItalic()
	{
	m_italic++;
	if (m_italic == 1)
		m_font_flags |= FONTFLAG_ITALIC;
	}

void CParseHTML::PopItalic()
	{
	m_italic--;
	if (m_italic == 0)
		m_font_flags &= ~FONTFLAG_ITALIC;
	}

void CParseHTML::PushUnderline()
	{
	m_underline++;
	if (m_underline == 1)
		m_font_flags |= FONTFLAG_UNDERLINE;
	}

void CParseHTML::PopUnderline()
	{
	m_underline--;
	if (m_underline == 0)
		m_font_flags &= ~FONTFLAG_UNDERLINE;
	}
	    		
void CParseHTML::PushFixed()
	{	    		
	m_font_fixed++;
	if (m_font_fixed == 1)
		m_font_flags |= FONTFLAG_FIXED;
	}

void CParseHTML::PopFixed()
	{
	m_font_fixed--;
	if (m_font_fixed == 0)
		m_font_flags &= ~FONTFLAG_FIXED;
	}

void CParseHTML::PushFontSize(INT32 size )
	{
	if (size < 1)
		size = 1;
	if (size > 7)
		size = 7;

	m_font_size_stack.Push( m_font_flags & FONTFLAG_SIZE);
	m_font_flags &= ~FONTFLAG_SIZE;
	m_font_flags |= (size & FONTFLAG_SIZE);
	}

void CParseHTML::PopFontSize()
	{
	if (m_font_size_stack.GetSize() > 0)
		{
		INT32 size = m_font_size_stack.Pop();
		m_font_flags &= ~FONTFLAG_SIZE;
		m_font_flags |= (size & FONTFLAG_SIZE);
		}		
	}


POSITION CParseHTML::GetFirstTagPos()
	{ 
	m_access_mutex.LockRead();
	POSITION pos = m_taglist.GetHeadPosition(); 
	m_access_mutex.UnlockRead();		
	return pos;
	}

const CTag * CParseHTML::GetNextTag(POSITION &walk)
	{
	m_access_mutex.LockRead();		
	const CTag *tag = (const CTag *)m_taglist.GetNext(walk);
	m_access_mutex.UnlockRead();		
	return tag;
	}


POSITION CParseHTML::FindTagPos( const CTag *tag)
	{
	m_access_mutex.LockRead();
	POSITION pos = m_taglist.Find( (CContainerItem *) tag); 
	m_access_mutex.UnlockRead();		
	return pos;
	}

CBigString CParseHTML::GetPlainText() 
	{
	return m_plain_text;
	}

#include "cross_p.h"

#include "fmthtml.h"

void GetTableSize( FORMAT_PARAMS& fp, CFormatHTML *format_html, 
		CMimeObject *parsetags, POSITION& walk, 
		CInt32Array& i_max_column_widths, CInt32Array& i_min_column_widths,
		INT32& i_min_table_size, INT32& i_max_table_size);

void GetColumnSize( FORMAT_PARAMS& fp, CFormatHTML *format_html, CMimeObject *parsetags, POSITION& walk, 
	INT32& min_size, INT32& max_size)
	{
	min_size = 0;
	max_size = 0;

	INT32 cur_indent = 0;
	INT32 next_line = 0;


	INT32 cur_line_min = 0;
	INT32 cur_line_max = 0;
	
	while(walk)
		{
		POSITION oldwalk = walk;
		
		parsetags->DEBUG_LOCK();
		const CTag * ptag = (const CTag *)parsetags->GetNextTag(walk);
		parsetags->Unlock();
		
		switch(ptag->m_type)
			{
			case TAG_TABLE:
					{
					walk = oldwalk;

					CInt32Array max_column_widths, min_column_widths;
					INT32 min_table_size, max_table_size;
					
					GetTableSize(fp, format_html, parsetags, walk, max_column_widths, min_column_widths, min_table_size, max_table_size);
					Assign_Max( min_size, min_table_size );
					Assign_Max( max_size, max_table_size );
					}
				break;
			case TAG_TABLE|TAG_END:
			case TAG_TABLEROW:
			case TAG_TABLEROW|TAG_END:
			case TAG_TABLECELL:
			case TAG_TABLECELL|TAG_END:

				Assign_Max(min_size, cur_line_min + cur_indent);
				Assign_Max(max_size, cur_line_max + cur_indent);

				if (max_size > 10 * min_size )
					max_size = 10 * min_size;

				walk = oldwalk;
				return;
			case TAG_TEXT:
					{
					const CTagText *ptagtext = (const CTagText *) ptag;
					CString tstring;
					INT32 width;

					format_html->m_plain_text.GetString(tstring,  ptagtext->m_startpos, ptagtext->m_textlen);

					width = format_html->m_format_font->GetStringWidth(ptagtext->m_font_flags, tstring, tstring.GetLength() );

	  				if ( ptagtext->m_font_flags & FONTFLAG_NO_WRAP )
						{
						cur_line_min += width;
						cur_line_max += width;
						}
					else
						{
						cur_line_min += width;

						if (cur_line_min > format_html->m_minimum_text_width)
							cur_line_min = format_html->m_minimum_text_width;

						cur_line_max += width;
						}
					}
				break;
#ifdef USES_OLE_CONTROLS
			case TAG_OLECONTROL:
					{
					const CTagOLEControl *ptagole = (const CTagOLEControl *) ptag;

					INT32 width;
					
					width = ptagole->m_width;
					
					Assign_Max(cur_line_min, width);
					cur_line_max += width;
					}
				break;
#endif
			case TAG_IMAGE:
					{
					const CTagImage *ptagimage = (const CTagImage *) ptag;

					void *p = NULL;
					INT32 width;
					
					if (ptagimage->m_width != 0)
						width = ptagimage->m_width;
					else
						{
						CSize size;
						VERIFY( fp.m_picture_info.GetPictureSize((LPCSTR)(ptagimage->m_url), size) );
						width = size.cx;
						}

					if (ptagimage->m_can_break == FALSE)
						{
						cur_line_min += width;
						cur_line_max += width;
						}
					else
						{
						Assign_Max(cur_line_min, width);
						cur_line_max += width;
						}
					}
				break;
			case TAG_NEWLINE:
			case TAG_NEWLINE_CLEAR:
			case TAG_NEWLINE_CLEAR_EN:
			case TAG_NEWLINE_CLEAR_PIXEL:
			case TAG_NEWLINE_HARD_BREAK:
				Assign_Max(min_size, cur_line_min + cur_indent);
				Assign_Max(max_size, cur_line_max + cur_indent);

				cur_line_min = 0;
				cur_line_max = 0;

				cur_indent = next_line;
				break;
			case TAG_CONDITIONAL_BREAK:
				Assign_Max(min_size, cur_line_min + cur_indent);
				Assign_Max(max_size, cur_line_max + cur_indent);

				cur_line_min = 0;
				break;
			case TAG_PARAGRAPH:
					{
					Assign_Max(min_size, cur_line_min + cur_indent);
					Assign_Max(max_size, cur_line_max + cur_indent);
	
					cur_line_min = 0;
					cur_line_max = 0;

					const CTagBeginParagraph *ptagpara = (const CTagBeginParagraph *) ptag;

					cur_indent = (ptagpara->m_indent_amount + ptagpara->m_indent_first_line) * format_html->m_tab_size;
					next_line = (ptagpara->m_indent_amount) * format_html->m_tab_size;
					}
				break;
			case TAG_PARAGRAPH|TAG_END:
				cur_indent = 0;
				next_line = 0;
				break;
			case TAG_BULLET:
				cur_line_min += format_html->m_tab_size;
				cur_line_max += format_html->m_tab_size;
				break;
			}
		}
	}

void GetTableSize( FORMAT_PARAMS& fp, CFormatHTML *format_html, 
		CMimeObject *parsetags, POSITION& walk, 
		CInt32Array& max_column_widths, CInt32Array& min_column_widths,
		INT32& min_table_size, INT32& max_table_size)
	{
	CInt32Array row_span_array, col_span_array;

	parsetags->DEBUG_LOCK();
	const CTagTable * ptagtable = (const CTagTable *)parsetags->GetNextTag(walk);
	parsetags->Unlock();

	ASSERT(ptagtable->m_type == TAG_TABLE );

	INT32 border_size = ptagtable->m_border_width;

	INT32 padding = ptagtable->m_cell_padding;
	INT32 intercell_spacing = padding*2 + border_size;

	INT32 i,ilen,num_columns = ptagtable->m_num_columns;
	for(i=0;i<num_columns;i++)
		{
		min_column_widths.Add( 0 );
		max_column_widths.Add( 0 );
	  	row_span_array.Add(0);
		col_span_array.Add(1);
		}

	INT32 current_column = 0;

	while(walk)
		{
		POSITION oldwalk = walk;

		parsetags->DEBUG_LOCK();
		const CTag * ptag = (const CTag *)parsetags->GetNextTag(walk);	
		parsetags->Unlock();

		const CTagTableRow *ptagrow = (const CTagTableRow *)ptag;
		const CTagTableCell *ptagcell = (const CTagTableCell *)ptag;

		switch(ptag->m_type)
			{
			case TAG_TABLE|TAG_END:
					{
					min_table_size = border_size;
					max_table_size = border_size;

					for(i=0;i<num_columns;i++)
						{
						min_column_widths[i] += padding*2;
						max_column_widths[i] += padding*2;

						min_table_size += min_column_widths[i] + border_size;
						max_table_size += max_column_widths[i] + border_size; 
						}
					return;
					}
				break;
			case TAG_TABLEROW:
				for(i=0;i<num_columns;i++)
					{
					if (row_span_array[i] > 0)
						{
						if (--row_span_array[i] == 0)
							{
							INT32 j,jlen = col_span_array[i];

							for(j=0;j<jlen;j++)
								{
								row_span_array[i + j] = 0;
								col_span_array[i + j] = 1;
								}
							}
						}
					}
				current_column = 0;
				break;
			case TAG_TABLECELL:
					{
					// find a free cell to put this cel into
					while( row_span_array[ current_column ] > 0)
						{
						current_column += col_span_array[ current_column ];
						}

					row_span_array[ current_column ] = ptagcell->m_row_span;
					col_span_array[ current_column ] = ptagcell->m_col_span;

					INT32 min_width, max_width;
									
					GetColumnSize( fp, format_html, parsetags, walk, min_width, max_width );

					if (ptagcell->m_specified_width > 0)
						{
						if (min_width < ptagcell->m_specified_width)
							{
							// make the cell as big as the specified width and no bigger
							max_width = min_width = ptagcell->m_specified_width;
							}
						else
							{
							// otherwise just make it as small as possible
							max_width = min_width;
							}
						}

					ilen = ptagcell->m_col_span;
					
					min_width -= (ilen-1)* intercell_spacing;
					max_width -= (ilen-1)* intercell_spacing;

					INT32 min_w = min_width/ilen, max_w = max_width/ilen;
					
					Assign_Max( min_column_widths[current_column], min_w + min_width%ilen );
					Assign_Max( max_column_widths[current_column], max_w + max_width%ilen );

					for(i=1;i<ilen;i++)
						{
						Assign_Max( min_column_widths[current_column + i], min_w );
						Assign_Max( max_column_widths[current_column + i], max_w );
						}
					}
				break;
			}
		}
	}

#include "cross_p.h"

#ifndef _BITBLT_H_
#include "bitblt.h"
#endif

#ifndef _FMTHTML_H_
#include "fmthtml.h"
#endif

#ifndef _READGIF_H_
#include "readgif.h"	
#endif

#ifdef _WINDOWS
#include "resource.h"
#endif

#define INTERLACE	0x40	/* mask for bit signifying interlaced image */
#define COLORMAPFLAG	0x80	/* mask for bit signifying colormap presence */
#define MAX_CODES   4095

class DECODE_STRUCT
	{
public:
	BYTE stack[MAX_CODES + 1];            /* Stack for storing pixels */
	BYTE suffix[MAX_CODES + 1];           /* Suffix table */
	UINT prefix[MAX_CODES + 1];           /* Prefix linked list */

	int bad_code_count;

		/* Static variables */
	UINT curr_size;                     /* The current code size */
	UINT clear;                         /* Value for a clear code */
	UINT ending;                        /* Value for a ending code */
	UINT newcodes;                      /* First available code */
	UINT top_slot;                      /* Highest code for current size */
	UINT slot;                          /* Last read code */


	BOOL last_code_clear;				/* TRUE if the last code was a clear code */

	/* The following static variables are used
	 * for seperating out codes
	 */
	UINT navail_bytes;              /* # bytes left in block */
	UINT nbits_left;                /* # bits left in current byte */
	//UINT ncode_left;				/* # bits left to fill in the current code */
	UINT bits_left;
	UINT bits_read;
	UINT code_bit;                        /* Current code */
	BYTE b1;

	UINT amount_needed;
	BYTE buffer[768];            	/* Read buffer */
	BYTE *pbytes;                   /* Pointer to next byte in block */

	BYTE *line_buf;

	UINT oc, fc;				/* old code */
	INT bufcnt;					// number of bytes left to process on this line
	INT linewidth;				// number of bytes on one line
	INT current_y;				// current pixel line

	UINT size;					// minimum code size

	BYTE *sp;					// stack pointer

	BYTE *bufptr;				// pointer to current pixel on current line

	UINT remainder;				// remaining code from the last block

	CFsDither *m_dither;		// dither routine
	};
 
inline UINT ConstructUINT(BYTE a, BYTE b)
	{
	return (((UINT)b)<<8) + a;
	}

CGifPicture::CGifPicture( LPCSTR url, LPCSTR mime_type, const PICTURE_FORMAT_INFO& requested_format )
	: CPicture(url, mime_type, requested_format)
	{
	m_decode_struct = DEBUG_NEW DECODE_STRUCT;
	memset(m_decode_struct,0,sizeof(DECODE_STRUCT) );

	m_picture_state = loading_header;
	m_decode_struct->sp = m_decode_struct->stack;

	m_bitmap = NULL;
	m_remapped_bitmap = NULL;
	m_broken_gif = FALSE;
	m_transp_color_index = -1;

	if (requested_format.m_use_standard_palette)
		m_use_standard_palette = TRUE;

	// need 13 bytes for the header
	m_decode_struct->amount_needed = 13;
	}

CGifPicture::~CGifPicture()
	{
	if (m_decode_struct)
		{
		if (m_decode_struct->line_buf)
			delete [] m_decode_struct->line_buf;
		delete m_decode_struct;
		}

	if (m_bitmap)
		delete [] m_bitmap;
	if (m_remapped_bitmap)
		delete [] m_remapped_bitmap;
	}


void CGifPicture::GetPalette( UINT32 palette[256] ) const
	{
	ASSERT_LOCKED(this);
	}

void CGifPicture::GetFormatInfo(PICTURE_FORMAT_INFO& format_info) const
	{
	ASSERT_LOCKED(this);

	format_info.m_bits_per_pixel = 8;
	format_info.m_use_standard_palette = TRUE;
	format_info.m_bytes_per_line = RoundUp4(m_size.cx);
	}

void CGifPicture::out_line(BYTE *input_pixels, UINT current_y)
	{
	long map_y = 0;
	
	UINT height = m_size.cy;
	UINT width = m_size.cx;
	UINT dup = 1;

	INT32 bytes_per_line = RoundUp4(width);

	if (m_is_interlaced)
		{
		UINT pass2ofs = (height + 7)/8,
			pass3ofs = pass2ofs + (height + 3)/8,
			pass4ofs = pass3ofs + (height+1)/4;
		
		if (current_y < pass2ofs)
			{
			map_y = current_y * 8;
			dup = 8;
			}
		else if (current_y < pass3ofs)
			{
			map_y = (current_y - pass2ofs)*8 + 4;
			dup = 4;
			}
		else if (current_y < pass4ofs)
			{
			map_y = (current_y - pass3ofs)*4 + 2;
			dup = 2;
			}
		else
			map_y = (current_y - pass4ofs)*2 + 1;

		if (map_y + dup > height)
			dup = height - map_y;

		UINT i;

		BYTE *p = m_bitmap + (bytes_per_line * map_y);

		// copy line w/o translation to untranslated bitmap
		for (i=0;i<dup;i++)
			{
			memcpy(p, input_pixels, m_size.cx );
			p+= bytes_per_line;
			}

		p = m_remapped_bitmap  + ( bytes_per_line * map_y);

		for (i=0;i<dup;i++)
			{
			// copy line with translation to translated bitmap
			TranslateIndexs(m_original_palette, input_pixels, p, m_size.cx, m_transp_color_index );
			p+= bytes_per_line;
			}
		}
	else
		{
		map_y = current_y;
		BYTE *p = m_bitmap + (bytes_per_line * map_y);

		memcpy(p, input_pixels, m_size.cx );

		p = m_remapped_bitmap  + ( bytes_per_line * map_y);

		m_decode_struct->m_dither->Dither( input_pixels, p, 1 );
		}
	}		
	
/* This function initializes the decoder for reading a new image.
 */
static UINT init_exp(DECODE_STRUCT& ds, UINT size)
   {
   ds.curr_size = size + 1;
   ds.top_slot = 1 << ds.curr_size;
   ds.clear = 1 << size;
   ds.ending = ds.clear + 1;
   ds.slot = ds.newcodes = ds.ending + 1;
   ds.navail_bytes = ds.nbits_left = 0;
   return(0);
   }

BOOL CGifPicture::IsHeaderRead()  const
	{
	ASSERT_LOCKED(this);

	if (m_picture_state > loading_header)
		{
		return TRUE;
		}
	else
		{
		return FALSE;	// we don't know how big this picture is
		}
	}
	
CSize CGifPicture::GetSize() const
	{
	ASSERT_LOCKED(this);

	if (GetLoadState() == LOAD_STATE_ABORTED)
		{
		if (m_picture_state > loading_header)
			return m_size;
		else
			return CSize(32,32);
		}
	else if (m_picture_state > loading_header)
		{
		return m_size;
		}
	else
		{
		/* Can't know bound rectangle at this point  */
		ASSERT(FALSE);
		return CSize(32,32);
		}
	}

BOOL CGifPicture::IsTransparent() const
	{
	ASSERT_LOCKED(this);

	if (GetLoadState() == LOAD_STATE_LOADING ||
		GetLoadState() == LOAD_STATE_ABORTED)
		return TRUE;
	else
		return m_transp_color_index != -1;
	}

LPBYTE CGifPicture::LockBitmapBits()
	{
	ASSERT_LOCKED(this);

	return m_remapped_bitmap;
	}

void CGifPicture::UnlockBitmapBits(LPBYTE bits)
	{
	}

static const UINT code_mask[13] = { 0,
							0x0001, 0x0003,
							0x0007, 0x000f,
							0x001f, 0x003f,
							0x007f, 0x00ff,
							0x01ff, 0x03ff,
							0x07ff, 0x0fff };

LOAD_STATE CGifPicture::OnReadData(LPCBYTE buffer, INT32 buffer_size)
	{
	UINT i,ilen,j;
	BYTE *p;
	DECODE_STRUCT& ds = *m_decode_struct;
	LOAD_STATE return_value;
	INT32 amount_to_read;
	INT32 buffersize;

use_remainder:

	if (buffer_size + ds.navail_bytes < ds.amount_needed)
		{
		memcpy(ds.buffer + ds.navail_bytes, buffer,
			buffer_size);
		ds.navail_bytes += buffer_size;

		return_value = LOAD_STATE_LOADING;
		goto exit;
		}

	amount_to_read = ds.amount_needed - ds.navail_bytes;
	memcpy(ds.buffer + ds.navail_bytes, buffer,
		amount_to_read);
	buffer_size -= amount_to_read;
	buffer += amount_to_read;
	ds.navail_bytes = 0;

	switch(m_picture_state)
		{
		case loading_header:
			if (memcmp(ds.buffer, "GIF87a", 6) != 0 &&
				memcmp(ds.buffer, "GIF89a", 6) != 0)
				{
				return_value = LOAD_STATE_ABORTED;
				goto exit;
				}

			DEBUG_LOCK();
			
			ds.linewidth = m_size.cx  = ConstructUINT( ds.buffer[6 + 0], ds.buffer[6 +1] );
			m_size.cy = ConstructUINT( ds.buffer[6 + 2], ds.buffer[6 +3] );

			if (ds.buffer[6 + 4] & COLORMAPFLAG)
				{
				m_color_map_len =  2 << (ds.buffer[6 + 4] & 0x07);
		
				m_picture_state = loading_global_color_map;

				ds.amount_needed = ((UINT)m_color_map_len)*3;
				}
			else
				{
				m_picture_state = loading_extension_blocks;			
				ds.amount_needed = 1;
				}			

			Unlock();
			Notify( CHANGEFLAG_PICTURE_CHANGE );

			goto use_remainder;
		case loading_global_color_map:

			ilen = m_color_map_len;
			for(i=0,j=0;i<ilen;i++)	
				{
				m_original_palette[i].rgbRed = 	ds.buffer[j++];
				m_original_palette[i].rgbGreen = ds.buffer[j++];
				m_original_palette[i].rgbBlue = 	ds.buffer[j++];
				m_original_palette[i].rgbReserved = 0;
				}

			DEBUG_LOCK();
			m_picture_state = loading_extension_blocks;	
			Unlock();

			ds.amount_needed = 1;				

			goto use_remainder;

		case loading_extension_blocks:

			if (ds.buffer[0] == '!')
				{
				DEBUG_LOCK();
				m_picture_state = loading_begin_extension;	
				Unlock();

				ds.amount_needed = 2;	
				}
			else if (ds.buffer[0] == ',')
				{
				DEBUG_LOCK();
				m_picture_state = loading_local_header;
				Unlock();

				ds.amount_needed = 9;
				}
			else
				{
				return_value = LOAD_STATE_ABORTED;
				goto exit;
				}
			goto use_remainder;

		case loading_begin_extension:
			
			if (ds.buffer[0] == 0xF9)
				{
				// this is the only extension block we care about
				DEBUG_LOCK();
				m_picture_state = loading_transp_extension;
				Unlock();
				}
			else
				{
				DEBUG_LOCK();
				m_picture_state = loading_skip_extension;
				Unlock();
				}

			ds.amount_needed = ds.buffer[1]+1;
			goto use_remainder;

		case loading_transp_extension:

			DEBUG_LOCK();
			if (ds.buffer[0] & 1)
				{
				m_transp_color_index = ds.buffer[3];
				}

			ds.amount_needed = 
				ds.buffer[ ds.amount_needed - 1 ];

			if (ds.amount_needed == 0)				
				{
				ds.amount_needed = 1;
				m_picture_state = loading_extension_blocks;
				}
			else	
				{
				m_picture_state = loading_skip_extension;
				ds.amount_needed++;
				}
			Unlock();

			goto use_remainder;

		case loading_skip_extension:

			ds.amount_needed = 
				ds.buffer[ ds.amount_needed-1 ]+1;

			if (ds.amount_needed == 1)
				{
				// end of extension block
				ds.amount_needed = 1;
				DEBUG_LOCK();
				m_picture_state = loading_extension_blocks; 
				Unlock();
				}
			goto use_remainder;

		case loading_local_header:

		    m_is_interlaced = ((ds.buffer[8] & INTERLACE)!=0);
						
		    /* Read local colormap if header indicates it is present */
		    /* Note: if we wanted to support skipping images, */
		    /* we'd need to skip rather than read colormap for ignored images */
		    if (ds.buffer[8] & COLORMAPFLAG) 
		    	{
		      	m_color_map_len = 2 << (ds.buffer[8] & 0x07);

				ds.amount_needed = ((UINT)m_color_map_len)*3;
				DEBUG_LOCK();
				m_picture_state = loading_local_color_map;
				Unlock();
		      	}
			else
				{
				ds.amount_needed = 2;
				DEBUG_LOCK();
				m_picture_state = loading_start_picture;
				Unlock();
				}
			goto use_remainder;

		case loading_local_color_map:

			DEBUG_LOCK();
			ilen = m_color_map_len;
			for(i=0,j=0;i<ilen;i++)	
				{
				m_original_palette[i].rgbRed = 	ds.buffer[j++];
				m_original_palette[i].rgbGreen = ds.buffer[j++];
				m_original_palette[i].rgbBlue = 	ds.buffer[j++];
				m_original_palette[i].rgbReserved = 0;
				}
			ds.amount_needed = 2;
			m_picture_state = loading_start_picture;
			Unlock();
			goto use_remainder;

		case loading_start_picture:
		
			ds.size = ds.buffer[0];
			if (ds.size < 2 || ds.size > 9)
				{
				return_value = LOAD_STATE_ABORTED;
				goto exit;
				}

			init_exp( *m_decode_struct, ds.size);

			ds.line_buf = DEBUG_NEW BYTE[ m_size.cx + 1 ];
			ds.bufptr = ds.line_buf;
			ds.bufcnt = ds.linewidth;
			ds.amount_needed = (UINT)ds.buffer[1]+1;
			
			ds.code_bit = 0;
			ds.bits_read = 0;
			
			DEBUG_LOCK();
			m_picture_state = loading_picture;

			buffersize =  m_size.cy * RoundUp4(m_size.cx);

			if (buffersize == 0)	
				{
				//  empty GIF file
				Unlock();
				return_value = LOAD_STATE_COMPLETE;
				goto exit;
				}

			m_bitmap = DEBUG_NEW BYTE[ buffersize ];
			
			m_remapped_bitmap = DEBUG_NEW BYTE[ buffersize ];
			memset(m_remapped_bitmap, TRANS_COLOR, buffersize);

			m_decode_struct->m_dither = new CFsDither( m_original_palette, m_size.cx,
				RoundUp4(m_size.cx), m_transp_color_index );

			Unlock();
			goto use_remainder;

		case loading_picture:
			// here's a block of ds.amount_needed size... decode it
			
			ilen = ds.amount_needed;
			if (ilen == 0)
				{
				return_value = LOAD_STATE_ABORTED;
				goto exit;
				}

			p = ds.buffer;
			ds.amount_needed = 
				p[ --ilen ]+1;
						
			UINT c;

			while(TRUE)
				{
				UINT bits_needed = ds.curr_size - ds.bits_read;
				ASSERT(bits_needed >= 0);

				while (bits_needed > 0)
					{
					if (bits_needed <= ds.bits_left)
						{
						ds.code_bit |= ((UINT)ds.b1 << ds.bits_read);
						ds.bits_left -= bits_needed;
						ds.b1 >>= bits_needed;
						break;
						}
					
					if (ds.bits_left > 0)
						{
						ds.code_bit |= ((UINT)ds.b1 << ds.bits_read);
						ds.bits_read += ds.bits_left;
						bits_needed -= ds.bits_left;
						}
					if (ilen == 0)
						{
						// oh oh.. out of bits... 
						ds.bits_left = 0;
						goto use_remainder;
						}
					ds.b1 = *(p++);
					ilen--;
					ds.bits_left = 8;
					}

				c = ds.code_bit & code_mask[ds.curr_size];

				ds.bits_read = 0;
				ds.code_bit = 0;

				// c is assigned the next code, the loop is terminated if no more bytes are left to read

				if (c == ds.ending)
					{
		   			if (ds.bufcnt != ds.linewidth)
						{
		      			out_line(ds.line_buf, ds.current_y++);

						if (ds.current_y == m_size.cy)
							{
							return_value = LOAD_STATE_COMPLETE;
							goto exit;
							}
						}

					return_value = LOAD_STATE_COMPLETE;
					goto exit;
					}
			
			      /* If the code is a clear code, reinitialize all necessary items.
			       */

				if (ds.last_code_clear)
					{
					if (c == ds.clear)
						continue; 		// ignore additional clear codes

					/* Finally, if the code is beyond the range of already set codes,
	          			* (This one had better NOT happen...  I have no idea what will
	          		* result from this, but I doubt it will look good...) then set it
	          		* to color zero.
	          		*/

					if (c >= ds.slot)
						c = 0;

					ds.oc = ds.fc = c;

	         		/* And let us not forget to put the char into the buffer... And
	          		* if, on the off chance, we were exactly one pixel from the end
	          		* of the line, we have to send the buffer to the out_line()
	          		* routine...
	          		*/

	         		*ds.bufptr++ = (BYTE)c;
	         		if (--ds.bufcnt == 0)
	            		{
	            		out_line(ds.line_buf, ds.current_y++);
						Notify(CHANGEFLAG_PICTURE_CHANGE);
						if (ds.current_y == m_size.cy)
							{
							return_value = LOAD_STATE_COMPLETE;
							goto exit;
							}
						
	            		ds.bufptr = ds.line_buf;
	            		ds.bufcnt = ds.linewidth;
	            		}
					ds.last_code_clear = FALSE;
					}
				else if (c == ds.clear)
	         		{
	         		ds.curr_size = ds.size + 1;
	         		ds.slot = ds.newcodes;
	         		ds.top_slot = 1 << ds.curr_size;
					ds.last_code_clear = TRUE;
					}
				else
					{
		         	/* In this case, it's not a clear code or an ending code, so
		          	* it must be a code code...  So we can now decode the code into
		          	* a stack of character codes. (Clear as mud, right?)
		          	*/
		         	UINT code = c;

		         	/* Here we go again with one of those off chances...  If, on the
		          	* off chance, the code we got is beyond the range of those already
	          		* set up (Another thing which had better NOT happen...) we trick
	          		* the decoder into thinking it actually got the last code read.
	          		* (Hmmn... I'm not sure why this works...  But it does...)
	          		*/
	         		if (code >= ds.slot)
	            		{
	            		if (code > ds.slot)
							{
	               			++ds.bad_code_count;
							}
	            		code = ds.oc;
	            		*ds.sp++ = (BYTE)ds.fc;
	            		}

					/* Here we scan back along the linked list of prefixes, pushing
          			* helpless characters (ie. suffixes) onto the stack as we do so.
          			*/

			         while (code >= ds.newcodes)
			            {
			            *ds.sp++ = ds.suffix[code];
			            code = ds.prefix[code];
			            }

	         		/* Push the last character on the stack, and set up the new
	          		* prefix and suffix, and if the required slot number is greater
	          		* than that allowed by the current bit size, increase the bit
	          		* size.  (NOTE - If we are all full, we *don't* save the new
	          		* suffix and prefix...  I'm not certain if this is correct...
	          		* it might be more proper to overwrite the last code...
	          		*/

	         		*ds.sp++ = (BYTE)code;
	         		if (ds.slot < ds.top_slot)
	            		{
	            		ds.suffix[ds.slot] = (BYTE)(ds.fc = code);
	            		ds.prefix[ds.slot++] = ds.oc;
	            		ds.oc = c;
	            		}

	         		if (ds.slot >= ds.top_slot)
	            		if (ds.curr_size < 12)
	               			{
	               			ds.top_slot <<= 1;
	               			++ds.curr_size;
	               			} 

	         		/* Now that we've pushed the decoded string (in reverse order)
	          		* onto the stack, lets pop it off and put it into our decode
	          		* buffer...  And when the decode buffer is full, write another
	          		* line...
	          		*/

	         		while (ds.sp > ds.stack)
	            		{
	            		*ds.bufptr++ = *(--ds.sp);
	            		if (--ds.bufcnt == 0)
	               			{
	            			out_line(ds.line_buf, ds.current_y++);
							ds.bufptr = ds.line_buf;
	               			ds.bufcnt = ds.linewidth;
							Notify(CHANGEFLAG_PICTURE_CHANGE);

							if (ds.current_y == m_size.cy)
								{
								return_value = LOAD_STATE_COMPLETE;
								goto exit;
								}
	               			}
	            		}
					}
				}
		default:
			return_value = LOAD_STATE_ABORTED;
			goto exit;
		}
exit:
	return return_value;
	}

LOAD_STATE CGifPicture::OnEndOfFile()
	{
	DEBUG_LOCK();
	LOAD_STATE l = GetLoadState();
	Unlock();

	if (l == LOAD_STATE_COMPLETE && m_is_interlaced)
		{
		// If we are complete and are interlaced, do  the final dither of everything
		m_decode_struct->m_dither->Dither( m_bitmap, m_remapped_bitmap, m_size.cy );
		Notify(CHANGEFLAG_PICTURE_CHANGE);
		}

	// Delete original bitmap since we don't need it anymore
	if (m_bitmap)
		{
		delete m_bitmap;
		m_bitmap = NULL;
		}
		
	if (m_decode_struct)
		{
		if (m_decode_struct->line_buf)
			delete m_decode_struct->line_buf;
		if (m_decode_struct->m_dither)
			delete m_decode_struct->m_dither;

		delete m_decode_struct;
		m_decode_struct = NULL;
		}

	switch(l)
		{
		case LOAD_STATE_LOADING:
		case LOAD_STATE_NOT_LOADED:
			return LOAD_STATE_ABORTED;
		case LOAD_STATE_COMPLETE:
			return LOAD_STATE_COMPLETE;
		case LOAD_STATE_ABORTED:
			return LOAD_STATE_ABORTED;
		default:
			ASSERT(FALSE);
			return LOAD_STATE_ABORTED;
		}
	}

CString CGifPicture::GetTitle() const
	{
	ASSERT_LOCKED(this);

	if (IsHeaderRead() )
		{
		CString f;

		f.Format("GIF Image (%d by %d)", (int)GetSize().cx, (int)GetSize().cy);
		return f;
		}
	else
		{
		return "";
		}
	}

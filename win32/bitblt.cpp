#include "cross_p.h"
#include "resource.h"

#ifndef _FMTHTML_H_
#include "fmthtml.h"
#endif

#ifndef _PICTURE_H_
#include "picture.h"
#endif

#ifndef _BITBLT_H_
#include "bitblt.h"
#endif

extern BYTE const aDividedBy51RoundedTimes51[256];

// the cheap alternative to dither

void TranslateIndexs( const RGBQUAD *original_palette, LPCBYTE input, LPBYTE output, INT amount, INT transparent_color)
	{
	INT count = amount;

	while(count > 0)
		{
		BYTE i = *input;

		if (i == transparent_color)
			*output = TRANS_COLOR;
		else
			*output = aTranslateToPaletteEntry[
				aDividedBy51Rounded[ original_palette[i].rgbRed ] +
				aDividedBy51RoundedTimes6[ original_palette[i].rgbGreen ] +
				aDividedBy51RoundedTimes36[ original_palette[i].rgbBlue] ];
		
		input++;
		output++;
		count--;
		}
	}

CFsDither::CFsDither( RGBQUAD original_palette[256], INT xsize, INT bytes_per_row, INT transparent_color)
	{
	memcpy(m_picture_palette, original_palette, sizeof(RGBQUAD) * 256 );
	m_row_index = 0;
	m_row_width = xsize;
	m_bytes_per_row = bytes_per_row;
	m_transparent_color = transparent_color;

	INT32 array_size = (xsize + 2) * 3;

	m_fserrors = new INT[ array_size];
	memset(m_fserrors, 0, array_size * sizeof(INT) );

	m_error_limiter = new INT[ 512 ];
	m_on_odd_row = FALSE;

	
#define STEPSIZE 16

	INT out,in;

  	/* Map errors 1:1 up to +- MAXJSAMPLE/16 */
  	out = 0;
  	for (in = 0; in < STEPSIZE; in++, out++) 
  		{
    	m_error_limiter[256+in] = out; m_error_limiter[256-in] = -out;
  		}

  	/* Map errors 1:2 up to +- 3*MAXJSAMPLE/16 */
  	for (; in < STEPSIZE*3; in++, out += (in&1) ? 0 : 1) 
  		{
    	m_error_limiter[256+in] = out; m_error_limiter[256-in] = -out;
  		}

  /* Clamp the rest to final out value (which is (MAXJSAMPLE+1)/8) */
  	for (; in <= 255; in++) 
  		{
    	m_error_limiter[256+in] = out; m_error_limiter[256-in] = -out;
		}
	}

CFsDither::~CFsDither()
	{
	delete [] m_fserrors;
	delete [] m_error_limiter;
	}

void CFsDither::Dither(LPCBYTE input_buf, LPBYTE output_buf, int num_rows)
	{
	INT cur0, cur1, cur2;	/* current error or pixel value */
	INT belowerr0, belowerr1, belowerr2; /* error for pixel below cur */
	INT bpreverr0, bpreverr1, bpreverr2; /* error for below/prev col */
	LPINT errorptr;	/* => fserrors[] at column before current */
	LPCBYTE inptr;		/* => current input pixel */
	LPBYTE outptr;		/* => current output pixel */
	INT dir;			/* +1 or -1 depending on direction */
	INT dir3;			/* 3*dir, for advancing errorptr */
	INT row;
	INT col;
	INT width = m_row_width;
	LPINT error_limit = m_error_limiter+256;
	
	INT transparent_color = m_transparent_color;

	for (row = 0; row < num_rows; row++) 
		{
		inptr = input_buf + row*m_bytes_per_row;
		outptr = output_buf + row*m_bytes_per_row;

		if (m_on_odd_row) 
			{
			/* work right to left in this row */
			inptr += (width-1);	/* so point to rightmost pixel */
			outptr += width-1;
			dir = -1;
			dir3 = -3;
			errorptr = m_fserrors + (width+1)*3; /* => entry after last column */
			m_on_odd_row = FALSE; /* flip for next time */
			} 
		else 
			{
			/* work left to right in this row */
			dir = 1;
			dir3 = 3;
			errorptr = m_fserrors; /* => entry before first real column */
			m_on_odd_row = TRUE; /* flip for next time */
			}
		/* Preset error values: no error propagated to first pixel from left */
		cur0 = cur1 = cur2 = 0;
		/* and no error propagated to row below yet */
		belowerr0 = belowerr1 = belowerr2 = 0;
		bpreverr0 = bpreverr1 = bpreverr2 = 0;

		for (col = width; col > 0; col--) 
			{
			/* curN holds the error propagated from the previous pixel on the
			* current line.  Add the error propagated from the previous line
			* to form the complete error correction term for this pixel, and
			* round the error term (which is expressed * 16) to an integer.
			* RIGHT_SHIFT rounds towards minus infinity, so adding 8 is correct
			* for either sign of the error value.
			* Note: errorptr points to *previous* column's array entry.
			*/
			cur0 = (cur0 + errorptr[dir3+0] + 8) >> 4;
			cur1 = (cur1 + errorptr[dir3+1] + 8) >> 4;
			cur2 = (cur2 + errorptr[dir3+2] + 8) >> 4;
			/* Limit the error using transfer function set by init_error_limit.
			* See comments with init_error_limit for rationale.
			*/
			cur0 = error_limit[cur0];
			cur1 = error_limit[cur1];
			cur2 = error_limit[cur2];

			/* Form pixel value + error, and range-limit to 0..MAXJSAMPLE.
			* The maximum error is +- MAXJSAMPLE (or less with error limiting);
			* this sets the required size of the range_limit array.
			*/

			INT input = *inptr;

			if (input == transparent_color)
				{
				// transparent color does not propagate any error
				*outptr = TRANS_COLOR;
				cur0 = 0;
				cur1 = 0;
				cur2 = 0;
				}
			else
				{
				cur0 += m_picture_palette[ input ].rgbRed;
				cur1 += m_picture_palette[ input ].rgbGreen;
				cur2 += m_picture_palette[ input ].rgbBlue;

				if (cur0 < 0)
					cur0 = 0;
				else if (cur0 > 255)
					cur0 = 255;

				if (cur1 < 0)
					cur1 = 0;
				else if (cur1 > 255)
					cur1 = 255;

				if (cur2 < 0)
					cur2 = 0;
				else if (cur2 > 255)
					cur2 = 255;

				*outptr = aTranslateToPaletteEntry[
					aDividedBy51Rounded[ cur0 ] +
					aDividedBy51RoundedTimes6[ cur1 ] +
					aDividedBy51RoundedTimes36[ cur2 ] ];

				/* Compute representation error for this pixel */
				cur0 -= aDividedBy51RoundedTimes51[ cur0 ];
				cur1 -= aDividedBy51RoundedTimes51[ cur1 ];
				cur2 -= aDividedBy51RoundedTimes51[ cur2 ];
				}
			
			/* Compute error fractions to be propagated to adjacent pixels.
			* Add these into the running sums, and simultaneously shift the
			* next-line error sums left by 1 column.
			*/
		
			INT bnexterr, delta;

			bnexterr = cur0;	/* Process component 0 */
			delta = cur0 * 2;
			cur0 += delta;		/* form error * 3 */
			errorptr[0] = (INT) (bpreverr0 + cur0);
			cur0 += delta;		/* form error * 5 */
			bpreverr0 = belowerr0 + cur0;
			belowerr0 = bnexterr;
			cur0 += delta;		/* form error * 7 */
			bnexterr = cur1;	/* Process component 1 */
			delta = cur1 * 2;
			cur1 += delta;		/* form error * 3 */
			errorptr[1] = (INT) (bpreverr1 + cur1);
			cur1 += delta;		/* form error * 5 */
			bpreverr1 = belowerr1 + cur1;
			belowerr1 = bnexterr;
			cur1 += delta;		/* form error * 7 */
			bnexterr = cur2;	/* Process component 2 */
			delta = cur2 * 2;
			cur2 += delta;		/* form error * 3 */
			errorptr[2] = (INT) (bpreverr2 + cur2);
			cur2 += delta;		/* form error * 5 */
			bpreverr2 = belowerr2 + cur2;
			belowerr2 = bnexterr;
			cur2 += delta;		/* form error * 7 */

			/* At this point curN contains the 7/16 error value to be propagated
			* to the next pixel on the current line, and all the errors for the
			* next line have been shifted over.  We are therefore ready to move on.
			*/
			inptr += dir;		/* Advance pixel pointers to next column */
			outptr += dir;
			errorptr += dir3;		/* advance errorptr to current column */
			}
		/* Post-loop cleanup: we must unload the final error values into the
		* final fserrors[] entry.  Note we need not unload belowerrN because
		* it is for the dummy column before or after the actual array.
		*/
		errorptr[0] = (INT) bpreverr0; /* unload prev errs into array */
		errorptr[1] = (INT) bpreverr1;
		errorptr[2] = (INT) bpreverr2;
		}
	AfxCheckMemory();
	}


void DrawPicture(CPicture *picture, REDRAW_PARAMS& params, const CRect& extent )
	{
	if (!picture)
		{
		HICON hicon = LoadIcon( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDI_BAD_GIF ) );
		params.m_pDC->DrawIcon(extent.left, extent.top, hicon);
		return;
		}

	picture->DEBUG_LOCK();
	BOOL hdr_read = picture->IsHeaderRead();
	LOAD_STATE load_state = picture->GetLoadState();
	picture->Unlock();

	if (!hdr_read)
		{
		if ( load_state==LOAD_STATE_LOADING || 
			 load_state == LOAD_STATE_NOT_LOADED)
			{
			// draw nothing
			return;
			}

		// we encountered an LOAD_STATE_ABORTED gif where we don't have any image data, so draw our little "BAD" icon
		// just draw the question mark icon in the place of the bitmap

		HICON hicon = LoadIcon( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDI_BAD_GIF ) );
		params.m_pDC->DrawIcon(extent.left, extent.top, hicon);

		return;
		}	

	PICTURE_FORMAT_INFO format_info;
	LPBYTE source;
	CSize size;
	BOOL is_transparent;
	
	picture->DEBUG_LOCK();
	source = picture->LockBitmapBits();
	size = picture->GetSize();
	picture->GetFormatInfo(format_info );
	is_transparent = picture->IsTransparent();
	picture->Unlock();

	if (source == NULL)
		{
		// Wait until we have a picture before proceeding
		return;
		}


	CRect clip;
	CRect clipbox;
	params.m_pDC->GetClipBox(&clipbox);

	clip.IntersectRect( clipbox, extent );

	int actual_width = format_info.m_bytes_per_line;

	int tx = extent.left;
	int ty = extent.top;

	int xwidth = size.cx;
	int ywidth = size.cy;

	int dif; 
	if (tx >= clip.right || ty >= clip.bottom || tx + xwidth <= clip.left || ty + ywidth <= clip.top) 
		{
		picture->UnlockBitmapBits(source);
		return;
		}

	dif = clip.top - ty; 
	if (dif > 0) 
		{ 
		ty += dif; 
		ywidth -= dif; 
		source += dif * actual_width; 
		} 
	dif = clip.left - tx; 
	if (dif > 0) 
		{ 
		tx += dif; 
		xwidth -= dif; 
		source += dif; 
		} 
	dif = tx + xwidth - clip.right; 
	if (dif > 0) 
		{ 
		xwidth -= dif; 
		} 
	dif = ty + ywidth - clip.bottom; 
	if (dif > 0) 
		{ 
		ywidth -= dif; 
		}

	LPBYTE target =  params.m_frame_buffer +
			(ty + params.m_frame_buffer_ofs.y) * params.m_frame_buffer_size.cx + 
			(tx + params.m_frame_buffer_ofs.x);
	
	if (is_transparent)
		{
		// use the slower bitblt

		TransCopyDIBBits(target,
			source, xwidth, ywidth, params.m_frame_buffer_size.cx, actual_width);
		}
	else
		{
		// copy without transparent color
		CopyDIBBits( target,
			source, xwidth, ywidth, params.m_frame_buffer_size.cx, actual_width);
		}
	picture->UnlockBitmapBits(source);
	}


void TransCopyDIBBits( void * pDest,
  void const *pSource, DWORD dwWidth, DWORD dwHeight, DWORD dwScanD,
  long dwScanS)
  {
  _asm  	{
        push esi
        push edi

        mov ecx, dwWidth
        test ecx,ecx
        jz tcdb_nomore     // test for silly case

        mov edx, dwHeight       // EDX is line counter
        mov ah, TRANS_COLOR        // AH has transparency color

        mov esi, pSource         //; [ESI] point to source

        mov edi, pDest

        sub dwScanD,ecx         // bias these
        sub dwScanS,ecx

        mov ebx,ecx             // save this for later

        align 4

tcdb_morelines:
        mov ecx, ebx            //ECX is pixel counter
        shr ecx,2
        jz  short tcdb_nextscan

//
// The idea here is to not branch very often so we unroll the loop by four
// and try to not branch when a whole run of pixels is either transparent
// or not transparent.
//
// There are two loops. One loop is for a run of pixels equal to the
// transparent color, the other is for runs of pixels we need to store.
//
// When we detect a "bad" pixel we jump to the same position in the
// other loop.
//
// Here is the loop we will stay in as long as we encounter a "transparent"
// pixel in the source.
//

        align 4

tcdb_same:
        mov al, [esi]
        cmp al, ah
        jne short tcdb_diff0

tcdb_same0:
        mov al, [esi+1]
        cmp al, ah
        jne short tcdb_diff1

tcdb_same1:
        mov al, [esi+2]
        cmp al, ah
        jne short tcdb_diff2

tcdb_same2:
        mov al, [esi+3]
        cmp al, ah
        jne short tcdb_diff3

tcdb_same3:
        add edi,4
        add esi,4
        dec ecx
        jnz short tcdb_same
        jz  short tcdb_nextscan

//
// Here is the loop we will stay in as long as 
// we encounter a "non transparent" pixel in the source.
//

        align 4

tcdb_diff:
        mov al, [esi]
        cmp al, ah
        je short tcdb_same0

tcdb_diff0:
        mov [edi],al
        mov al, [esi+1]
        cmp al, ah
        je short tcdb_same1

tcdb_diff1:
        mov [edi+1],al
        mov al, [esi+2]
        cmp al, ah
        je short tcdb_same2

tcdb_diff2:
        mov [edi+2],al
        mov al, [esi+3]
        cmp al, ah
        je short tcdb_same3

tcdb_diff3:
        mov [edi+3],al

        add edi,4
        add esi,4
        dec ecx
        jnz short tcdb_diff
        jz  short tcdb_nextscan

//
// We are at the end of a scan, check for odd leftover pixels to do
// and go to the next scan.
//

        align 4

tcdb_nextscan:
        mov ecx,ebx
        and ecx,11b
        jnz short tcdb_oddstuff
        // move on to the start of the next line

tcdb_nextscan1:
        add esi, dwScanS
        add edi, dwScanD

        dec edx                 // line counter
        jnz short tcdb_morelines
        jz  short tcdb_nomore

//
// If the width is not a multiple of 4 we will come here to clean up
// the last few pixels
//

tcdb_oddstuff:
        inc ecx
tcdb_oddloop:
        dec ecx
        jz  short tcdb_nextscan1
        mov al, [esi]
        inc esi
        inc edi
        cmp al, ah
        je  short tcdb_oddloop
        mov [edi-1],al
        jmp short tcdb_oddloop

tcdb_nomore:
        pop edi
        pop esi
		}
	}

void CopyDIBBits( void * pDest,
  void const *pSource, DWORD dwWidth, DWORD dwHeight, DWORD dwScanD,
  DWORD dwScanS )
  {
  _asm  {
        push esi
        push edi

        mov ecx, dwWidth
        test ecx,ecx
        jz cdb_nomore     // test for silly case

        mov edx, dwHeight       // EDX is line counter
        test edx,edx
        jz cdb_nomore     // test for silly case

        mov esi, pSource         // [ESI] point to source

        mov edi, pDest

        sub dwScanD,ecx         // bias these
        sub dwScanS,ecx

        mov ebx,ecx
        shr ebx,2

        mov eax,ecx
        and eax,11b

        align 4

cdb_loop:
        mov ecx, ebx
        rep movs dword ptr [edi], dword ptr [esi]
        mov ecx,eax
        rep movs byte ptr [edi], byte ptr [esi]

        add esi, dwScanS
        add edi, dwScanD
        dec edx                 // line counter
        jnz short cdb_loop

cdb_nomore:
        pop edi
        pop esi
		}
	}

static CPalette halftone_palette;

static struct {
	WORD         palVersion;
    WORD         palNumEntries;
    PALETTEENTRY palPalEntry[256];
	} log_halftone_palette = 
	{
	0x300,
	256,

	{
	{0,0,0,0}, 		{128,0,0,0}, 		{0,128,0,0}, 	{128,128,0,0}, 
	{0,0,128,0},  	{128,0,128,0}, 		{0,128,128,0},	{192,192,192,0},
	{192,220,192,0},{166,202,240,0},	{4,4,4,4},		{8,8,8,4},
	{12,12,12,4},	{17,17,17,4},		{22,22,22,4},	{28,28,28,4},
	{34,34,34,4},	{41,41,41,4},		{85,85,85,4},	{77,77,77,4},
	{66,66,66,4},	{57,57,57,4},		{129,129,129,4},{129,0,0,4},
	{0,129,0,4},	{129,129,0,4},		{0,0,129,4},	{129,0,129,4},
	{0,129,129,4},	{51,0,0,4},			{102,0,0,4},	{153,0,0,4},
	{204,0,0,4},	{0,51,0,4},			{51,51,0,4},	{102,51,0,4},
	{153,51,0,4},	{204,51,0,4},		{255,51,0,4},	{0,102,0,4},
	{51,102,0,4},	{102,102,0,4},		{153,102,0,4},	{204,102,0,4},
	{255,102,0,4},	{0,153,0,4},		{51,153,0,4},	{102,153,0,4},
	{153,153,0,4},	{204,153,0,4},		{255,153,0,4},	{0,204,0,4},
	{51,204,0,4},	{102,204,0,4},		{153,204,0,4},	{204,204,0,4},
	{255,204,0,4},	{102,255,0,4},		{153,255,0,4},	{204,255,0,4},
	{0,0,51,4},		{51,0,51,4},		{102,0,51,4},	{153,0,51,4},
	{204,0,51,4},	{255,0,51,4},		{0,51,51,4},	{51,51,51,4},
	{102,51,51,4},	{153,51,51,4},		{204,51,51,4},	{255,51,51,4},
	{0,102,51,4},	{51,102,51,4},		{102,102,51,4},	{153,102,51,4},
	{204,102,51,4},	{255,102,51,4},		{0,153,51,4},	{51,153,51,4},
	{102,153,51,4},	{153,153,51,4},		{204,153,51,4},	{255,153,51,4},
	{0,204,51,4},	{51,204,51,4},		{102,204,51,4},	{153,204,51,4},
	{204,204,51,4},	{255,204,51,4},		{51,255,51,4},	{102,255,51,4},
	{153,255,51,4},	{204,255,51,4},		{255,255,51,4},	{0,0,102,4},
	{51,0,102,4},	{102,0,102,4},		{153,0,102,4},	{204,0,102,4},
	{255,0,102,4},	{0,51,102,4},		{51,51,102,4},	{102,51,102,4},
	{153,51,102,4},	{204,51,102,4},		{255,51,102,4},	{0,102,102,4},
	{51,102,102,4},	{102,102,102,4},	{153,102,102,4},{204,102,102,4},
	{0,153,102,4},	{51,153,102,4},		{102,153,102,4},{153,153,102,4},
	{204,153,102,4},{255,153,102,4},	{0,204,102,4},	{51,204,102,4},
	{153,204,102,4},{204,204,102,4},	{255,204,102,4},{0,255,102,4},
	{51,255,102,4},	{153,255,102,4},	{204,255,102,4},{255,0,204,4},
	{204,0,255,4},	{0,153,153,4},		{153,51,153,4},	{153,0,153,4},
	{204,0,153,4},	{0,0,153,4},		{51,51,153,4},	{102,0,153,4},
	{204,51,153,4},	{255,0,153,4},		{0,102,153,4},	{51,102,153,4},
	{102,51,153,4},	{153,102,153,4},	{204,102,153,4},{255,51,153,4},
	{51,153,153,4},	{102,153,153,4},	{153,153,153,4},{204,153,153,4},
	{255,153,153,4},{0,204,153,4},		{51,204,153,4},	{102,204,102,4},
	{153,204,153,4},{204,204,153,4},	{255,204,153,4},{0,255,153,4},
	{51,255,153,4},	{102,204,153,4},	{153,255,153,4},{204,255,153,4},
	{255,255,153,4},{0,0,204,4},		{51,0,153,4},	{102,0,204,4},
	{153,0,204,4},	{204,0,204,4},		{0,51,153,4},	{51,51,204,4},
	{102,51,204,4},	{153,51,204,4},		{204,51,204,4},	{255,51,204,4},
	{0,102,204,4},	{51,102,204,4},		{102,102,153,4},{153,102,204,4},
	{204,102,204,4},{255,102,153,4},	{0,153,204,4},	{51,153,204,4},
	{102,153,204,4},{153,153,204,4},	{204,153,204,4},{255,153,204,4},
	{0,204,204,4},	{51,204,204,4},		{102,204,204,4},{153,204,204,4},
	{204,204,204,4},{255,204,204,4},	{0,255,204,4},	{51,255,204,4},
	{102,255,153,4},{153,255,204,4},	{204,255,204,4},{255,255,204,4},
	{51,0,204,4},	{102,0,255,4},		{153,0,255,4},	{0,51,204,4},
	{51,51,255,4},	{102,51,255,4},		{153,51,255,4},	{204,51,255,4},
	{255,51,255,4},	{0,102,255,4},		{51,102,255,4},	{102,102,204,4},
	{153,102,255,4},{204,102,255,4},	{255,102,204,4},{0,153,255,4},
	{51,153,255,4},	{102,153,255,4},	{153,153,255,4},{204,153,255,4},
	{255,153,255,4},{0,204,255,4},		{51,204,255,4},	{102,204,255,4},
	{153,204,255,4},{204,204,255,4},	{255,204,255,4},{51,255,255,4},
	{102,255,204,4},{153,255,255,4},	{204,255,255,4},{255,102,102,4},
	{102,255,102,4},{255,255,102,4},	{102,102,255,4},{255,102,255,4},
	{102,255,255,4},{193,193,193,4},	{95,95,95,4},	{119,119,119,4},
	{134,134,134,4},{150,150,150,4},	{203,203,203,4},{178,178,178,4},
	{215,215,215,4},{221,221,221,4},	{227,227,227,4},{234,234,234,4},
	{241,241,241,4},{248,248,248,4},	{255,251,240,0},{160,160,164,0},
	{128,128,128,0},{255,0,0,0},		{0,255,0,0},	{255,255,0,0},
	{0,0,255,0},	{255,0,255,0},		{0,255,255,0},	{255,255,255,0}}};


CPalette *GetHalftonePalette()
	{
	if (!halftone_palette.m_hObject)
		halftone_palette.CreatePalette( (LOGPALETTE *)&log_halftone_palette );

	return &halftone_palette;
	}

void ClearSystemPalette(void)
{
  //*** A dummy palette setup
  struct
  {
    WORD Version;
    WORD NumberOfEntries;
    PALETTEENTRY aEntries[256];
  } Palette =
  {
    0x300,
    256
  };

  HPALETTE ScreenPalette = 0;
  HDC ScreenDC;
  int Counter;
  UINT nMapped = 0;
  BOOL bOK = FALSE;
  int  nOK = 0;
  
  //*** Reset everything in the system palette to black
  for(Counter = 0; Counter < 256; Counter++)
  {
    Palette.aEntries[Counter].peRed = 0;
    Palette.aEntries[Counter].peGreen = 0;
    Palette.aEntries[Counter].peBlue = 0;
    Palette.aEntries[Counter].peFlags = PC_NOCOLLAPSE;
  }

  //*** Create, select, realize, deselect, and delete the palette
  ScreenDC = GetDC(NULL);
  ScreenPalette = CreatePalette((LOGPALETTE *)&Palette);

  if (ScreenPalette)
  {
    ScreenPalette = SelectPalette(ScreenDC,ScreenPalette,FALSE);
    nMapped = RealizePalette(ScreenDC);
    ScreenPalette = SelectPalette(ScreenDC,ScreenPalette,FALSE);
    bOK = DeleteObject(ScreenPalette);
  }

  nOK = ReleaseDC(NULL, ScreenDC);

  return;
}


BYTE const aDividedBy51Rounded[256] =
{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
};

BYTE const aDividedBy51RoundedTimes51[256] =
{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
  51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
  51, 51, 51, 51, 51, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
  102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
  102, 102, 102, 102, 102, 102, 102, 102, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153,
  153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153,
  153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
  204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
  204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};

BYTE const aDividedBy51RoundedTimes6[256] =
	{
  	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  	0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  	6, 6, 6, 6, 6, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
  	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
  	12, 12, 12, 12, 12, 12, 12, 12, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
  	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
  	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
  	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
  	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
  	30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30
	};

BYTE const aDividedBy51RoundedTimes36[256] =
	{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
  36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
  36, 36, 36, 36, 36, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72,
  72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72,
  72, 72, 72, 72, 72, 72, 72, 72, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108,
  108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108,
  108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144,
  144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144,
  144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180,
  180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180
	};


INT8 const aDividedBy51RoundedError[256] =
	{
   0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 
  24, 25,-25,-24,-23,-22,-21,-20,-19,-18,-17,-16,-15,-14,-13,-12,-11,-10, -9, -8, -7, -6, -5, -4, 
  -3, -2, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 
  21, 22, 23, 24, 25,-25,-24,-23,-22,-21,-20,-19,-18,-17,-16,-15,-14,-13,-12,-11,-10, -9, -8, -7, 
  -6, -5, -4, -3, -2, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17,
  18, 19, 20, 21, 22, 23, 24, 25,-25,-24,-23,-22,-21,-20,-19,-18,-17,-16,-15,-14,-13,-12,-11,-10, 
  -9, -8, -7, -6, -5, -4, -3, -2, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,-25,-24,-23,-22,-21,-20,-19,-18,-17,-16,-15,-14,-13,
 -12,-11,-10, -9, -8, -7, -6, -5, -4, -3, -2, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 
  12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,-25,-24,-23,-22,-21,-20,-19,-18,-17,-16,
 -15,-14,-13,-12,-11,-10, -9, -8, -7, -6, -5, -4, -3, -2, -1,  0
	};
  
/*  
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 
  22, 23, 24, 25, 26,-26,-25,-24,-23,-22,-21,-20,-19,-18,-17,-16,-15,-14,-13,-12,-11,-10, -9, -8, -7, 
  -6, -5, -4, -3, -2, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 
  19, 20, 21, 22, 23, 24, 25, 26,-26,-25,-24,-23,-22,-21,-20,-19,-18,-17,-16,-15,-14,-13,-12,-11,-10, 
  -9, -8, -7, -6, -5, -4, -3, -2, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,-26,-25,-24,-23,-22,-21,-20,-19,-18,-17,-16,-15,-14,-13,
  -12,-11,-10, -9, -8, -7,-6, -5, -4, -3, -2, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 
  13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,-26,-25,-24,-23,-22,-21,-20,-19,-18,-17,-16,
  -15,-14,-13,-12,-11,-10, -9, -8, -7,-6, -5, -4, -3, -2, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 
  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,-26,-25,-24,-23,-22,-21,-20,-19,
  -18,-17,-16,-15,-14,-13,-12,-11,-10, -9, -8, -7,-6, -5, -4, -3, -2, -1,  0
  };
*/

BYTE const aDividedBy51[256] =
{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5 
};

BYTE const aModulo51[256] =
{
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
  20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
  38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 0, 1, 2, 3, 4, 5, 6,
  7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
  44, 45, 46, 47, 48, 49, 50, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
  13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
  31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
  49, 50, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
  18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
  36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 0, 1, 2, 3,
  4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
  23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 0
};


/*----------------------------------------------------------------------------

Multiplication LUTs.  These compute 0-5 times 6 and 36.

*/

BYTE const aTimes6[6] =
{
  0, 6, 12, 18, 24, 30
};

BYTE const aTimes36[6] =
{
  0, 36, 72, 108, 144, 180
};


/*----------------------------------------------------------------------------

Dither matrices for 8 bit to 2.6 bit halftones.

*/

BYTE const aHalftone16x16[256] =
{
  0, 44, 9, 41, 3, 46, 12, 43, 1, 44, 10, 41, 3, 46, 12, 43,
  34, 16, 25, 19, 37, 18, 28, 21, 35, 16, 26, 19, 37, 18, 28, 21,
  38, 6, 47, 3, 40, 9, 50, 6, 38, 7, 47, 4, 40, 9, 49, 6,
  22, 28, 13, 31, 25, 31, 15, 34, 22, 29, 13, 32, 24, 31, 15, 34,
  2, 46, 12, 43, 1, 45, 10, 42, 2, 45, 11, 42, 1, 45, 11, 42,
  37, 18, 27, 21, 35, 17, 26, 20, 36, 17, 27, 20, 36, 17, 26, 20,
  40, 8, 49, 5, 38, 7, 48, 4, 39, 8, 48, 5, 39, 7, 48, 4,
  24, 30, 15, 33, 23, 29, 13, 32, 23, 30, 14, 33, 23, 29, 14, 32,
  2, 46, 12, 43, 0, 44, 10, 41, 3, 47, 12, 44, 0, 44, 10, 41,
  37, 18, 27, 21, 35, 16, 25, 19, 37, 19, 28, 22, 35, 16, 25, 19,
  40, 9, 49, 5, 38, 7, 47, 4, 40, 9, 50, 6, 38, 6, 47, 3,
  24, 30, 15, 34, 22, 29, 13, 32, 25, 31, 15, 34, 22, 28, 13, 31,
  1, 45, 11, 42, 2, 46, 11, 42, 1, 45, 10, 41, 2, 46, 11, 43,
  36, 17, 26, 20, 36, 17, 27, 21, 35, 16, 26, 20, 36, 18, 27, 21,
  39, 8, 48, 4, 39, 8, 49, 5, 38, 7, 48, 4, 39, 8, 49, 5,
  23, 29, 14, 33, 24, 30, 14, 33, 23, 29, 13, 32, 24, 30, 14, 33
};

BYTE const aHalftone8x8[64] =
{
   0, 38,  9, 47,  2, 40, 11, 50,
  25, 12, 35, 22, 27, 15, 37, 24,
   6, 44,  3, 41,  8, 47,  5, 43,
  31, 19, 28, 15, 34, 21, 31, 18,
   1, 39, 11, 49,  0, 39, 10, 48,
  27, 14, 36, 23, 26, 13, 35, 23,
   7, 46,  4, 43,  7, 45,  3, 42,
  33, 20, 30, 17, 32, 19, 29, 16
};

BYTE const aHalftone4x4_1[16] =
{
  0, 25, 6, 31,
  38, 12, 44, 19,
  9, 35, 3, 28,
  47, 22, 41, 15
};

BYTE const aHalftone4x4_2[16] =
{
  41, 3, 9, 28,
  35, 15, 22, 47,
  6, 25, 38, 0,
  19, 44, 31, 12
};

BYTE const aHalftone2x2[4] =
{
  0, 50,
  50,25
};


/***************************************************************************
  aWinGHalftoneTranslation

  Translates a 2.6 bit-per-pixel halftoned representation into the
  slightly rearranged WinG Halftone Palette.
*/

BYTE const aTranslateToPaletteEntry[216] =
{
  0,
  29,
  30,
  31,
  32,
  249,
  33,
  34,
  35,
  36,
  37,
  38,
  39,
  40,
  41,
  42,
  43,
  44,
  45,
  46,
  47,
  48,
  49,
  50,
  51,
  52,
  53,
  54,
  55,
  56,
  250,
  250,
  57,
  58,
  59,
  251,
  60,
  61,
  62,
  63,
  64,
  65,
  66,
  67,
  68,
  69,
  70,
  71,
  72,
  73,
  74,
  75,
  76,
  77,
  78,
  79,
  80,
  81,
  82,
  83,
  84,
  85,
  86,
  87,
  88,
  89,
  250,
  90,
  91,
  92,
  93,
  94,
  95,
  96,
  97,
  98,
  99,
  100,
  101,
  102,
  103,
  104,
  105,
  106,
  107,
  108,
  109,
  110,
  111,
  227,
  112,
  113,
  114,
  115,
  116,
  117,
  118,
  119,
  151,
  120,
  121,
  122,
  123,
  124,
  228,
  125,
  126,
  229,
  133,
  162,
  135,
  131,
  132,
  137,
  166,
  134,
  140,
  130,
  136,
  143,
  138,
  139,
  174,
  141,
  142,
  177,
  129,
  144,
  145,
  146,
  147,
  148,
  149,
  150,
  157,
  152,
  153,
  154,
  155,
  156,
  192,
  158,
  159,
  160,
  161,
  196,
  163,
  164,
  165,
  127,
  199,
  167,
  168,
  169,
  170,
  171,
  172,
  173,
  207,
  175,
  176,
  210,
  178,
  179,
  180,
  181,
  182,
  183,
  184,
  185,
  186,
  187,
  188,
  189,
  190,
  191,
  224,
  193,
  194,
  195,
  252,
  252,
  197,
  198,
  128,
  253,
  252,
  200,
  201,
  202,
  203,
  204,
  205,
  206,
  230,
  208,
  209,
  231,
  211,
  212,
  213,
  214,
  215,
  216,
  217,
  218,
  219,
  220,
  221,
  222,
  254,
  223,
  232,
  225,
  226,
  255,
};


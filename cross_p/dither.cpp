#include "cross_p.h"

#include "bitblt.h"



CFsDither::CFsDither( RGBQUAD original_palette[256], INT xsize, INT bytes_per_row, INT transparent_color)
	{
	memcpy(m_picture_palette, original_palette, sizeof(original_palette) );
	m_row_index = 0;
	m_row_width = xsize;
	m_bytes_per_row = bytes_per_row;
	m_transparent_color = transparent_color;
	m_fserrors = new INT[ m_row_width * 3];
	m_error_limiter = new INT[ m_row_width ];

	// error is between -128, and 127

	m_error_limiter += 128;
	
#define STEPSIZE 16

  	/* Map errors 1:1 up to +- MAXJSAMPLE/16 */
  	out = 0;
  	for (in = 0; in < STEPSIZE; in++, out++) 
  		{
    	m_error_limiter[in] = out; m_error_limiter[-in] = -out;
  		}

  	/* Map errors 1:2 up to +- 3*MAXJSAMPLE/16 */
  	for (; in < STEPSIZE*3; in++, out += (in&1) ? 0 : 1) 
  		{
    	m_error_limiter[in] = out; m_error_limiter[-in] = -out;
  		}

  /* Clamp the rest to final out value (which is (MAXJSAMPLE+1)/8) */
  	for (; in <= MAXJSAMPLE; in++) 
  		{
    	m_error_limiter[in] = out; m_error_limiter[-in] = -out;
		}
	}

CFsDither::~CFsDither()
	{
	delete [] m_fserrors;
	delete [] (m_error_limiter - 128);
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
	LPINT error_limit = m_error_limiter;
	
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
				else if (cur1 > 255)
					cur2 = 255;

				*outptr = aTranslateToPaletteEntry[
					aDividedBy51Rounded[ cur0 ] |
					aDividedBy51RoundedTimes6[ cur1 ] |
					aDividedBy51RoundedTimes36[ cur2 ] ];
				 
				/* Compute representation error for this pixel */
				cur0 -= aDividedBy51RoundedError[ cur0 ];
				cur1 -= aDividedBy51RoundedError[ cur1 ];
				cur2 -= aDividedBy51RoundedError[ cur2 ];
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
	}

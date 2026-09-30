#include "cross_p.h"

#include "readjpeg.h"

#ifndef _BITBLT_H_
#include "bitblt.h"
#endif

#include "jpeglib.h"
#include "jerror.h"

typedef enum
	{
	reading_header,
	reading_scanlines,
	reading_done
	} JPEG_STATE;

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */
};

class DECODE_STRUCT
	{
public:
	CBigString m_file_buffer;		
		// used to store the JPEG data as it is read, discarded after the operation is complete
    struct jpeg_decompress_struct m_cinfo;
	struct my_error_mgr m_jerr;
  	JSAMPARRAY m_line_buffer;		
  		/* Output row buffer */
	JPEG_STATE m_jpeg_state;
	};


/*
 * Error exit handler: must not return to caller.
 *
 * Applications may override this if they want to get control back after
 * an error.  Typically one would longjmp somewhere instead of exiting.
 * The setjmp buffer can be made a private field within an expanded error
 * handler object.  Note that the info needed to generate an error message
 * is stored in the error object, so you can generate the message now or
 * later, at your convenience.
 * You should make sure that the JPEG object is cleaned up (with jpeg_abort
 * or jpeg_destroy) at some point.
 */

METHODDEF void error_exit (j_common_ptr cinfo)
	{
  	/* Always display the message */
  	(*cinfo->err->output_message) (cinfo);

  	AfxThrowUserException();
	}


/*
 * Actual output of an error or trace message.
 * Applications may override this method to send JPEG messages somewhere
 * other than stderr.
 */

METHODDEF void output_message (j_common_ptr cinfo)
	{
  	char buffer[JMSG_LENGTH_MAX];

  	/* Create the message */
  	(*cinfo->err->format_message) (cinfo, buffer);

	TRACE0( buffer );
	}


/*
 * Decide whether to emit a trace or warning message.
 * msg_level is one of:
 *   -1: recoverable corrupt-data warning, may want to abort.
 *    0: important advisory messages (always display to user).
 *    1: first level of tracing detail.
 *    2,3,...: successively more detailed tracing messages.
 * An application might override this method if it wanted to abort on warnings
 * or change the policy about which messages to display.
 */

METHODDEF void emit_message (j_common_ptr cinfo, int msg_level)
	{
  	struct jpeg_error_mgr * err = cinfo->err;

  	if (msg_level < 0) 
  		{
    	/* It's a warning message.  Since corrupt files may generate many warnings,
     	* the policy implemented here is to show only the first warning,
     	* unless trace_level >= 3.
     	*/
    	if (err->num_warnings == 0 || err->trace_level >= 3)
      		(*err->output_message) (cinfo);
    	/* Always count warnings in num_warnings. */
   		err->num_warnings++;
  		} 
  	else 
  		{
    	/* It's a trace message.  Show it if trace_level >= msg_level. */
    	if (err->trace_level >= msg_level)
      		(*err->output_message) (cinfo);
  		}
	}

/* Expanded data source object for stdio input */

#define INPUT_BUF_SIZE  1024	/* choose an efficiently fread'able size */

typedef struct {
  struct jpeg_source_mgr pub;	/* public fields */

  DECODE_STRUCT *decode;
  BOOL start_of_file;
  INT32 current_position;
  BYTE *buffer;
  INT32 buffer_size;
} my_source_mgr;

typedef my_source_mgr * my_src_ptr;

/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */

METHODDEF void	init_source (j_decompress_ptr cinfo)
	{
  	my_src_ptr src = (my_src_ptr) cinfo->src;

  	/* We reset the empty-input-file flag for each image,
   	* but we don't clear the input buffer.
   	* This is correct behavior for reading a series of images from one source.
   	*/
  	src->start_of_file = TRUE;
	src->current_position = 0;
	src->buffer_size = 0;
	src->buffer = NULL;
	}


/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * In typical applications, this should read fresh data into the buffer
 * (ignoring the current state of next_input_byte & bytes_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been reloaded.  It is not necessary to
 * fill the buffer entirely, only to obtain at least one more byte.
 *
 * There is no such thing as an EOF return.  If the end of the file has been
 * reached, the routine has a choice of ERREXIT() or inserting fake data into
 * the buffer.  In most cases, generating a warning message and inserting a
 * fake EOI marker is the best course of action --- this will allow the
 * decompressor to output however much of the image is there.  However,
 * the resulting error message is misleading if the real problem is an empty
 * input file, so we handle that case specially.
 *
 * In applications that need to be able to suspend compression due to input
 * not being available yet, a FALSE return indicates that no more data can be
 * obtained right now, but more may be forthcoming later.  In this situation,
 * the decompressor will return to its caller (with an indication of the
 * number of scanlines it has read, if any).  The application should resume
 * decompression after it has loaded more data into the input buffer.  Note
 * that there are substantial restrictions on the use of suspension --- see
 * the documentation.
 *
 * When suspending, the decompressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point must be rescanned after resumption, so move it to
 * the front of the buffer rather than discarding it.
 */

METHODDEF boolean fill_input_buffer (j_decompress_ptr cinfo)
	{
  	my_src_ptr src = (my_src_ptr) cinfo->src;
  	INT nbytes;

	nbytes = src->decode->m_file_buffer.GetLength() - src->current_position;

	if (nbytes > src->buffer_size)
		{
		if (src->buffer != NULL)
			delete [] src->buffer;
		src->buffer = DEBUG_NEW BYTE[ nbytes ];
		src->buffer_size = nbytes;
		}
		
	if (nbytes <= 0) 
		{
		src->current_position -= src->pub.bytes_in_buffer;
  		src->pub.bytes_in_buffer = 0;
		return FALSE;
		}
	else
		{
		src->decode->m_file_buffer.GetData( src->buffer, src->current_position, nbytes );
		src->current_position += nbytes;
		}

  	src->pub.next_input_byte = src->buffer;
  	src->pub.bytes_in_buffer = nbytes;
  	src->start_of_file = FALSE;

	return TRUE;
	}



METHODDEF void skip_input_data (j_decompress_ptr cinfo, long num_bytes)
	{
  	my_src_ptr src = (my_src_ptr) cinfo->src;

  	/* Just a dumb implementation for now.  Could use fseek() except
   	* it doesn't work on pipes.  Not clear that being smart is worth
   	* any trouble anyway --- large skips are infrequent.
   	*/

	if (num_bytes >= (long)src->pub.bytes_in_buffer)
		{
		src->pub.bytes_in_buffer = 0;
		src->current_position += num_bytes;
		}
	else
		{
    	src->pub.next_input_byte += (size_t) num_bytes;
    	src->pub.bytes_in_buffer -= (size_t) num_bytes;
		} 
	}


/*
 * An additional method that can be provided by data source modules is the
 * resync_to_restart method for error recovery in the presence of RST markers.
 * For the moment, this source module just uses the default resync method
 * provided by the JPEG library.  That method assumes that no backtracking
 * is possible.
 */


/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

METHODDEF void
term_source (j_decompress_ptr cinfo)
{
}


/*
 * Prepare for input from a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing decompression.
 */

void jpeg_setup_source (j_decompress_ptr cinfo, DECODE_STRUCT *ds)
	{
  	my_src_ptr src;

	if (cinfo->src == NULL) 
  		{	/* first time for this JPEG object? */
    	cinfo->src = (struct jpeg_source_mgr *)
	      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
			  sizeof(my_source_mgr));

    	src = (my_src_ptr) cinfo->src;
    	}

	src = (my_src_ptr) cinfo->src;
  	src->pub.init_source = init_source;
  	src->pub.fill_input_buffer = fill_input_buffer;
  	src->pub.skip_input_data = skip_input_data;
  	src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  	src->pub.term_source = term_source;
  	src->decode = ds;
	src->start_of_file = TRUE;
	src->current_position = 0;
	src->buffer = NULL;
	src->buffer_size = 0;
  	src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
  	src->pub.next_input_byte = NULL; /* until buffer loaded */
	}

CJpegPicture::CJpegPicture( LPCSTR url, LPCSTR mime_type, const PICTURE_FORMAT_INFO& requested_format )
	: CPicture( url, mime_type, requested_format  )
	{
	m_decode = new DECODE_STRUCT;

  	/* We set up the normal JPEG error routines, then override error_exit. */
	m_decode->m_cinfo.err = jpeg_std_error(&m_decode->m_jerr.pub);

	m_decode->m_jerr.pub.error_exit = error_exit;
  	m_decode->m_jerr.pub.emit_message = emit_message;

	m_decode->m_jpeg_state = reading_header;

	/* Now we can initialize the JPEG decompression object. */
 	jpeg_create_decompress(&m_decode->m_cinfo);

	jpeg_setup_source (&m_decode->m_cinfo, m_decode);

	m_bitmap = NULL;
  	}

CJpegPicture::~CJpegPicture()
	{
	ASSERT(m_decode == NULL);
	if (m_bitmap)
		delete [] m_bitmap;
	}

BOOL CJpegPicture::IsHeaderRead() const
	{
	ASSERT_LOCKED(this);

	if (m_decode)
		{
		if (m_decode->m_jpeg_state > reading_header)
			return TRUE;
		else
			return FALSE;
		}
	else
		{
		return m_bitmap!=NULL;
		}
	}

CSize CJpegPicture::GetSize() const
	{
	ASSERT_LOCKED(this);

	return m_size;
	}

void CJpegPicture::GetPalette( UINT32 palette[256] ) const
	{
	ASSERT_LOCKED(this);
	}

void CJpegPicture::GetFormatInfo(PICTURE_FORMAT_INFO& format_info) const
	{
	ASSERT_LOCKED(this);

	format_info.m_bits_per_pixel = 8;
	format_info.m_use_standard_palette = TRUE;
	format_info.m_bytes_per_line = RoundUp4(m_size.cx);
	}

BOOL CJpegPicture::IsTransparent() const
	{
	ASSERT_LOCKED(this);

	if (GetLoadState() == LOAD_STATE_LOADING || GetLoadState() == LOAD_STATE_ABORTED)
		return TRUE;
	else
		return FALSE;
	}

CString CJpegPicture::GetTitle() const
	{
	ASSERT_LOCKED(this);

	if (IsHeaderRead() )
		{
		CString f;

		f.Format("JPEG Image (%d by %d)", (int)GetSize().cx, (int)GetSize().cy);
		return f;
		}
	else
		{
		return "";
		}
	}

LPBYTE CJpegPicture::LockBitmapBits()
	{
	ASSERT_LOCKED(this);

	return m_bitmap;
	}

void CJpegPicture::UnlockBitmapBits(LPBYTE bits)
	{
	}

// wingtranslate table with the R and G reversed (because thats the way the JPEG toolkit likes it

static BYTE TranslateTable[216] = 
	{
	0x00, 0x3C, 0x5F, 0x85, 0xA1, 0xFC, 0x21, 0x42, 0x65, 0xA6, 0xC7, 0xFC, 0x27, 0x48, 0x6B, 0x8A, 0xAC, 0xCD, 0x2D,
	0x4E, 0x70, 0x81, 0xB2, 0xD3, 0x33, 0x54, 0x76, 0x95, 0xB8, 0xD9, 0xFA, 0xFA, 0x7B, 0x9B, 0xBE, 0xFE, 0x1D, 0x3D,
	0x60, 0xA2, 0xC4, 0xFC, 0x22, 0x43, 0x66, 0x86, 0xA7, 0xC8, 0x28, 0x49, 0x6C, 0x8B, 0xAD, 0xCE, 0x2E, 0x4F, 0x71,
	0x90, 0xB3, 0xD4, 0x34, 0x55, 0x77, 0x96, 0xB9, 0xDA, 0xFA, 0x5A, 0x7C, 0x9C, 0xBF, 0xDF, 0x1E, 0x3E, 0x61, 0x87,
	0xA3, 0xC5, 0x23, 0x44, 0x67, 0x8C, 0xA8, 0xC9, 0x29, 0x4A, 0x6D, 0xAE, 0xCF, 0xE6, 0x2F, 0x50, 0x72, 0x91, 0xB4,
	0xD5, 0x35, 0x56, 0x97, 0x9D, 0xBA, 0xDB, 0x39, 0x5B, 0xE4, 0xC0, 0xE0, 0xE8, 0x1F, 0x3F, 0x62, 0x83, 0xA4, 0xC6,
	0x24, 0x45, 0x68, 0x82, 0xA9, 0xCA, 0x2A, 0x4B, 0x6E, 0x8D, 0xAF, 0xD0, 0x30, 0x51, 0x73, 0x92, 0xB5, 0xD6, 0x36,
	0x57, 0x78, 0x98, 0xBB, 0xDC, 0x3A, 0x5C, 0x7D, 0x9E, 0xC1, 0xE1, 0x20, 0x40, 0x63, 0x84, 0xA5, 0x80, 0x25, 0x46,
	0x69, 0x88, 0xAA, 0xCB, 0x2B, 0x4C, 0x6F, 0x8E, 0xB0, 0xD1, 0x31, 0x52, 0x74, 0x93, 0xB6, 0xD7, 0x37, 0x58, 0x79,
	0x99, 0xBC, 0xDD, 0x3B, 0x5D, 0x7E, 0x9F, 0xC2, 0xE2, 0xF9, 0x41, 0x64, 0x89, 0x7F, 0xFD, 0x26, 0x47, 0x6A, 0x8F,
	0xAB, 0xCC, 0x2C, 0x4D, 0xE3, 0xB1, 0xD2, 0xE7, 0x32, 0x53, 0x75, 0x94, 0xB7, 0xD8, 0x38, 0x59, 0x7A, 0x9A, 0xBD,
	0xDE, 0xFB, 0x5E, 0xE5, 0xA0, 0xC3, 0xFF
	};

void CJpegPicture::out_line( INT32 scan_line )
	{
	ASSERT(m_bitmap);

	LPBYTE out = m_bitmap + scan_line * RoundUp4(m_size.cx);
	LPBYTE in = m_decode->m_line_buffer[0];

	INT i,ilen = m_size.cx;
	for(i=0;i<ilen;i++)
		{
		*out = TranslateTable[ *in ];
		out++;
		in++;
		}
	}

LOAD_STATE CJpegPicture::OnReadData(LPCBYTE buffer, INT32 buffer_size)
	{
	ASSERT_WORKER_THREAD();
	ASSERT(m_decode);

	m_decode->m_file_buffer.AppendData(buffer, buffer_size);

	TRY 
		{
		if (m_decode->m_jpeg_state == reading_header)
			{
			if (jpeg_read_header(&m_decode->m_cinfo, TRUE) == JPEG_SUSPENDED)
				return LOAD_STATE_LOADING;

		// Set parameters for decompression
			m_decode->m_cinfo.quantize_colors = 1;
			m_decode->m_cinfo.do_fancy_upsampling = FALSE;
			m_decode->m_cinfo.two_pass_quantize = FALSE;
			m_decode->m_cinfo.dct_method = JDCT_FASTEST;
			m_decode->m_cinfo.desired_number_of_colors = 216;

		  	jpeg_start_decompress(&m_decode->m_cinfo);

			DEBUG_LOCK();
			m_size.cx = m_decode->m_cinfo.output_width;
			m_size.cy = m_decode->m_cinfo.output_height;
	
			m_decode->m_jpeg_state = reading_scanlines;		
		  	m_decode->m_line_buffer = (*m_decode->m_cinfo.mem->alloc_sarray)
				((j_common_ptr) &m_decode->m_cinfo, JPOOL_IMAGE, m_size.cx, 1);

			INT32 bitmap_size = RoundUp4(m_size.cx) * m_size.cy;
			m_bitmap = DEBUG_NEW BYTE[ bitmap_size ];
			memset(m_bitmap, TRANS_COLOR, bitmap_size);

			Unlock();
			Notify( CHANGEFLAG_PICTURE_CHANGE );
			}

		BOOL changed = FALSE;
		while( m_decode->m_cinfo.output_scanline < m_decode->m_cinfo.output_height )
			{
			if (jpeg_read_scanlines(&m_decode->m_cinfo, m_decode->m_line_buffer, 1) == 0)
				{
				if (changed)
					{
					Notify( CHANGEFLAG_PICTURE_CHANGE );
					}
				return LOAD_STATE_LOADING;
				}
			out_line( m_decode->m_cinfo.output_scanline - 1 );
			changed = TRUE;
			}
		}
	CATCH( CUserException, e )
		{
		Notify( CHANGEFLAG_PICTURE_CHANGE );
		return LOAD_STATE_ABORTED;
		}
	END_CATCH

	Notify( CHANGEFLAG_PICTURE_CHANGE );
	return LOAD_STATE_COMPLETE;
   	}

LOAD_STATE CJpegPicture::OnEndOfFile()
	{
	ASSERT_WORKER_THREAD();

	DEBUG_LOCK();
	LOAD_STATE l = GetLoadState();
	Unlock();

	if (m_decode)
		{
		if (l == LOAD_STATE_COMPLETE)
			{
			TRY
				{
	  			jpeg_finish_decompress(&m_decode->m_cinfo);
				}
			CATCH( CUserException, e )
				{
				// ignore exceptions that occur here
				}
			END_CATCH
			}

		my_src_ptr src = (my_src_ptr) m_decode->m_cinfo.src;
		if (src->buffer)
			{
			delete [] src->buffer;
			src->buffer = NULL;
			}

 		jpeg_destroy_decompress(&m_decode->m_cinfo);

		delete m_decode;
		m_decode = NULL;
		}
	
	if (l == LOAD_STATE_COMPLETE)
		return LOAD_STATE_COMPLETE;

	return LOAD_STATE_ABORTED;
	}

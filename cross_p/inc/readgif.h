#define _READGIF_H_

#ifndef _PICTURE_H_
#include "picture.h"
#endif

class DECODE_STRUCT;
class REDRAW_PARAMS;

typedef struct { 
		BYTE colortranslate[8];
		BYTE r_error, g_error, b_error, reserved;
		} DITHER_TABLE;

class CGifPicture : public CPicture
	{
public:
	CGifPicture( LPCSTR url, LPCSTR mime_type, const PICTURE_FORMAT_INFO& requested_format);
	~CGifPicture();

// These functions do not require locking
	virtual LPBYTE LockBitmapBits();
	virtual void UnlockBitmapBits(LPBYTE bits);

// These functions require locking
	virtual BOOL IsHeaderRead() const;
	virtual CSize GetSize() const;
	virtual BOOL IsTransparent() const;

	virtual void GetPalette( UINT32 palette[256] ) const;
	virtual void GetFormatInfo(PICTURE_FORMAT_INFO& format_info) const;

protected:
	virtual LOAD_STATE OnReadData(LPCBYTE buffer, INT32 buffer_size);
		// Returns TRUE when the object has finished reading its data
	virtual LOAD_STATE OnEndOfFile();
		// Called at end of file
	
	typedef enum
		{
		loading_header = 0,
		loading_global_color_map,
		loading_extension_blocks,
		loading_begin_extension,
		loading_transp_extension,
		loading_skip_extension,
		loading_local_header,
		loading_local_color_map,
		loading_start_picture,
		loading_picture
		} PICTURE_STATE;

	virtual CString GetTitle() const;

private:
	void out_line(BYTE *pixels, UINT current_y);

	PICTURE_STATE m_picture_state;

	CSize m_size;
	
	BOOL m_broken_gif;
	
	BOOL m_is_interlaced;
	INT m_transp_color_index;

	UINT m_color_map_len;

	DECODE_STRUCT *m_decode_struct;

	LPBYTE m_bitmap;		
		// pointer to the original bitmap
	LPBYTE m_remapped_bitmap;
		// pointer to the remapped bitmap (dithered or remapped, depending on whether
		// we are an interlaced picture, and at which point we are)

	BOOL m_use_standard_palette;
	RGBQUAD m_original_palette[256];
	};


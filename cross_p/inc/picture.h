#define _PICTURE_H_

#ifndef _MIMELOAD_H_
#include "mimeload.h"
#endif

class REDRAW_PARAMS;

/* 
This is the base class for all picture objects such as GIFs or JPEGs
*/

//#define CHANGEFLAG_MIMEHEADER_READ 	0x00000001
//#define CHANGEFLAG_CACHED 		 	0x00000002
#define CHANGEFLAG_PICTURE_CHANGE 		0x00000010

class PICTURE_FORMAT_INFO
	{
public:
	BOOL m_use_standard_palette;
	INT m_bits_per_pixel;
	INT	m_mask_red_lshift, m_mask_green_lshift, m_mask_blue_lshift;
	INT m_mask_red_rshift, m_mask_green_rshift, m_mask_blue_rshift;

// output only
	INT32 m_bytes_per_line;

	inline UINT32 ConvertMaskRGB( BYTE r, BYTE g, BYTE b)
		{ 
		return 	(( ((UINT32)r) >> m_mask_red_rshift) << m_mask_red_lshift) |
				(( ((UINT32)g) >> m_mask_green_rshift) << m_mask_green_lshift) |
				(( ((UINT32)b) >> m_mask_blue_rshift) << m_mask_blue_lshift);
		}
	};

class CPicture : public CMimeObject
	{
public:
	CPicture( LPCSTR url, LPCSTR mime_type, const PICTURE_FORMAT_INFO& requested_format);
	~CPicture();

// These functions can be called without locking
	virtual void UnlockBitmapBits(LPBYTE bits) = 0;

// These functions can be called with locking
	virtual BOOL IsHeaderRead() const = 0;
	virtual CSize GetSize() const = 0;
	virtual BOOL IsTransparent() const = 0;
	
	virtual POSITION FindTagPos( const CTag *tag) const;
	virtual POSITION GetFirstTagPos() const;
	virtual const CTag * GetNextTag(POSITION &walk) const;

	virtual void GetPalette( UINT32 palette[256] ) const = 0;
	virtual void GetFormatInfo(PICTURE_FORMAT_INFO& format_info) const = 0;

	virtual LPBYTE LockBitmapBits( ) = 0;

// friend function that actually draws the picture
	friend void DrawPicture(CPicture *picture, REDRAW_PARAMS& params, const CRect& extent );
	};

void DrawPicture(CPicture *picture, REDRAW_PARAMS& params, const CRect& extent );

#define _READJPEG_H_

#ifndef _PICTURE_H_
#include "picture.h"
#endif

class DECODE_STRUCT;

class CJpegPicture : public CPicture
	{
public:
	CJpegPicture(LPCSTR url, LPCSTR mime_type, const PICTURE_FORMAT_INFO& requested_format);
	~CJpegPicture();

// These functions require locking
	virtual BOOL IsHeaderRead() const;
	virtual CSize GetSize() const;
	virtual BOOL IsTransparent() const;

	virtual void GetPalette( UINT32 palette[256] ) const;
	virtual void GetFormatInfo(PICTURE_FORMAT_INFO& format_info) const;

	virtual CString GetTitle() const;

// These functions do not require locking
	virtual LPBYTE LockBitmapBits();
	virtual void UnlockBitmapBits(LPBYTE bits);

protected:
	virtual LOAD_STATE OnReadData(LPCBYTE buffer, INT32 buffer_size);
		// Returns TRUE when the object has finished reading its data
	virtual LOAD_STATE OnEndOfFile();
		// Called at end of file
	
private:
	CSize m_size;
	LPBYTE m_bitmap;
	DECODE_STRUCT *m_decode;

	void out_line( INT32 scan_line );
	};

#define _BITBLT_H_

#pragma once

#define TRANS_COLOR 1

void TranslateIndexs( const RGBQUAD *original_palette, LPCBYTE input, LPBYTE output, INT amount, INT transparent_color);
	// translates a set of palette indexes to the system palette, mapping each color as close
	// to the system palette as it can get, and translating the transparent color to TRANS_COLOR

void CopyDIBBits( void * pDest,
  void const *pSource, DWORD dwWidth, DWORD dwHeight, DWORD dwScanD,
  DWORD dwScanS );

void TransCopyDIBBits( void * pDest,
  void const *pSource, DWORD dwWidth, DWORD dwHeight, DWORD dwScanD,
  long dwScanS);

void ClearSystemPalette(void);

CPalette *GetHalftonePalette();

extern const BYTE aDividedBy51Rounded[256];
extern const BYTE aDividedBy51RoundedTimes6[256];
extern const BYTE aDividedBy51RoundedTimes36[256];

extern const INT8 aDividedBy51RoundedError[256];
extern const BYTE aDividedBy51[256];
extern const BYTE aModulo51[256];
extern const BYTE aTimes6[6];
extern const BYTE aTimes36[6];

extern const BYTE aHalftone8x8[64];
extern const BYTE aHalftone16x16[256];
extern const BYTE aHalftone4x4_1[16];
extern const BYTE aHalftone4x4_2[16];
extern const BYTE aHalftone2x2[4];

extern const BYTE aTranslateToPaletteEntry[216];

class CFsDither
	 {
public:
	CFsDither(RGBQUAD original_palette[256], INT xsize, INT bytes_per_row, INT transparent_color);
	~CFsDither();

	void Dither( LPCBYTE input_buf, LPBYTE output_buf, INT32 num_rows );

private:
	INT m_row_index;		
  	INT m_row_width, m_bytes_per_row;
	INT m_transparent_color;
	
  	/* Variables for Floyd-Steinberg dithering */

	LPINT m_error_limiter;
  	LPINT m_fserrors; 		/* accumulated errors */
  	BOOL m_on_odd_row;		/* flag to remember which row we are on */

	RGBQUAD m_picture_palette[256];
	};

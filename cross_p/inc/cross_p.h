#ifndef _WINDOWS
#define _MACINTOSH
#endif

#ifdef _MACINTOSH
#include "mfc_mac.h"

#include <stdlib.h>
#include <unix.h>
#include <MWException.h>

#else
#include "stdafx.h"

typedef BYTE const *LPCBYTE;


#ifdef WIN32

typedef unsigned char UINT8;
typedef signed char INT8;

typedef unsigned short UINT16;
typedef short INT16;

typedef unsigned int UINT32;
typedef int INT32;

#else
typedef long INT32;
#endif

class CInt32Array : protected CDWordArray
	{
public:
	inline CInt32Array() : CDWordArray()
		{ }
	inline int GetSize() const
		{ return CDWordArray::GetSize(); }
	inline int GetUpperBound() const
		{ return CDWordArray::GetUpperBound(); }
	inline void SetSize(int nNewSize, int nGrowBy = -1)
		{ CDWordArray::SetSize(nNewSize, nGrowBy); }
	inline void FreeExtra()
		{ CDWordArray::FreeExtra(); }
	inline void RemoveAll()
		{ CDWordArray::RemoveAll(); }
	inline void Push( INT32 l )
		{ CDWordArray::Add( (DWORD)l); }
	inline INT32 Pop()
		{ 
		ASSERT( GetSize()>0);	

		INT32 r = (INT32)CDWordArray::GetAt( CDWordArray::GetUpperBound() );	
		CDWordArray::RemoveAt( CDWordArray::GetUpperBound() );
		return r;
		}
	inline INT32 GetAt(int nIndex) const
		{ return (INT32) CDWordArray::GetAt(nIndex); }
	inline void SetAt(int nIndex, INT32 newElement)
		{ CDWordArray::SetAt(nIndex, newElement); }
	inline INT32& ElementAt(int nIndex)
		{ return (INT32&) CDWordArray::ElementAt(nIndex); }
	inline int Add(DWORD newElement)
		{ return CDWordArray::Add( (INT32)newElement); }
	inline INT32 operator[](int nIndex) const
		{ return (INT32) CDWordArray::GetAt(nIndex); }
	inline INT32& operator[](int nIndex)
		{ return (INT32&) CDWordArray::ElementAt(nIndex); }
	inline void InsertAt(int nIndex, INT32 newElement, int nCount = 1)
		{ InsertAt(nIndex, (DWORD)newElement, nCount); }
	inline void RemoveAt(int nIndex, int nCount = 1)
		{ RemoveAt(nIndex, nCount); }
	inline void InsertAt(int nStartIndex, CDWordArray* pNewArray)
		{ InsertAt(nStartIndex, pNewArray); }
	};

int ErrorExit(LPCSTR string);
// rounds up a number to the nearest multiple of 4
inline INT32 RoundUp4(INT32 number) 
	{ return (number+3)&0xfffffffc; }

#endif

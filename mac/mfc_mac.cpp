#include "cross_p.h"

#include <Types.h>
#include <ctype.h>
#include <stdarg.h>

void	ValidateMemory();

#define new DEBUG_NEW

CRect::CRect()
	{
#ifndef NDEBUG
	left = -1; right = -1; top = -1; bottom = -1;
#endif
	}
	
CRect::CRect(const CRect& r)
	{
	left = r.left; right = r.right; top = r.top; bottom = r.bottom;
	}
	
CRect::CRect(long _left, long _top, long _right, long _bottom)
	{
	left = _left; right = _right; top = _top; bottom = _bottom;
	}
	
CRect& CRect::operator	= (const CRect& r)
	{
	left = r.left; right = r.right;  top = r.top;  bottom = r.bottom;
	return *this;
	}

void CRect::SetRect(long _left, long _top, long _right, long _bottom)
	{
	left = _left; right = _right; top = _top; bottom = _bottom;
	}

void ErrorExit(const char *string)
	{
	Str255 buffer;
	
	sprintf((LPSTR)(buffer+1), "Fatal error:\r%s", string);
	buffer[0] = strlen((LPSTR)(buffer+1));	
	
	ParamText(buffer,0,0,0);
	Alert(128,NULL);
	ExitToShell();
	}


void ShowError(const char *string)
	{
	Str255 buffer;
	
	sprintf((LPSTR)(buffer+1), "Error:\r%s", string);
	buffer[0] = strlen((LPSTR)(buffer+1));	
	
	ParamText(buffer,0,0,0);
	Alert(128,NULL);
	}
	
#ifndef NDEBUG
int AssertFail(const char *filename, int line)
	{
	Str255 buffer;
	
	sprintf((LPSTR)(buffer+1), "Assertion failed in %s, line %d\r", filename, line);
	buffer[0] = strlen((LPSTR)(buffer+1));	
	
	ParamText(buffer,0,0,0);
	Alert(128,NULL);
	ExitToShell();
	return 0;
	}
#endif

CPlex* CPlex::Create(CPlex*& pHead, long nMax, long cbElement)
{
	ASSERT(nMax > 0 && cbElement > 0);
	CPlex* p = (CPlex*) (new BYTE[sizeof(CPlex) + nMax * cbElement]);
			// may throw exception
	p->nMax = nMax;
	p->nCur = 0;
	p->pNext = pHead;
	pHead = p;  // change head (adds in reverse order for simplicity)
	return p;
}

void CPlex::FreeDataChain()     // free this one and links
{
	CPlex* p = this;
	while (p != NULL)
	{
		BYTE* bytes = (BYTE*) p;
		CPlex* pNext = p->pNext;
		delete bytes;
		p = pNext;
	}
}

CPtrList::CPtrList(long nBlockSize)
{
	ASSERT(nBlockSize > 0);

	m_nCount = 0;
	m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
	m_pBlocks = NULL;
	m_nBlockSize = nBlockSize;
}

void CPtrList::RemoveAll()
{
	// destroy elements


	m_nCount = 0;
	m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
	m_pBlocks->FreeDataChain();
	m_pBlocks = NULL;
}

CPtrList::~CPtrList()
{
	RemoveAll();
	ASSERT(m_nCount == 0);
}

/////////////////////////////////////////////////////////////////////////////
// Node helpers
/*
 * Implementation note: CNode's are stored in CPlex blocks and
 *  chained together. Free blocks are maintained in a singly linked list
 *  using the 'pNext' member of CNode with 'm_pNodeFree' as the head.
 *  Used blocks are maintained in a doubly linked list using both 'pNext'
 *  and 'pPrev' as links and 'm_pNodeHead' and 'm_pNodeTail'
 *   as the head/tail.
 *
 * We never free a CPlex block unless the List is destroyed or RemoveAll()
 *  is used - so the total number of CPlex blocks may grow large depending
 *  on the maximum past size of the list.
 */

CPtrList::CNode*
CPtrList::NewNode(CPtrList::CNode* pPrev, CPtrList::CNode* pNext)
{
	if (m_pNodeFree == NULL)
	{
		// add another block
		CPlex* pNewBlock = CPlex::Create(m_pBlocks, m_nBlockSize,
				 sizeof(CNode));

		// chain them into free list
		CNode* pNode = (CNode*) pNewBlock->data();
		// free in reverse order to make it easier to debug
		pNode += m_nBlockSize - 1;
		for (long i = m_nBlockSize-1; i >= 0; i--, pNode--)
		{
			pNode->pNext = m_pNodeFree;
			m_pNodeFree = pNode;
		}
	}
	ASSERT(m_pNodeFree != NULL);  // we must have something

	CPtrList::CNode* pNode = m_pNodeFree;
	m_pNodeFree = m_pNodeFree->pNext;
	pNode->pPrev = pPrev;
	pNode->pNext = pNext;
	m_nCount++;
	ASSERT(m_nCount > 0);  // make sure we don't overflow


	memset(&pNode->data, 0, sizeof(void*));  // zero fill

	return pNode;
}

void CPtrList::FreeNode(CPtrList::CNode* pNode)
{

	pNode->pNext = m_pNodeFree;
	m_pNodeFree = pNode;
	m_nCount--;
	ASSERT(m_nCount >= 0);  // make sure we don't underflow
}

/////////////////////////////////////////////////////////////////////////////

POSITION CPtrList::AddHead(void* newElement)
{
	CNode* pNewNode = NewNode(NULL, m_pNodeHead);
	pNewNode->data = newElement;
	if (m_pNodeHead != NULL)
		m_pNodeHead->pPrev = pNewNode;
	else
		m_pNodeTail = pNewNode;
	m_pNodeHead = pNewNode;
	return (POSITION) pNewNode;
}

POSITION CPtrList::AddTail(void* newElement)
{
	CNode* pNewNode = NewNode(m_pNodeTail, NULL);
	pNewNode->data = newElement;
	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = pNewNode;
	else
		m_pNodeHead = pNewNode;
	m_pNodeTail = pNewNode;
	return (POSITION) pNewNode;
}

void CPtrList::AddHead(CPtrList* pNewList)
{
	ASSERT(pNewList != NULL);
	
	// add a list of same elements to head (maintain order)
	POSITION pos = pNewList->GetTailPosition();
	while (pos != NULL)
		AddHead(pNewList->GetPrev(pos));
}

void CPtrList::AddTail(CPtrList* pNewList)
{
	ASSERT(pNewList != NULL);
	
	// add a list of same elements
	POSITION pos = pNewList->GetHeadPosition();
	while (pos != NULL)
		AddTail(pNewList->GetNext(pos));
}

void* CPtrList::RemoveHead()
{
	ASSERT(m_pNodeHead != NULL);  // don't call on empty list !!!

	CNode* pOldNode = m_pNodeHead;
	void* returnValue = pOldNode->data;

	m_pNodeHead = pOldNode->pNext;
	if (m_pNodeHead != NULL)
		m_pNodeHead->pPrev = NULL;
	else
		m_pNodeTail = NULL;
	FreeNode(pOldNode);
	return returnValue;
}

void* CPtrList::RemoveTail()
{
	ASSERT(m_pNodeTail != NULL);  // don't call on empty list !!!

	CNode* pOldNode = m_pNodeTail;
	void* returnValue = pOldNode->data;

	m_pNodeTail = pOldNode->pPrev;
	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = NULL;
	else
		m_pNodeHead = NULL;
	FreeNode(pOldNode);
	return returnValue;
}

POSITION CPtrList::InsertBefore(POSITION position, void* newElement)
{
	if (position == NULL)
		return AddHead(newElement); // insert before nothing -> head of the list

	// Insert it before position
	CNode* pOldNode = (CNode*) position;
	CNode* pNewNode = NewNode(pOldNode->pPrev, pOldNode);
	pNewNode->data = newElement;

	if (pOldNode->pPrev != NULL)
	{
		pOldNode->pPrev->pNext = pNewNode;
	}
	else
	{
		ASSERT(pOldNode == m_pNodeHead);
		m_pNodeHead = pNewNode;
	}
	pOldNode->pPrev = pNewNode;
	return (POSITION) pNewNode;
}

POSITION CPtrList::InsertAfter(POSITION position, void* newElement)
{
	if (position == NULL)
		return AddTail(newElement); // insert after nothing -> tail of the list

	// Insert it before position
	CNode* pOldNode = (CNode*) position;
	CNode* pNewNode = NewNode(pOldNode, pOldNode->pNext);
	pNewNode->data = newElement;

	if (pOldNode->pNext != NULL)
	{
		pOldNode->pNext->pPrev = pNewNode;
	}
	else
	{
		ASSERT(pOldNode == m_pNodeTail);
		m_pNodeTail = pNewNode;
	}
	pOldNode->pNext = pNewNode;
	return (POSITION) pNewNode;
}

void CPtrList::RemoveAt(POSITION position)
{
	CNode* pOldNode = (CNode*) position;

	// remove pOldNode from list
	if (pOldNode == m_pNodeHead)
	{
		m_pNodeHead = pOldNode->pNext;
	}
	else
	{
		pOldNode->pPrev->pNext = pOldNode->pNext;
	}
	if (pOldNode == m_pNodeTail)
	{
		m_pNodeTail = pOldNode->pPrev;
	}
	else
	{
		pOldNode->pNext->pPrev = pOldNode->pPrev;
	}
	FreeNode(pOldNode);
}


/////////////////////////////////////////////////////////////////////////////
// slow operations

POSITION CPtrList::FindIndex(long nIndex) const
{
	ASSERT(nIndex >= 0);

	if (nIndex >= m_nCount)
		return NULL;  // went too far

	CNode* pNode = m_pNodeHead;
	while (nIndex--)
	{
		pNode = pNode->pNext;
	}
	return (POSITION) pNode;
}

POSITION CPtrList::Find(void* searchValue, POSITION startAfter) const
{
	CNode* pNode = (CNode*) startAfter;
	if (pNode == NULL)
	{
		pNode = m_pNodeHead;  // start at head
	}
	else
	{
		pNode = pNode->pNext;  // start after the one specified
	}

	for (; pNode != NULL; pNode = pNode->pNext)
		if (pNode->data == searchValue)
			return (POSITION) pNode;
	return NULL;
}




/////////////////////////////////////////////////////////////////////////////
// static class data, special inlines

// For an empty string, m_???Data will point here
// (note: avoids a lot of NULL pointer tests when we call standard
//  C runtime libraries

char afxChNil = '\0';

// for creating empty key strings
const CString afxEmptyString;

void CString::Init()
{
	m_nDataLength = m_nAllocLength = 0;
	m_pchData = (LPSTR)&afxChNil;
}

// declared static
void CString::SafeDelete(LPSTR lpch)
{
	if (lpch != (LPSTR)&afxChNil)
		delete [](lpch);
}

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction

CString::CString()
{
	Init();
}

CString::CString(const CString& stringSrc)
{
	// if constructing a CString from another CString, we make a copy of the
	// original string data to enforce value semantics (i.e. each string
	// gets a copy of its own

	stringSrc.AllocCopy(*this, stringSrc.m_nDataLength, 0, 0);
}

void CString::AllocBuffer(long nLen)
 // always allocate one extra character for '\0' termination
 // assumes [optimistically] that data length will equal allocation length
{
	ASSERT(nLen >= 0);
	ASSERT(nLen <= 0x7ffffff - 1);    // max size (enough room for 1 extra)

	if (nLen == 0)
	{
		Init();
	}
	else
	{
		m_pchData = (char *)(new char[nLen+1]);       // may throw an exception
			
		m_pchData[nLen] = '\0';
		m_nDataLength = nLen;
		m_nAllocLength = nLen;
	}
}

void CString::Empty()
{
	SafeDelete(m_pchData);
	Init();
	ASSERT(m_nDataLength == 0);
	ASSERT(m_nAllocLength == 0);
}

CString::~CString()
 //  free any attached data
{
	SafeDelete(m_pchData);
}

//////////////////////////////////////////////////////////////////////////////
// Helpers for the rest of the implementation

static inline long SafeStrlen(LPCSTR lpsz)
{
	return (lpsz == NULL) ? 0 : strlen(lpsz);
}

void CString::AllocCopy(CString& dest, long nCopyLen, long nCopyIndex,
	 long nExtraLen) const
{
	// will clone the data attached to this string
	// allocating 'nExtraLen' characters
	// Places results in uninitialized string 'dest'
	// Will copy the part or all of original data to start of new string

	long nNewLen = nCopyLen + nExtraLen;

	if (nNewLen == 0)
	{
		dest.Init();
	}
	else
	{
		dest.AllocBuffer(nNewLen);
		memcpy(dest.m_pchData, &m_pchData[nCopyIndex], nCopyLen*sizeof(char));
	}
}

//////////////////////////////////////////////////////////////////////////////
// More sophisticated construction

CString::CString(LPCSTR lpsz)
{
	long nLen;
	if ((nLen = SafeStrlen(lpsz)) == 0)
		Init();
	else
	{
		AllocBuffer(nLen);
		memcpy(m_pchData, lpsz, nLen*sizeof(char));
	}
}

//////////////////////////////////////////////////////////////////////////////
// Assignment operators
//  All assign a new value to the string
//      (a) first see if the buffer is big enough
//      (b) if enough room, copy on top of old buffer, set size and type
//      (c) otherwise free old string data, and create a new one
//
//  All routines return the new string (but as a 'const CString&' so that
//      assigning it again will cause a copy, eg: s1 = s2 = "hi there".
//

void CString::AssignCopy(long nSrcLen, LPCSTR lpszSrcData)
{
	// check if it will fit
	if (nSrcLen > m_nAllocLength)
	{
		// it won't fit, allocate another one
		Empty();
		AllocBuffer(nSrcLen);
	}
	if (nSrcLen != 0)
		memcpy(m_pchData, lpszSrcData, nSrcLen*sizeof(char));
	m_nDataLength = nSrcLen;
	m_pchData[nSrcLen] = '\0';
}

const CString& CString::operator=(const CString& stringSrc)
{
	AssignCopy(stringSrc.m_nDataLength, stringSrc.m_pchData);
	return *this;
}

const CString& CString::operator=(LPCSTR lpsz)
{
	AssignCopy(SafeStrlen(lpsz), lpsz);
	return *this;
}

//////////////////////////////////////////////////////////////////////////////
// concatenation

// NOTE: "operator+" is done as friend functions for simplicity

//      There are three variants:
//          CString + CString
// and for ? = char, LPCSTR
//          CString + ?
//          ? + CString

void CString::ConcatCopy(long nSrc1Len, LPCSTR lpszSrc1Data,
	long nSrc2Len, LPCSTR lpszSrc2Data)
{
  // -- master concatenation routine
  // Concatenate two sources
  // -- assume that 'this' is a new CString object

	long nNewLen = nSrc1Len + nSrc2Len;
	AllocBuffer(nNewLen);
	memcpy(m_pchData, lpszSrc1Data, nSrc1Len*sizeof(char));
	memcpy(&m_pchData[nSrc1Len], lpszSrc2Data, nSrc2Len*sizeof(char));
}

CString operator+(const CString& string1, const CString& string2)
{
	CString s;
	s.ConcatCopy(string1.m_nDataLength, string1.m_pchData,
		string2.m_nDataLength, string2.m_pchData);
	return s;
}

CString operator+(const CString& string, LPCSTR lpsz)
{
	CString s;
	s.ConcatCopy(string.m_nDataLength, string.m_pchData, SafeStrlen(lpsz), lpsz);
	return s;
}

CString operator+(LPCSTR lpsz, const CString& string)
{
	CString s;
	s.ConcatCopy(SafeStrlen(lpsz), lpsz, string.m_nDataLength, string.m_pchData);
	return s;
}

//////////////////////////////////////////////////////////////////////////////
// concatenate in place

void CString::ConcatInPlace(long nSrcLen, LPCSTR lpszSrcData)
{
	//  -- the main routine for += operators

	// if the buffer is too small, or we have a width mis-match, just
	//   allocate a new buffer (slow but sure)
	if (m_nDataLength + nSrcLen > m_nAllocLength)
	{
		// we have to grow the buffer, use the Concat in place routine
		LPSTR lpszOldData = m_pchData;
		ConcatCopy(m_nDataLength, lpszOldData, nSrcLen, lpszSrcData);
		ASSERT(lpszOldData != NULL);
		SafeDelete(lpszOldData);
	}
	else
	{
		// fast concatenation when buffer big enough
		memcpy(&m_pchData[m_nDataLength], lpszSrcData, nSrcLen*sizeof(char));
		m_nDataLength += nSrcLen;
	}
	ASSERT(m_nDataLength <= m_nAllocLength);
	m_pchData[m_nDataLength] = '\0';
}

const CString& CString::operator+=(LPCSTR lpsz)
{
	ConcatInPlace(SafeStrlen(lpsz), lpsz);
	return *this;
}

const CString& CString::operator+=(char ch)
{
	ConcatInPlace(1, &ch);
	return *this;
}

const CString& CString::operator+=(const CString& string)
{
	ConcatInPlace(string.m_nDataLength, string.m_pchData);
	return *this;
}

///////////////////////////////////////////////////////////////////////////////
// Advanced direct buffer access

LPSTR CString::GetBuffer(long nMinBufLength)
{
	ASSERT(nMinBufLength >= 0);

	if (nMinBufLength > m_nAllocLength)
	{
		// we have to grow the buffer
		LPSTR lpszOldData = m_pchData;
		long nOldLen = m_nDataLength;        // AllocBuffer will tromp it

		AllocBuffer(nMinBufLength);
		memcpy(m_pchData, lpszOldData, nOldLen*sizeof(char));
		m_nDataLength = nOldLen;
		m_pchData[m_nDataLength] = '\0';

		SafeDelete(lpszOldData);
	}

	// return a polonger to the character storage for this string
	ASSERT(m_pchData != NULL);
	return m_pchData;
}

void CString::ReleaseBuffer(long nNewLength)
{
	if (nNewLength == -1)
		nNewLength = strlen(m_pchData); // zero terminated

	ASSERT(nNewLength <= m_nAllocLength);
	m_nDataLength = nNewLength;
	m_pchData[m_nDataLength] = '\0';
}

LPSTR CString::GetBufferSetLength(long nNewLength)
{
	ASSERT(nNewLength >= 0);

	GetBuffer(nNewLength);
	m_nDataLength = nNewLength;
	m_pchData[m_nDataLength] = '\0';
	return m_pchData;
}

void CString::FreeExtra()
{
	ASSERT(m_nDataLength <= m_nAllocLength);
	if (m_nDataLength != m_nAllocLength)
	{
		LPSTR lpszOldData = m_pchData;
		AllocBuffer(m_nDataLength);
		memcpy(m_pchData, lpszOldData, m_nDataLength*sizeof(char));
		ASSERT(m_pchData[m_nDataLength] == '\0');
		SafeDelete(lpszOldData);
	}
	ASSERT(m_pchData != NULL);
}

///////////////////////////////////////////////////////////////////////////////
// Commonly used routines (rarely used routines in STREX.CPP)

long CString::Find(char ch) const
{
	// find first single character
	LPSTR lpsz = strchr(m_pchData, ch);

	// return -1 if not found and index otherwise
	return (lpsz == NULL) ? -1 : (long)(lpsz - m_pchData);
}

long CString::FindOneOf(LPCSTR lpszCharSet) const
{
	LPSTR lpsz = strpbrk(m_pchData, lpszCharSet);
	return (lpsz == NULL) ? -1 : (long)(lpsz - m_pchData);
}

///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// More sophisticated construction

CString::CString(char ch, long nLength)
{
	if (nLength < 1)
	{
		// return empty string if invalid repeat count
		Init();
	}
	else
	{
		AllocBuffer(nLength);
		memset(m_pchData, ch, nLength);
	}
}

CString::CString(LPCSTR lpch, long nLength)
{
	if (nLength == 0)
		Init();
	else
	{
		AllocBuffer(nLength);
		memcpy(m_pchData, lpch, nLength*sizeof(char));
	}
}

//////////////////////////////////////////////////////////////////////////////
// Assignment operators

const CString& CString::operator=(char ch)
{
	AssignCopy(1, &ch);
	return *this;
}

//////////////////////////////////////////////////////////////////////////////
// less common string expressions

CString operator+(const CString& string1, char ch)
{
	CString s;
	s.ConcatCopy(string1.m_nDataLength, string1.m_pchData, 1, &ch);
	return s;
}

CString operator+(char ch, const CString& string)
{
	CString s;
	s.ConcatCopy(1, &ch, string.m_nDataLength, string.m_pchData);
	return s;
}

//////////////////////////////////////////////////////////////////////////////
// Very simple sub-string extraction

CString CString::Mid(long nFirst) const
{
	return Mid(nFirst, m_nDataLength - nFirst);
}

CString CString::Mid(long nFirst, long nCount) const
{
	ASSERT(nFirst >= 0);
	ASSERT(nCount >= 0);

	// out-of-bounds requests return sensible things
	if (nFirst + nCount > m_nDataLength)
		nCount = m_nDataLength - nFirst;
	if (nFirst > m_nDataLength)
		nCount = 0;

	CString dest;
	AllocCopy(dest, nCount, nFirst, 0);
	return dest;
}

CString CString::Right(long nCount) const
{
	ASSERT(nCount >= 0);

	if (nCount > m_nDataLength)
		nCount = m_nDataLength;

	CString dest;
	AllocCopy(dest, nCount, m_nDataLength-nCount, 0);
	return dest;
}

CString CString::Left(long nCount) const
{
	ASSERT(nCount >= 0);

	if (nCount > m_nDataLength)
		nCount = m_nDataLength;

	CString dest;
	AllocCopy(dest, nCount, 0, 0);
	return dest;
}

// strspn equivalent
CString CString::SpanIncluding(LPCSTR lpszCharSet) const
{
	return Left(strspn(m_pchData, lpszCharSet));
}

// strcspn equivalent
CString CString::SpanExcluding(LPCSTR lpszCharSet) const
{
	return Left(strcspn(m_pchData, lpszCharSet));
}

//////////////////////////////////////////////////////////////////////////////
// Finding

long CString::ReverseFind(char ch) const
{
	// find last single character
	LPSTR lpsz = strrchr(m_pchData, ch);

	// return -1 if not found, distance from beginning otherwise
	return (lpsz == NULL) ? -1 : (long)(lpsz - m_pchData);
}

// find a sub-string (like strstr)
long CString::Find(LPCSTR lpszSub) const
{
	// find first matching substring
	LPSTR lpsz = strstr(m_pchData, lpszSub);

	// return -1 for not found, distance from beginning otherwise
	return (lpsz == NULL) ? -1 : (long)(lpsz - m_pchData);
}

void CString::GetPString( Str255 s ) const
	{
	s[0] = GetLength();
	memcpy(s+1, m_pchData, s[0]);
	}		

/////////////////////////////////////////////////////////////////////////////
// CString formatting

#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000

#define _tcsinc(x) ((x)+1)

// formatting (using wsprlongf style formatting)
void CString::Format(LPCSTR lpszFormat, ...)
{
	va_list argList;
	long nMaxLen = strlen(lpszFormat) + 256;
	
	va_start(argList, lpszFormat);  
	GetBuffer(nMaxLen);
	vsprintf(m_pchData, lpszFormat, argList);
	
	ReleaseBuffer();
	va_end(argList);
}


/////////////////////////////////////////////////////////////////////////////

CLongStack::CLongStack()
{
	m_pData = NULL;
	m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

CLongStack::~CLongStack()
{
	if (m_pData)
		delete[] (BYTE*)m_pData;
}

inline long min(long a, long b)
	{
	if (a < b)
		return a;
	else
		return b;
	}
	
inline long max(long a, long b)
	{
	if (a > b)
		return a;
	else
		return b;
	}

void CLongStack::SetSize(long nNewSize, long nGrowBy)
{
	ASSERT(nNewSize >= 0);

	if (nGrowBy != -1)
		m_nGrowBy = nGrowBy;  // set new size

	if (nNewSize == 0)
	{
		// shrink to nothing
		if (m_pData)
			delete[] (BYTE*)m_pData;
		m_pData = NULL;
		m_nSize = m_nMaxSize = 0;
	}
	else if (m_pData == NULL)
	{
		// create one with exact size
		m_pData = (long*) new BYTE[nNewSize * sizeof(long)];

		memset(m_pData, 0, nNewSize * sizeof(long));  // zero fill

		m_nSize = m_nMaxSize = nNewSize;
	}
	else if (nNewSize <= m_nMaxSize)
	{
		// it fits
		if (nNewSize > m_nSize)
		{
			// initialize the new elements

			memset(&m_pData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(long));

		}

		m_nSize = nNewSize;
	}
	else
	{
		// otherwise, grow array
		long nGrowBy = m_nGrowBy;
		if (nGrowBy == 0)
		{
			// heuristically determine growth when nGrowBy == 0
			//  (this avoids heap fragmentation in many situations)
			nGrowBy = min(1024, max(4, m_nSize / 8));
		}
		long nNewMax;
		if (nNewSize < m_nMaxSize + nGrowBy)
			nNewMax = m_nMaxSize + nGrowBy;  // granularity
		else
			nNewMax = nNewSize;  // no slush

		ASSERT(nNewMax >= m_nMaxSize);  // no wrap around
		
		long* pNewData = (long*) (new BYTE[nNewMax * sizeof(long)] );

		// copy new data from old
		memcpy(pNewData, m_pData, m_nSize * sizeof(long));

		// construct remaining elements
		ASSERT(nNewSize > m_nSize);

		memset(&pNewData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(long));


		// get rid of old stuff (note: no destructors called)
		if (m_pData)
			delete[] (BYTE*)m_pData;
		m_pData = pNewData;
		m_nSize = nNewSize;
		m_nMaxSize = nNewMax;
	}
}

void CLongStack::FreeExtra()
{
	if (m_nSize != m_nMaxSize)
	{
		// shrink to desired size
		long* pNewData = NULL;
		if (m_nSize != 0)
		{
			pNewData = (long*) new BYTE[m_nSize * sizeof(long)];
			// copy new data from old
			memcpy(pNewData, m_pData, m_nSize * sizeof(long));
		}

		// get rid of old stuff (note: no destructors called)
		if (m_pData)
			delete[] (BYTE*)m_pData;
		m_pData = pNewData;
		m_nMaxSize = m_nSize;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CLongStack::SetAtGrow(long nIndex, long newElement)
{
	ASSERT(nIndex >= 0);

	if (nIndex >= m_nSize)
		SetSize(nIndex+1);
	m_pData[nIndex] = newElement;
}

void CLongStack::InsertAt(long nIndex, long newElement, long nCount)
{
	ASSERT(nIndex >= 0);    // will expand to meet need
	ASSERT(nCount > 0);     // zero or negative size not allowed

	if (nIndex >= m_nSize)
	{
		// adding after the end of the array
		SetSize(nIndex + nCount);  // grow so nIndex is valid
	}
	else
	{
		// inserting in the middle of the array
		long nOldSize = m_nSize;
		SetSize(m_nSize + nCount);  // grow it to new size
		// shift old data up to fill gap
		memmove(&m_pData[nIndex+nCount], &m_pData[nIndex],
			(nOldSize-nIndex) * sizeof(long));

		// re-init slots we copied from

		memset(&m_pData[nIndex], 0, nCount * sizeof(long));

	}

	// insert new value in the gap
	ASSERT(nIndex + nCount <= m_nSize);
	while (nCount--)
		m_pData[nIndex++] = newElement;
}

void CLongStack::RemoveAt(long nIndex, long nCount)
{
	ASSERT(nIndex >= 0);
	ASSERT(nCount >= 0);
	ASSERT(nIndex + nCount <= m_nSize);

	// just remove a range
	long nMoveCount = m_nSize - (nIndex + nCount);

	if (nMoveCount)
		memcpy(&m_pData[nIndex], &m_pData[nIndex + nCount],
			nMoveCount * sizeof(long));
	m_nSize -= nCount;
}

void CLongStack::InsertAt(long nStartIndex, CLongStack* pNewArray)
{
	ASSERT(pNewArray != NULL);
	ASSERT(nStartIndex >= 0);

	if (pNewArray->GetSize() > 0)
	{
		InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
		for (long i = 0; i < pNewArray->GetSize(); i++)
			SetAt(nStartIndex + i, pNewArray->GetAt(i));
	}
}

void CLongStack::Push(long value)
	{ 
	Add(value); 
	}
	
long CLongStack::Pop()
	{
	ASSERT( GetSize()>0);
	long r = GetAt( GetUpperBound() );
	RemoveAt( GetUpperBound() );
	return r;
	}


void strrev(char *str)
	{
	*str =0;
	ASSERT(FALSE);
	}
	
void strupr(char *str)
	{
	while(*str)
		{
		*str = toupper(*str);
		str++;
		}
	}
	
void strlwr(char *str)
	{
	while(*str)
		{
		*str = tolower(*str);
		str++;
		}
	}

BOOL stricmp(const char *str1, const char *str2)
	{
	while(1)
		{
		char c1 = toupper(*str1);
		char c2 = toupper(*str2);
		
		if (c1 == 0)
			{
			if (c2 == 0)
				return 0;
			else
				return 1;
			}
			
		if (c2 == 0)
			return -1;
			
		char c = c1 - c2;
		if (c != 0)
			return c;
		}
	}

// returns TRUE if some memory was freed
BOOL FreeSomeMemory();

#ifndef NDEBUG
#undef new

typedef struct _HEADER
	{
	long guard;
	LPCSTR filename;
	int lineno;
	size_t size;
	struct _HEADER *prev, *next;
	} HEADER;
	
HEADER *first;
long amountalloc, objectcount = 0;

void	DumpMemory()
	{
	HEADER *walk = first;
	Str255 buffer;
	
	int count = 0;
	
	while(walk && count < 2)
		{
		if(walk->guard != 0x12345678)
			ErrorExit("Memory corrupted!");
		
		sprintf( (char *)buffer+1, "%s:%d\r", walk->filename, walk->lineno);
		
		buffer[0] = strlen( (char *)buffer+1);
		
		ParamText(buffer,0,0,0);
		Alert(128,NULL);
		
		walk = walk->next;
		count++;
		}
	}	
	
void	ValidateMemory()
	{
	HEADER *walk = first;
	
	int count = 0;
	
	while(walk && count <= objectcount)
		{
		if(walk->guard != 0x12345678)
			ErrorExit("Memory corrupted!");
		
		BYTE *bp = (BYTE *)(walk+1);
		ASSERT( *(bp + walk->size) == 0xcc);

		
		if (walk->prev)
			{
			ASSERT( walk->prev->next == walk );
			}
		else
			{
			ASSERT( walk == first);
			}
			
		if (walk->next)
			{
			ASSERT( walk->next->prev == walk );
			}
			
		walk = walk->next;
		count++;
		}
	if (objectcount > count)
		ErrorExit("heap corrupted");
	}	
		

void *operator new(size_t size)
	{
	HEADER * p = (HEADER *)NewPtr(size + sizeof(HEADER) + sizeof(char) );
	while(!p)
		{
		if (!FreeSomeMemory() )
			{
			char buffer[256];
			
			sprintf(buffer,"new: %d bytes needed by unknown module", size);
			ErrorExit(buffer);
			}
		p = (HEADER *)NewPtr(size + sizeof(HEADER) + sizeof(char) );
		}
		
	ASSERT(p);
	p->filename = "unknown";
	p->lineno = 0;	
	p->size = size;
	p->next = first;
	p->prev = NULL;
	p->guard = 0x12345678;
	if (first)
		first->prev = p;
	
	BYTE *bp = (BYTE *)(p+1);
	*(bp + size)= 0xcc;
	
	first = p;
	amountalloc += size;
	objectcount++;
	return (p+1);
	}

void *operator new(size_t size, LPCSTR file, int lineno )
	{
	HEADER * p = (HEADER *)NewPtr(size + sizeof(HEADER) + sizeof(char) );
	while(!p)
		{
		if (!FreeSomeMemory() )
			{
			char buffer[256];
			
			sprintf(buffer,"new: %d bytes needed (module %s, line %d)", size, file, lineno);
			ErrorExit(buffer);
			}
		p = (HEADER *)NewPtr(size + sizeof(HEADER) + sizeof(char) );
		}
			
	ASSERT(size > 0);
	
	p->size = size;
	p->filename = file;
	p->lineno = lineno;	
	p->next = first;
	p->prev = NULL;
	p->guard = 0x12345678;
	
	BYTE *bp = (BYTE *)(p+1);
	*(bp + size)= 0xcc;
	
	if (first)
		first->prev = p;
	first = p;
	amountalloc += size;
	objectcount++;
	return (p+1);
	}

void operator delete(void *_p)
	{
	if (!_p)
		{
		char buffer[256];
			
		sprintf(buffer,"tried to deallocated %lx", (long)_p);
		ErrorExit(buffer);
		}
	HEADER *p = ((HEADER *)_p)-1;
	
	if (p->guard != 0x12345678)
		{
		ErrorExit("Start guard byte changed!");
		}
	
	BYTE *bp = (BYTE*)_p + p->size;
	if (*bp != 0xcc )
		{
		ErrorExit("End Guard byte changed!");
		}
		
	*bp = 0;
	p->guard = 0;
	
	if (p->next)
		p->next->prev = p->prev;
	if (p->prev)
		p->prev->next = p->next;
	else
		first = p->next;
	
	amountalloc -= p->size;
	objectcount--;
	DisposPtr( (Ptr)p);
	}
#else

void *operator new(size_t size)
	{
	void *p = NewPtr(size);
	while(!p)
		{
		if (!FreeSomeMemory())
			{
			ErrorExit("new: couldn't allocate enough memory to continue");
			}
		p = NewPtr(size);
		}
		
	return p;
	}
	
void operator delete(void *p)
	{
	DisposPtr( (Ptr)p);
	}

#endif

static inline void ConstructElement(CString *pNewData)
	{
	memcpy(pNewData, &afxEmptyString, sizeof(CString));
	}

static void ConstructElements(CString* pNewData, int nCount)
{
	ASSERT(nCount >= 0);

	while (nCount--)
	{
		ConstructElement(pNewData);
		pNewData++;
	}
}

static void DestructElements(CString* pOldData, int nCount)
{
	ASSERT(nCount >= 0);

	while (nCount--)
	{
		pOldData->Empty();
		pOldData++;
	}
}

/////////////////////////////////////////////////////////////////////////////

CStringArray::CStringArray()
{
	m_pData = NULL;
	m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

CStringArray::~CStringArray()
{
	ASSERT_VALID(this);


	DestructElements(m_pData, m_nSize);
	if (m_pData)
		delete[] (BYTE*)m_pData;
}

void CStringArray::SetSize(int nNewSize, int nGrowBy)
{
	ASSERT_VALID(this);
	ASSERT(nNewSize >= 0);

	if (nGrowBy != -1)
		m_nGrowBy = nGrowBy;  // set new size

	if (nNewSize == 0)
	{
		// shrink to nothing

		DestructElements(m_pData, m_nSize);
		if (m_pData)
			delete[] (BYTE*)m_pData;
		m_pData = NULL;
		m_nSize = m_nMaxSize = 0;
	}
	else if (m_pData == NULL)
	{
		// create one with exact size
#ifdef SIZE_T_MAX
		ASSERT(nNewSize <= SIZE_T_MAX/sizeof(CString));    // no overflow
#endif
		m_pData = (CString*) new BYTE[nNewSize * sizeof(CString)];

		ConstructElements(m_pData, nNewSize);

		m_nSize = m_nMaxSize = nNewSize;
	}
	else if (nNewSize <= m_nMaxSize)
	{
		// it fits
		if (nNewSize > m_nSize)
		{
			// initialize the new elements

			ConstructElements(&m_pData[m_nSize], nNewSize-m_nSize);

		}

		else if (m_nSize > nNewSize)  // destroy the old elements
			DestructElements(&m_pData[nNewSize], m_nSize-nNewSize);

		m_nSize = nNewSize;
	}
	else
	{
		// otherwise, grow array
		int nGrowBy = m_nGrowBy;
		if (nGrowBy == 0)
		{
			// heuristically determine growth when nGrowBy == 0
			//  (this avoids heap fragmentation in many situations)
			nGrowBy = min(1024, max(4, m_nSize / 8));
		}
		int nNewMax;
		if (nNewSize < m_nMaxSize + nGrowBy)
			nNewMax = m_nMaxSize + nGrowBy;  // granularity
		else
			nNewMax = nNewSize;  // no slush

		ASSERT(nNewMax >= m_nMaxSize);  // no wrap around
#ifdef SIZE_T_MAX
		ASSERT(nNewMax <= SIZE_T_MAX/sizeof(CString)); // no overflow
#endif
		CString* pNewData = (CString*) new BYTE[nNewMax * sizeof(CString)];

		// copy new data from old
		memcpy(pNewData, m_pData, m_nSize * sizeof(CString));

		// construct remaining elements
		ASSERT(nNewSize > m_nSize);

		ConstructElements(&pNewData[m_nSize], nNewSize-m_nSize);


		// get rid of old stuff (note: no destructors called)
		if (m_pData)
			delete[] (BYTE*)m_pData;
		m_pData = pNewData;
		m_nSize = nNewSize;
		m_nMaxSize = nNewMax;
	}
}

void CStringArray::FreeExtra()
{
	ASSERT_VALID(this);

	if (m_nSize != m_nMaxSize)
	{
		// shrink to desired size
#ifdef SIZE_T_MAX
		ASSERT(m_nSize <= SIZE_T_MAX/sizeof(CString)); // no overflow
#endif
		CString* pNewData = NULL;
		if (m_nSize != 0)
		{
			pNewData = (CString*) new BYTE[m_nSize * sizeof(CString)];
			// copy new data from old
			memcpy(pNewData, m_pData, m_nSize * sizeof(CString));
		}

		// get rid of old stuff (note: no destructors called)
		if (m_pData)
			delete[] (BYTE*)m_pData;
		m_pData = pNewData;
		m_nMaxSize = m_nSize;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CStringArray::SetAtGrow(int nIndex, LPCSTR newElement)
{
	ASSERT_VALID(this);
	ASSERT(nIndex >= 0);

	if (nIndex >= m_nSize)
		SetSize(nIndex+1);
	m_pData[nIndex] = newElement;
}

void CStringArray::InsertAt(int nIndex, LPCSTR newElement, int nCount)
{
	ASSERT_VALID(this);
	ASSERT(nIndex >= 0);    // will expand to meet need
	ASSERT(nCount > 0);     // zero or negative size not allowed

	if (nIndex >= m_nSize)
	{
		// adding after the end of the array
		SetSize(nIndex + nCount);  // grow so nIndex is valid
	}
	else
	{
		// inserting in the middle of the array
		int nOldSize = m_nSize;
		SetSize(m_nSize + nCount);  // grow it to new size
		// shift old data up to fill gap
		memmove(&m_pData[nIndex+nCount], &m_pData[nIndex],
			(nOldSize-nIndex) * sizeof(CString));

		// re-init slots we copied from

		ConstructElements(&m_pData[nIndex], nCount);

	}

	// insert new value in the gap
	ASSERT(nIndex + nCount <= m_nSize);
	while (nCount--)
		m_pData[nIndex++] = newElement;
}

void CStringArray::RemoveAt(int nIndex, int nCount)
{
	ASSERT_VALID(this);
	ASSERT(nIndex >= 0);
	ASSERT(nCount >= 0);
	ASSERT(nIndex + nCount <= m_nSize);

	// just remove a range
	int nMoveCount = m_nSize - (nIndex + nCount);

	DestructElements(&m_pData[nIndex], nCount);

	if (nMoveCount)
		memcpy(&m_pData[nIndex], &m_pData[nIndex + nCount],
			nMoveCount * sizeof(CString));
	m_nSize -= nCount;
}

void CStringArray::InsertAt(int nStartIndex, CStringArray* pNewArray)
{
	ASSERT_VALID(this);
	ASSERT(pNewArray != NULL);
	ASSERT_VALID(pNewArray);
	ASSERT(nStartIndex >= 0);

	if (pNewArray->GetSize() > 0)
	{
		InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
		for (int i = 0; i < pNewArray->GetSize(); i++)
			SetAt(nStartIndex + i, pNewArray->GetAt(i));
	}
}


/////////////////////////////////////////////////////////////////////////////
// Diagnostics

#ifndef NDEBUG

void CStringArray::AssertValid() const
{
	
	if (m_pData == NULL)
	{
		ASSERT(m_nSize == 0);
		ASSERT(m_nMaxSize == 0);
	}
	else
	{
		ASSERT(m_nSize >= 0);
		ASSERT(m_nMaxSize >= 0);
		ASSERT(m_nSize <= m_nMaxSize);
	}
}
#endif

MEM_HANDLE xHeapAlloc(long amount)
	{
	Handle h = NewHandle(amount);
	while (!h)
		{
		if (!FreeSomeMemory())
			ErrorExit("HeapAlloc out of memory");
			
		h = NewHandle(amount);	
		}
	return h;
	}

void * xHeapLock( MEM_HANDLE handle )
	{
	HLock(handle);
	ASSERT(*handle);
	return *handle;
	}
	
void xHeapUnlock( MEM_HANDLE handle )
	{
	HUnlock(handle);
	}
	
void xHeapFree( MEM_HANDLE handle)
	{
	DisposHandle(handle);
	}
	
void xHeapSetDiscardable( MEM_HANDLE handle, BOOL flag )
	{
	if (flag)
		HPurge(handle);
	else
		HNoPurge(handle);
	}
	
BOOL xHeapIsDiscarded( MEM_HANDLE handle )
	{
	return *(handle) == 0;
	}

void xHeapRealloc( MEM_HANDLE *handle, long newamount)
	{
	SetHandleSize(*handle, newamount);
	if (MemError() )
		ErrorExit("HeapRealloc out of memory");
	}


static char * exception_strings[] =
	{ 
	"File could not be found",
	"Could not open file",
	"Could not read the contents of file",
	"The file contains invalid information"
	};
	
void CViewException::FormatErrorString( char *buffer )
	{
	sprintf(buffer, "%s: %s", exception_strings[m_error_code], m_context);
	}


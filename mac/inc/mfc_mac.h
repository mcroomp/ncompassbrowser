#include <string.h>

#ifdef __POWERPC__
#define NDEBUG
#else
#define _DEBUG
#endif

typedef unsigned short USHORT;
typedef unsigned long ULONG;

typedef int BOOL;
typedef unsigned char BYTE;

typedef char *LPSTR;
typedef const char * LPCSTR;

typedef void *POSITION;

#ifndef NULL
#define NULL 0
#endif

void strrev(char *str);
void strupr(char *str);
void strlwr(char *str);
BOOL stricmp(const char *str1, const char *str2);

#include "vexcept.h"

class CRect
	{
public:
	CRect();
	CRect(const CRect& r);
	CRect(long _left, long _top, long _right, long _bottom);
	CRect& operator	= (const CRect& r);
	
	void SetRect(long _left, long _top, long _right, long _bottom);
	
	long left, top, right, bottom;
	};

#define ERROREXIT(str) ErrorExit(str)
void ErrorExit(const char *string);
void ShowError(const char *string);

void *operator new(size_t size);
void operator delete(void *p);

#ifndef NDEBUG

#define THIS_FILE __FILE__

void	DumpMemory();

void *operator new(size_t size);
void *operator new(size_t size, LPCSTR file, int lineno );
void operator delete(void *p);

#define DEBUG_NEW new(THIS_FILE, __LINE__)

int AssertFail(const char *filename, int line);
#define ASSERT(cond) ( (cond) ? 1 : AssertFail(THIS_FILE,__LINE__) )
#define ASSERT_VALID(ob) ( ASSERT(ob), ob->AssertValid() )
#define VERIFY(cond) ( (cond) ? 1 : AssertFail(THIS_FILE,__LINE__) )
#else

#define DEBUG_NEW new
#define ASSERT(cond)
#define VERIFY(cond) (cond)
#define ASSERT_VALID(ob)
#endif

struct CPlex    // warning variable length structure
{
	CPlex* pNext;
	long nMax;
	long nCur;
	/* BYTE data[maxNum*elementSize]; */

	void* data() { return this+1; }

	static CPlex* Create(CPlex*& head, long nMax, long cbElement);
			// like 'calloc' but no zero fill
			// may throw memory exceptions

	void FreeDataChain();       // free this one and links
};

class CPtrList
	{
protected:
	struct CNode
		{
		CNode* pNext;
		CNode* pPrev;
		void* data;
		};
public:

// Construction
	CPtrList(long nBlockSize = 10);

// Attributes (head and tail)
	// count of elements
	long GetCount() const;
	BOOL IsEmpty() const;

	// peek at head or tail
	void*& GetHead();
	void* GetHead() const;
	void*& GetTail();
	void* GetTail() const;

// Operations
	// get head or tail (and remove it) - don't call on empty list!
	void* RemoveHead();
	void* RemoveTail();

	// add before head or after tail
	POSITION AddHead(void* newElement);
	POSITION AddTail(void* newElement);

	// add another list of elements before head or after tail
	void AddHead(CPtrList* pNewList);
	void AddTail(CPtrList* pNewList);

	// remove all elements
	void RemoveAll();

	// iteration
	POSITION GetHeadPosition() const;
	POSITION GetTailPosition() const;
	void*& GetNext(POSITION& rPosition); // return *Position++
	void* GetNext(POSITION& rPosition) const; // return *Position++
	void*& GetPrev(POSITION& rPosition); // return *Position--
	void* GetPrev(POSITION& rPosition) const; // return *Position--

	// getting/modifying an element at a given position
	void*& GetAt(POSITION position);
	void* GetAt(POSITION position) const;
	void SetAt(POSITION pos, void* newElement);
	void RemoveAt(POSITION position);

	// inserting before or after a given position
	POSITION InsertBefore(POSITION position, void* newElement);
	POSITION InsertAfter(POSITION position, void* newElement);

	// helper functions (note: O(n) speed)
	POSITION Find(void* searchValue, POSITION startAfter = NULL) const;
						// defaults to starting at the HEAD
						// return NULL if not found
	POSITION FindIndex(long nIndex) const;
						// get the 'nIndex'th element (may return NULL)

// Implementation
protected:
	CNode* m_pNodeHead;
	CNode* m_pNodeTail;
	long m_nCount;
	CNode* m_pNodeFree;
	struct CPlex* m_pBlocks;
	long m_nBlockSize;

	CNode* NewNode(CNode*, CNode*);
	void FreeNode(CNode*);

public:
	~CPtrList();
	};

inline long CPtrList::GetCount() const
	{ return m_nCount; }
inline BOOL CPtrList::IsEmpty() const
	{ return m_nCount == 0; }
inline void*& CPtrList::GetHead()
	{ ASSERT(m_pNodeHead != NULL);
		return m_pNodeHead->data; }
inline void* CPtrList::GetHead() const
	{ ASSERT(m_pNodeHead != NULL);
		return m_pNodeHead->data; }
inline void*& CPtrList::GetTail()
	{ ASSERT(m_pNodeTail != NULL);
		return m_pNodeTail->data; }
inline void* CPtrList::GetTail() const
	{ ASSERT(m_pNodeTail != NULL);
		return m_pNodeTail->data; }
inline POSITION CPtrList::GetHeadPosition() const
	{ return (POSITION) m_pNodeHead; }
inline POSITION CPtrList::GetTailPosition() const
	{ return (POSITION) m_pNodeTail; }
inline void*& CPtrList::GetNext(POSITION& rPosition) // return *Position++
	{ CNode* pNode = (CNode*) rPosition;
		rPosition = (POSITION) pNode->pNext;
		return pNode->data; }
inline void* CPtrList::GetNext(POSITION& rPosition) const // return *Position++
	{ CNode* pNode = (CNode*) rPosition;
		rPosition = (POSITION) pNode->pNext;
		return pNode->data; }
inline void*& CPtrList::GetPrev(POSITION& rPosition) // return *Position--
	{ CNode* pNode = (CNode*) rPosition;
		rPosition = (POSITION) pNode->pPrev;
		return pNode->data; }
inline void* CPtrList::GetPrev(POSITION& rPosition) const // return *Position--
	{ CNode* pNode = (CNode*) rPosition;
		rPosition = (POSITION) pNode->pPrev;
		return pNode->data; }
inline void*& CPtrList::GetAt(POSITION position)
	{ CNode* pNode = (CNode*) position;
		return pNode->data; }
inline void* CPtrList::GetAt(POSITION position) const
	{ CNode* pNode = (CNode*) position;
		return pNode->data; }
inline void CPtrList::SetAt(POSITION pos, void* newElement)
	{ CNode* pNode = (CNode*) pos;
		pNode->data = newElement; }


class CString
{
public:

// Constructors
	CString();
	CString(const CString& stringSrc);
	CString(char ch, long nRepeat = 1);
	CString(LPCSTR lpsz);
	CString(LPCSTR lpch, long nLength);
	
// Attributes & Operations
	// as an array of characters
	CString(const unsigned char* lpsz);
		
	const CString& operator=(const unsigned char* lpsz);
	long GetLength() const;
	long GetAllocLength() const;
	BOOL IsEmpty() const;
	operator LPCSTR() const;
	long SafeStrlen(LPCSTR lpsz);
 	int Compare(LPCSTR lpsz) const;
	int CompareNoCase(LPCSTR lpsz) const;
	
	void MakeUpper();
	void MakeLower();
	
	void MakeReverse();
	
	void SetAt(long nIndex, char ch);
	void Empty();                       // free up the data

	char GetAt(long nIndex) const;      // 0 based
	char operator[](long nIndex) const; // same as GetAt

	// overloaded assignment
	const CString& operator=(const CString& stringSrc);
	const CString& operator=(char ch);
	const CString& operator=(LPCSTR lpsz);

	// string concatenation
	const CString& operator+=(const CString& string);
	const CString& operator+=(char ch);
	const CString& operator+=(LPCSTR lpsz);

	friend CString  operator+(const CString& string1,
			const CString& string2);
	friend CString  operator+(const CString& string, char ch);
	friend CString  operator+(char ch, const CString& string);
	
	friend CString  operator+(const CString& string, LPCSTR lpsz);
	friend CString  operator+(LPCSTR lpsz, const CString& string);

	void GetPString( Str255 s ) const;
	
	// simple sub-string extraction
	CString Mid(long nFirst, long nCount) const;
	CString Mid(long nFirst) const;
	CString Left(long nCount) const;
	CString Right(long nCount) const;

	CString SpanIncluding(LPCSTR lpszCharSet) const;
	CString SpanExcluding(LPCSTR lpszCharSet) const;

	// searching (return starting index, or -1 if not found)
	// look for a single character match
	long Find(char ch) const;               // like "C" strchr
	long ReverseFind(char ch) const;
	long FindOneOf(LPCSTR lpszCharSet) const;

	// look for a specific sub-string
	long Find(LPCSTR lpszSub) const;        // like "C" strstr

	// simple formatting
	void Format(LPCSTR lpszFormat, ...);
	
	// Windows support
	BOOL LoadString(unsigned long nID);          // load from string resource
										// 255 chars max
	// Access to string implementation buffer as "C" character array
	LPSTR GetBuffer(long nMinBufLength);
	void ReleaseBuffer(long nNewLength = -1);
	LPSTR GetBufferSetLength(long nNewLength);
	void FreeExtra();

// Implementation
public:
	~CString();

protected:
	// lengths/sizes in characters
	//  (note: an extra character is always allocated)
	LPSTR m_pchData;           // actual string (zero terminated)
	long m_nDataLength;          // does not include terminating 0
	long m_nAllocLength;         // does not include terminating 0

	// implementation helpers
	void Init();
	void AllocCopy(CString& dest, long nCopyLen, long nCopyIndex, long nExtraLen) const;
	void AllocBuffer(long nLen);
	void AssignCopy(long nSrcLen, LPCSTR lpszSrcData);
	void ConcatCopy(long nSrc1Len, LPCSTR lpszSrc1Data, long nSrc2Len, LPCSTR lpszSrc2Data);
	void ConcatInPlace(long nSrcLen, LPCSTR lpszSrcData);
	static void SafeDelete(LPSTR lpch);
};

inline CString::CString(const unsigned char* lpsz)
		{ Init(); *this = (LPCSTR)lpsz; }
	
inline  const CString& CString::operator=(const unsigned char* lpsz)
	{ *this = (LPCSTR)lpsz; return *this; }

inline  long CString::GetLength() const
	{ return m_nDataLength; }
inline  long CString::GetAllocLength() const
	{ return m_nAllocLength; }
inline  BOOL CString::IsEmpty() const
	{ return m_nDataLength == 0; }
inline  CString::operator LPCSTR() const
	{ return (LPCSTR)m_pchData; }
inline  long CString::SafeStrlen(LPCSTR lpsz)
	{ return (lpsz == NULL) ? NULL : strlen(lpsz); }

inline  int CString::Compare(LPCSTR lpsz) const
	{ return strcmp(m_pchData, lpsz); }  
inline  int CString::CompareNoCase(LPCSTR lpsz) const
	{ return stricmp(m_pchData, lpsz); }   

inline  void CString::MakeUpper()
	{ strupr(m_pchData); }
inline  void CString::MakeLower()
	{ strlwr(m_pchData); }

inline  void CString::MakeReverse()
	{ strrev(m_pchData); }

inline  char CString::GetAt(long nIndex) const
	{
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < m_nDataLength);

	return m_pchData[nIndex];
	}
inline  char CString::operator[](long nIndex) const
	{
	// same as GetAt

	ASSERT(nIndex >= 0);
	ASSERT(nIndex < m_nDataLength);

	return m_pchData[nIndex];
	}
	
inline  void CString::SetAt(long nIndex, char ch)
	{
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < m_nDataLength);	
	ASSERT(ch != 0);

	m_pchData[nIndex] = ch;
	}

inline  BOOL  operator==(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) == 0; }
inline  BOOL  operator==(const CString& s1, LPCSTR s2)
	{ return s1.Compare(s2) == 0; }
inline  BOOL  operator==(LPCSTR s1, const CString& s2)
	{ return s2.Compare(s1) == 0; }
inline  BOOL  operator!=(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) != 0; }
inline  BOOL  operator!=(const CString& s1, LPCSTR s2)
	{ return s1.Compare(s2) != 0; }
inline  BOOL  operator!=(LPCSTR s1, const CString& s2)
	{ return s2.Compare(s1) != 0; }
inline  BOOL  operator<(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) < 0; }
inline  BOOL  operator<(const CString& s1, LPCSTR s2)
	{ return s1.Compare(s2) < 0; }
inline  BOOL  operator<(LPCSTR s1, const CString& s2)
	{ return s2.Compare(s1) > 0; }
inline  BOOL  operator>(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) > 0; }
inline  BOOL  operator>(const CString& s1, LPCSTR s2)
	{ return s1.Compare(s2) > 0; }
inline  BOOL  operator>(LPCSTR s1, const CString& s2)
	{ return s2.Compare(s1) < 0; }
inline  BOOL  operator<=(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) <= 0; }
inline  BOOL  operator<=(const CString& s1, LPCSTR s2)
	{ return s1.Compare(s2) <= 0; }
inline  BOOL  operator<=(LPCSTR s1, const CString& s2)
	{ return s2.Compare(s1) >= 0; }
inline  BOOL  operator>=(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) >= 0; }
inline  BOOL  operator>=(const CString& s1, LPCSTR s2)
	{ return s1.Compare(s2) >= 0; }
inline  BOOL  operator>=(LPCSTR s1, const CString& s2)
	{ return s2.Compare(s1) <= 0; }

// Globals
extern const CString afxEmptyString;
extern char afxChNil;

////////////////////////////////////////////////////////////////////////////

class CLongStack
{
public:

// Construction
	CLongStack();

// Attributes
	long GetSize() const;
	long GetUpperBound() const;
	void SetSize(long nNewSize, long nGrowBy = -1);

// Operations
	// Clean up
	void FreeExtra();
	void RemoveAll();

	// Accessing elements
	long GetAt(long nIndex) const;
	void SetAt(long nIndex, long newElement);
	long& ElementAt(long nIndex);

	// Potentially growing the array
	void SetAtGrow(long nIndex, long newElement);
	long Add(long newElement);

	// overloaded operator helpers
	long operator[](long nIndex) const;
	long& operator[](long nIndex);

	// Operations that move elements around
	void InsertAt(long nIndex, long newElement, long nCount = 1);
	void RemoveAt(long nIndex, long nCount = 1);
	void InsertAt(long nStartIndex, CLongStack* pNewArray);

	// Stack operations
	
	void Push(long value);
	long Pop();
	
// Implementation
protected:
	long* m_pData;   // the actual array of data
	long m_nSize;     // # of elements (upperBound - 1)
	long m_nMaxSize;  // max allocated
	long m_nGrowBy;   // grow amount

public:
	~CLongStack();
};

inline long CLongStack::GetSize() const
	{ return m_nSize; }
inline long CLongStack::GetUpperBound() const
	{ return m_nSize-1; }
inline void CLongStack::RemoveAll()
	{ SetSize(0); }
inline long CLongStack::GetAt(long nIndex) const
	{ ASSERT(nIndex >= 0 && nIndex < m_nSize);
		return m_pData[nIndex]; }
inline void CLongStack::SetAt(long nIndex, long newElement)
	{ ASSERT(nIndex >= 0 && nIndex < m_nSize);
		m_pData[nIndex] = newElement; }
inline long& CLongStack::ElementAt(long nIndex)
	{ ASSERT(nIndex >= 0 && nIndex < m_nSize);
		return m_pData[nIndex]; }
inline long CLongStack::Add(long newElement)
	{ long nIndex = m_nSize;
		SetAtGrow(nIndex, newElement);
		return nIndex; }
inline long CLongStack::operator[](long nIndex) const
	{ return GetAt(nIndex); }
inline long& CLongStack::operator[](long nIndex)
	{ return ElementAt(nIndex); }

class CStringArray
{

public:

// Construction
	CStringArray();

// Attributes
	int GetSize() const;
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

// Operations
	// Clean up
	void FreeExtra();
	void RemoveAll();

	// Accessing elements
	CString GetAt(int nIndex) const;
	void SetAt(int nIndex, LPCSTR newElement);
	CString& ElementAt(int nIndex);

	// Potentially growing the array
	void SetAtGrow(int nIndex, LPCSTR newElement);
	int Add(LPCSTR newElement);

	// overloaded operator helpers
	CString operator[](int nIndex) const;
	CString& operator[](int nIndex);

	// Operations that move elements around
	void InsertAt(int nIndex, LPCSTR newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CStringArray* pNewArray);

// Implementation
protected:
	CString* m_pData;   // the actual array of data
	int m_nSize;     // # of elements (upperBound - 1)
	int m_nMaxSize;  // max allocated
	int m_nGrowBy;   // grow amount

public:
	~CStringArray();

#ifndef NDEBUG
	void AssertValid() const;
#endif
};

inline int CStringArray::GetSize() const
	{ return m_nSize; }
inline int CStringArray::GetUpperBound() const
	{ return m_nSize-1; }
inline void CStringArray::RemoveAll()
	{ SetSize(0); }
inline CString CStringArray::GetAt(int nIndex) const
	{ ASSERT(nIndex >= 0 && nIndex < m_nSize);
		return m_pData[nIndex]; }
inline void CStringArray::SetAt(int nIndex, LPCSTR newElement)
	{ ASSERT(nIndex >= 0 && nIndex < m_nSize);
		m_pData[nIndex] = newElement; }
inline CString& CStringArray::ElementAt(int nIndex)
	{ ASSERT(nIndex >= 0 && nIndex < m_nSize);
		return m_pData[nIndex]; }
inline int CStringArray::Add(LPCSTR newElement)
	{ int nIndex = m_nSize;
		SetAtGrow(nIndex, newElement);
		return nIndex; }
inline CString CStringArray::operator[](int nIndex) const
	{ return GetAt(nIndex); }
inline CString& CStringArray::operator[](int nIndex)
	{ return ElementAt(nIndex); }

typedef Handle MEM_HANDLE;

MEM_HANDLE 	xHeapAlloc(long amount);
void * 		xHeapLock( MEM_HANDLE handle );
void 		xHeapUnlock( MEM_HANDLE handle );
void 		xHeapFree( MEM_HANDLE );
void 		xHeapSetDiscardable( MEM_HANDLE handle, BOOL flag );
BOOL 		xHeapIsDiscarded( MEM_HANDLE handle );
void 		xHeapRealloc( MEM_HANDLE *handle, long newsize);

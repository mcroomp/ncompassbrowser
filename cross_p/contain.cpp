#include "cross_p.h"

#include "contain.h"

CContainerItem::~CContainerItem()
	{
	}
	
void *CContainerItem::operator new(size_t nSize, CContainer& store)
	{
	return store.Allocate(nSize);
	}
	
void CContainerItem::operator delete( void * p )
	{
	}

CContainer::CContainer()
	{
	m_allocated = 0;
	m_num_items = 0;
	
	m_first_chunk = m_current_chunk = NULL;;
	}

void CContainer::RemoveAll()
	{
	if (!m_first_chunk)
		return;			// nothing to do
	
	BYTE *p = (BYTE *)m_first_chunk;
	BYTE *current_chunk = p;
	
	while(1)
		{
		INT32 size = *(INT32 *)p;
		
		if (size == 0xffffffff)
			break;		// signals end
		else if (size == 0)
			{
			// go to next chunk
			p = *(BYTE **) (((INT32 *)p)+1);
			delete current_chunk;
			current_chunk = p;
			continue;		// try again with new chunk	
			}
		
		CContainerItem *pitem = (CContainerItem *) ( ((INT32 *)p)+1);
		delete pitem;
//		pitem->~CContainerItem(); 	// call destructor
		
		p += size;
		}
	delete current_chunk;
	
	m_allocated = 0;
	m_current_chunk = m_first_chunk = NULL;
	}

CContainer::~CContainer()
	{
	RemoveAll();
	}

POSITION CContainer::GetHeadPosition() const
	{
	if (m_first_chunk == NULL)
		return NULL;
	else
		return m_first_chunk;
	}
	
CContainerItem *CContainer::GetNext(POSITION& pos) const
	{
	INT32 cursize = *(INT32 *)pos;
	
	ASSERT(cursize != 0xffffffff && cursize != 0);
	
	void *retval = (( (INT32 *)pos) + 1);
	
	INT32 * curpos = (INT32 *)( ((BYTE *)pos) + cursize );
	if (*curpos == 0)
		{
		pos = *(void **)(curpos+1);
		}
	else if (*curpos == 0xffffffff)
		{
		pos = NULL;
		}
	else
		{
		pos = (POSITION)( ((BYTE *)pos) + cursize );
		}
		
	return (CContainerItem *)retval;
	}

POSITION CContainer::Find( CContainerItem *i) const
	{
	return (( (INT32 *)i) - 1);
	}
	
void *CContainer::Allocate(INT32 size)
	{
	ASSERT(size < CHUNK_SIZE);
	
	INT32 *place;
	
	if (!m_current_chunk)
		{
		m_current_chunk = m_first_chunk = DEBUG_NEW BYTE[ CHUNK_SIZE ];
		m_allocated = 0;
		}
		
	size += sizeof(INT32);
	size = RoundUp4(size);		// align everything within in 4 byte boundry
	
	place = (INT32 *) ( ((BYTE *)m_current_chunk) + m_allocated );
	
	if (m_allocated + size > CHUNK_SIZE-8)
		{
		// reserve 4 bytes for the next chunk pointer
		
		BYTE *bp = DEBUG_NEW BYTE[ CHUNK_SIZE ];
		*place = 0;
		place++;
		*(BYTE **)place = bp;
		
		m_allocated = 0;
		m_current_chunk = bp;
		place = (INT32 *)bp;
		}
		
	m_allocated += size;
	*place = size;
	
	// write sentinal at end of block
	*(INT32 *) ( ((BYTE *)m_current_chunk) + m_allocated ) = 0xffffffff;
	
	m_num_items++;

	return place+1; 
	}

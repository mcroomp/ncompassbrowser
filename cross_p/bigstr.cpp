#include "cross_p.h"

#include "bigstr.h"

#ifndef _MUTEX_H_
#include "mutex.h"
#endif

#define BLOCK_SIZE 4096
#define RIGHT_SHIFT 12

class BIGSTRING_RECORD
	{
public:
	INT32 m_ref_count;

	CPtrArray m_block_array;
	
	LPSTR m_current_write_pos;
	INT32 m_amount_left_on_block;
	INT32 m_length;

	CAccessLock m_access_lock;
	};
	

INT32 CBigString::GetLength() const
	{
	m_rec->m_access_lock.DEBUG_LOCK();
	INT32 length = m_rec->m_length;
	m_rec->m_access_lock.Unlock();
	return length; 
	}

CBigString::CBigString()
	{
	m_rec = DEBUG_NEW BIGSTRING_RECORD;
	m_rec->m_ref_count = 1;
	m_rec->m_length = 0;
	m_rec->m_amount_left_on_block = 0;
	m_rec->m_current_write_pos = NULL;
	}

void CBigString::GetData( LPVOID store, INT32 textofs, INT32 textlen) const
	{
	LPBYTE buffer = (LPBYTE)store;

	m_rec->m_access_lock.DEBUG_LOCK();

	INT32 page = (textofs >> RIGHT_SHIFT), pageofs = (textofs & (BLOCK_SIZE-1));

	if (pageofs + textlen <= BLOCK_SIZE)
		{
		/* handle most common case first, block is entirely contained within a BLOCK_SIZE boundary */
		memcpy(buffer, ((LPCSTR)m_rec->m_block_array[page]) + pageofs, textlen);
		}
	else
		{
		/* the stuff we are looking for spans a block */
		INT32 amount_to_copy = BLOCK_SIZE - pageofs;
		INT32 amount_left = textlen;
	
		memcpy(buffer, ((LPCSTR)m_rec->m_block_array[page]) + pageofs, amount_to_copy);
		buffer += amount_to_copy;
		amount_left -= amount_to_copy;
		page++;

		while(amount_left > BLOCK_SIZE)
			{
			memcpy(buffer, m_rec->m_block_array[page], BLOCK_SIZE);
			buffer += BLOCK_SIZE;
			amount_left -= BLOCK_SIZE;
			page++;
			}

		if (amount_left > 0)
			{
			// copy remainder to buffer
			memcpy(buffer, ((LPCSTR)m_rec->m_block_array[page]), amount_left );
			}
		}

	m_rec->m_access_lock.Unlock();
	}


void CBigString::GetString(CString& store, INT32 textofs, INT32 textlen ) const
	{
	LPSTR buffer = store.GetBufferSetLength(textlen+1);
	GetData(buffer, textofs, textlen );
	store.ReleaseBuffer(textlen);
	}

CBigString::~CBigString()
	{
	m_rec->m_access_lock.DEBUG_LOCK();

	if (m_rec->m_ref_count == 1)
		{
		// free everything
		INT32 i, ilen = m_rec->m_block_array.GetSize();
		for(i=0;i<ilen;i++)
			{
			delete [] (LPSTR)m_rec->m_block_array[i];
			}
		delete m_rec;
		// no need to unlock the mutex since this object should not get accessed again anyhow
		}
	else
		{
		m_rec->m_ref_count--;
		m_rec->m_access_lock.Unlock();
		}
	}
		
CBigString::CBigString( const CBigString& str)
	{
	m_rec = str.m_rec;
	
	m_rec->m_access_lock.DEBUG_LOCK();
	m_rec->m_ref_count++;
	m_rec->m_access_lock.Unlock();
	}
	
CBigString& CBigString::operator = (const CBigString& str)
	{
	m_rec->m_access_lock.DEBUG_LOCK();

	if (m_rec->m_ref_count == 1)
		{
		// free everything
		INT32 i, ilen = m_rec->m_block_array.GetSize();
		for(i=0;i<ilen;i++)
			{
			delete [] (LPSTR)m_rec->m_block_array[i];
			}
		delete m_rec;
		// no need to unlock the mutex since this object should not get accessed again anyhow
		}
	else
		{
		m_rec->m_ref_count--;
		m_rec->m_access_lock.Unlock();
		}

	m_rec = str.m_rec;
	m_rec->m_access_lock.DEBUG_LOCK();
	m_rec->m_ref_count++;
	m_rec->m_access_lock.Unlock();

	return *this;
	}
	
void CBigString::AppendString(const char* string)
	{
	INT32 l = strlen(string);
	AppendString(string, l);
	}
	

void CBigString::AppendChar(char c)
	{
	ASSERT( m_rec );

	m_rec->m_access_lock.DEBUG_LOCK();

	if (m_rec->m_amount_left_on_block == 0 )
		{
		m_rec->m_current_write_pos = new CHAR[ BLOCK_SIZE ] ;
		m_rec->m_block_array.Add( m_rec->m_current_write_pos );
		m_rec->m_amount_left_on_block = BLOCK_SIZE;
		}

	m_rec->m_amount_left_on_block--;
	*((m_rec->m_current_write_pos)++) = c;
	m_rec->m_length++;

	m_rec->m_access_lock.Unlock();
	}

void CBigString::AppendString(const char* string, INT32 length) 
	{
	ASSERT( m_rec );
	m_rec->m_access_lock.DEBUG_LOCK();

	while(length > 0)
		{
		if (m_rec->m_amount_left_on_block >= length )
			{
			// most common case... string fits onto current block
			memcpy(m_rec->m_current_write_pos, string, length);

			m_rec->m_current_write_pos += length;
			m_rec->m_amount_left_on_block -= length;
			m_rec->m_length += length;
			length = 0;
			}
		else
			{
			if (m_rec->m_amount_left_on_block > 0)
				{
				memcpy(m_rec->m_current_write_pos, string, m_rec->m_amount_left_on_block);
				m_rec->m_length += m_rec->m_amount_left_on_block;
				length -= m_rec->m_amount_left_on_block;
				string += m_rec->m_amount_left_on_block;
				}
			
			m_rec->m_current_write_pos = new CHAR[ BLOCK_SIZE ] ;
			m_rec->m_block_array.Add( m_rec->m_current_write_pos );
			m_rec->m_amount_left_on_block = BLOCK_SIZE;
			}
		}

	m_rec->m_access_lock.Unlock();

	}
	
void CBigString::AppendData( LPCVOID source, INT32 length)
	{
	AppendString( (LPCSTR)source, length);
	}

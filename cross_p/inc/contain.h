#define _CONTAIN_H_

class CContainer;

class CContainerItem
	{
private:

static void *operator new(size_t nSize);

public:
	virtual ~CContainerItem();
	
static void *operator new(size_t nSize, CContainer& store);

protected:
	void operator delete( void *p );

	friend class CContainer;
	};

#define CHUNK_SIZE 4096

class CContainer
	{
	void *m_first_chunk;
	void *m_current_chunk;
	
	INT32 m_allocated;
	INT32 m_num_items;

	// disallow the use of the following operators
	CContainer( const CContainer& c);
	CContainer& operator =(const CContainer& c);
	
	void *Allocate(INT32 amount);
public:
	POSITION GetHeadPosition() const;
	CContainerItem *GetNext(POSITION& pos) const;
	POSITION Find( CContainerItem *item) const;

	INT32 GetCount() const
		{ return m_num_items; }

	void RemoveAll();

	CContainer();
	~CContainer();

	friend class CContainerItem;
	};

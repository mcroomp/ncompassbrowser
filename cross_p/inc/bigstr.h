#define _BIGSTR_H_

class BIGSTRING_RECORD;

// Thread safe string class which uses reference counting and non-realloc approach to
// growing the string.

class CBigString
	{
public:
	CBigString();
	~CBigString();
	
	CBigString( const CBigString& str);
	CBigString& operator = (const CBigString& str);
	
	void AppendChar(char c);
	void AppendString(const char* string);
	void AppendString(const char* string, INT32 length);

	INT32 GetLength() const;
	void GetString( CString& store, INT32 startpos, INT32 length) const;
	

// If CBigString is just binary data, use these functions
	void AppendData( LPCVOID source, INT32 length);
	void GetData( LPVOID store, INT32 startpos, INT32 length) const;
		
private:
	BIGSTRING_RECORD *m_rec;
	};
	

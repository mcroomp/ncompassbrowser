#define _VEXCEPT_H_

#ifdef xxx
class CViewException
	{
public:
	typedef enum 
		{
		file_not_exist_error = 0,
		file_open_error,
		file_read_error,
		file_invalid_format
		
		} ERROR_CODE;
		
	CViewException( ERROR_CODE error_code )
		{ m_error_code = error_code; m_context[0] = 0; }
	CViewException( ERROR_CODE error_code, const char *context)
		{ m_error_code = error_code; strncpy( m_context, context, sizeof(m_context) ); }

	ERROR_CODE GetErrorCode() const
		{ return m_error_code; }
		
	virtual	void FormatErrorString( char *buffer );
	
private:
	ERROR_CODE m_error_code;
	char m_context[256];
	};

#endif

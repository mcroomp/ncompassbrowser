// inc\smtp.h : header file
//

#define _SMTP_H_

#ifndef _MIMELOAD_H_
#include "mimeload.h"
#endif

#ifndef _PROTOCOL_H_
#include "protocol.h"
#endif


class CProtocolSMTP : public CProtocol
	{
public:
	CProtocolSMTP(CDynamicLoad *load_object);
	~CProtocolSMTP();

	virtual void BeginLoadThread();
	virtual void AbortLoadThread();
	virtual void WaitEndLoadThread();

static	void ShowDialog(LPCSTR url);

private:
	CThreadSocket m_socket;
	HANDLE m_thread_done_semaphore;

	friend UINT SMTPWorkerThread( LPVOID lparam );
	};


// this object does absolutely nothing. It is simply a placeholder to allow the SMTP protocol do to
// its work within the defined protocol mechanism


// this object does nothing except show the Send dialog in the Launch Viewer function
class CMimeSMTP : public CMimeObject
	{
public:
	CMimeSMTP(LPCSTR url, LPCSTR mime_type);

protected:
// overrides
	virtual LOAD_STATE OnReadData(LPCBYTE buffer, INT32 buffer_size);
	virtual LOAD_STATE OnEndOfFile();

	virtual void LaunchViewer();
	virtual BOOL UsesInternalViewer() const;
	};

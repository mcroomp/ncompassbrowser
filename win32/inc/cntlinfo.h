///////////////////////////////////////////////////////////////////////////
// CNTLINFO.H -- Information structures for CControlItem

#ifndef __CNTLINFO_H__
#define __CNTLINFO_H__

///////////////////////////////////////////////////////////////////////////
// definitions

#define IMPLTYPE_MASK \
    (IMPLTYPEFLAG_FDEFAULT | IMPLTYPEFLAG_FSOURCE | IMPLTYPEFLAG_FRESTRICTED)

#define IMPLTYPE_DEFAULTSOURCE \
    (IMPLTYPEFLAG_FDEFAULT | IMPLTYPEFLAG_FSOURCE)
              
              
///////////////////////////////////////////////////////////////////////////
// structures

///////////////////////////////////////////////////////////////////////////
// EVENTINFO -- OLE Control Event Information Structure

struct EVENTINFO
{
    MEMBERID memid;			// ID of the event
    SHORT cParams;			// Number of pararameters in event
    BSTR* pbstr;			// Name of the event
};

///////////////////////////////////////////////////////////////////////////
// PARAMPROPINFO -- OLE Control Data Binding Parameter Information Structure

struct PARAMPROPINFO
{
    ~PARAMPROPINFO() {}
    MEMBERID id;
    CString strName;
};

typedef PARAMPROPINFO FAR*	LPPARAMPROPINFO;


///////////////////////////////////////////////////////////////////////////
// BINDINFO -- OLE Control Data Binding Information Structure

struct BINDINFO
{
    BINDINFO() { m_nParamCount = 0; m_lpParamProps = NULL;
    			 m_nFuncCount = 0; m_lpFuncProps = NULL; }
    ~BINDINFO();
    UINT m_nParamCount;
    PARAMPROPINFO* m_lpParamProps;
	UINT m_nFuncCount;
	PARAMPROPINFO* m_lpFuncProps;
};


///////////////////////////////////////////////////////////////////////////
// APROP -- OLE Control Ambient Properties Information Structure

struct APROP {
    DISPID              dispid;
    CString             strName;
    VARIANT             varValue;
    UINT                idsTypeInterp;
};

typedef APROP FAR* LPAPROP;

///////////////////////////////////////////////////////////////////////////

// Event Types supported by this container

struct EVENTTYPE
{
	CString		eventName;
};

struct OCXINFO 
{
	CString		m_browserVersion,
				m_companyName,
				m_fileDescription,
				m_fileVersion,
				m_internalName,
				m_legalCopyright,
				m_legalTrademarks,
				m_originalFilename,
				m_productName,
				m_productVersion,
				m_oleSelfRegister;
};

#endif // __CNTLINFO_H__

///////////////////////////////////////////////////////////////////////////

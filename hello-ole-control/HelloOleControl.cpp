// HelloOleControl.cpp
//
// A tiny, purpose-built ActiveX/OLE control used to demonstrate the
// historical Ncompass browser's OLE-control hosting feature (the
// <xolecontrol> tag). It paints a static message and nothing else.
//
// This is modern demo tooling, NOT part of the 1996 browser source.
// It exists so the browser's OLE container code can be exercised
// end-to-end without depending on long-vanished CaptiveX controls or
// fighting version/bitness mismatches with today's system OCXes.
//
// Registration is done by hand (plain Win32 registry calls) instead of
// ATL's usual .rgs/type-library machinery, so the on-disk registration
// footprint is exactly what CControlItem::CheckIfLocalOCX() (win32/cntlitem.cpp)
// looks for: a CLSID key with a "Control" subkey and an InprocServer32 path.

#include <atlbase.h>
#include <atlcom.h>
#include <atlwin.h>
#include <atlctl.h>
#include <cstdio>

using namespace ATL;

// {3F2600F1-CF01-4618-BAC4-9A8AC30B8402}
const CLSID CLSID_HelloOleControl =
{ 0x3f2600f1, 0xcf01, 0x4618, { 0xba, 0xc4, 0x9a, 0x8a, 0xc3, 0x0b, 0x84, 0x02 } };

static const wchar_t* const kFriendlyName = L"Hello OLE Control";

class CHelloOleControlModule : public CAtlDllModuleT<CHelloOleControlModule>
{
public:
	DECLARE_LIBID(CLSID_NULL)
};

CHelloOleControlModule _AtlModule;

// class CHelloControl
//
// Minimal windowed ActiveX control: IOleObject + IPersistStorage (so the
// browser's existing storage-based load path in CControlItem works
// unmodified) + in-place activation/drawing support. No properties, no
// automation (IDispatch), no property pages -- just enough surface area
// for a container to create it, show it, and let it paint.

class ATL_NO_VTABLE CHelloControl :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CHelloControl, &CLSID_HelloOleControl>,
	public CComControl<CHelloControl>,
	public IOleObjectImpl<CHelloControl>,
	public IOleControlImpl<CHelloControl>,
	public IOleInPlaceActiveObjectImpl<CHelloControl>,
	public IViewObjectExImpl<CHelloControl>,
	public IOleInPlaceObjectWindowlessImpl<CHelloControl>,
	public IDataObjectImpl<CHelloControl>,
	public IPersistStreamInitImpl<CHelloControl>,
	public IPersistStorageImpl<CHelloControl>
{
public:
	CHelloControl()
	{
	}

	DECLARE_NO_REGISTRY()
	DECLARE_NOT_AGGREGATABLE(CHelloControl)
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_PROPERTY_MAP(CHelloControl)
	END_PROPERTY_MAP()

	DECLARE_OLEMISC_STATUS(OLEMISC_RECOMPOSEONRESIZE |
		OLEMISC_CANTLINKINSIDE |
		OLEMISC_INSIDEOUT |
		OLEMISC_ACTIVATEWHENVISIBLE |
		OLEMISC_SETCLIENTSITEFIRST)

	BEGIN_COM_MAP(CHelloControl)
		COM_INTERFACE_ENTRY(IViewObjectEx)
		COM_INTERFACE_ENTRY(IViewObject2)
		COM_INTERFACE_ENTRY(IViewObject)
		COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
		COM_INTERFACE_ENTRY(IOleInPlaceObject)
		COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObject)
		COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
		COM_INTERFACE_ENTRY(IOleControl)
		COM_INTERFACE_ENTRY(IOleObject)
		COM_INTERFACE_ENTRY(IDataObject)
		COM_INTERFACE_ENTRY(IPersistStreamInit)
		COM_INTERFACE_ENTRY(IPersistStorage)
		COM_INTERFACE_ENTRY2(IPersist, IPersistStorage)
	END_COM_MAP()

	BEGIN_MSG_MAP(CHelloControl)
		CHAIN_MSG_MAP(CComControl<CHelloControl>)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	// Paints the control. Called both for on-screen (in-place active)
	// rendering and for any static/metafile rendering the container asks for.
	HRESULT OnDraw(ATL_DRAWINFO& di)
	{
		HDC hdc = di.hdcDraw;
		const RECT& rc = *(const RECT*)di.prcBounds;

		::FillRect(hdc, &rc, (HBRUSH)(COLOR_BTNFACE + 1));
		::FrameRect(hdc, &rc, (HBRUSH)::GetStockObject(BLACK_BRUSH));

		::SetBkMode(hdc, TRANSPARENT);
		::SetTextColor(hdc, RGB(0, 0, 0x80));

		static const char szText[] = "Hi I'm an OLE Control running in the browser!";
		RECT rcText = rc;
		::InflateRect(&rcText, -8, -8);
		::DrawTextA(hdc, szText, -1, &rcText,
			DT_CENTER | DT_VCENTER | DT_WORDBREAK | DT_NOPREFIX);

		return S_OK;
	}
};

OBJECT_ENTRY_AUTO(CLSID_HelloOleControl, CHelloControl)

// ---------------------------------------------------------------------
// Module exports
// ---------------------------------------------------------------------

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	return _AtlModule.DllMain(dwReason, lpReserved);
}

STDAPI DllCanUnloadNow(void)
{
	return _AtlModule.DllCanUnloadNow();
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

// ---------------------------------------------------------------------
// Hand-written (de)registration.
//
// Deliberately bypasses ATL's usual .rgs/type-library registrar so the
// registry footprint is exactly what the 1996 container code expects:
//
//   HKEY_CLASSES_ROOT\CLSID\{clsid}                (default) = friendly name
//   HKEY_CLASSES_ROOT\CLSID\{clsid}\Control          (marker, just needs to exist)
//   HKEY_CLASSES_ROOT\CLSID\{clsid}\InprocServer32  (default) = full DLL path
//                                                    ThreadingModel = Apartment
//
// See CControlItem::CheckIfLocalOCX() in win32/cntlitem.cpp.
//
// Registration is written under HKEY_CURRENT_USER\Software\Classes rather
// than directly under HKEY_LOCAL_MACHINE, so no admin elevation is needed.
// Windows transparently merges HKCU\Software\Classes into
// HKEY_CLASSES_ROOT for the current user, so CheckIfLocalOCX()'s reads via
// HKEY_CLASSES_ROOT still find it.
// ---------------------------------------------------------------------

static void GetClsidKeyPath(wchar_t* buf, size_t cchBuf)
{
	wchar_t clsidStr[64];
	::StringFromGUID2(CLSID_HelloOleControl, clsidStr, 64);
	swprintf_s(buf, cchBuf, L"Software\\Classes\\CLSID\\%s", clsidStr);
}

STDAPI DllRegisterServer(void)
{
	wchar_t modulePath[MAX_PATH];
	if (::GetModuleFileNameW(_AtlBaseModule.GetModuleInstance(), modulePath, MAX_PATH) == 0)
		return HRESULT_FROM_WIN32(::GetLastError());

	wchar_t keyPath[128];
	GetClsidKeyPath(keyPath, 128);

	HKEY hClsidKey = NULL;
	LONG lRes = ::RegCreateKeyExW(HKEY_CURRENT_USER, keyPath, 0, NULL, 0,
		KEY_WRITE, NULL, &hClsidKey, NULL);
	if (lRes != ERROR_SUCCESS)
		return HRESULT_FROM_WIN32(lRes);

	::RegSetValueExW(hClsidKey, NULL, 0, REG_SZ,
		(const BYTE*)kFriendlyName,
		(DWORD)((wcslen(kFriendlyName) + 1) * sizeof(wchar_t)));

	HKEY hControlKey = NULL;
	::RegCreateKeyExW(hClsidKey, L"Control", 0, NULL, 0, KEY_WRITE, NULL, &hControlKey, NULL);
	if (hControlKey != NULL)
		::RegCloseKey(hControlKey);

	HKEY hInprocKey = NULL;
	lRes = ::RegCreateKeyExW(hClsidKey, L"InprocServer32", 0, NULL, 0,
		KEY_WRITE, NULL, &hInprocKey, NULL);
	if (lRes == ERROR_SUCCESS)
	{
		::RegSetValueExW(hInprocKey, NULL, 0, REG_SZ,
			(const BYTE*)modulePath,
			(DWORD)((wcslen(modulePath) + 1) * sizeof(wchar_t)));

		static const wchar_t kApartment[] = L"Apartment";
		::RegSetValueExW(hInprocKey, L"ThreadingModel", 0, REG_SZ,
			(const BYTE*)kApartment, sizeof(kApartment));

		::RegCloseKey(hInprocKey);
	}

	::RegCloseKey(hClsidKey);

	return (lRes == ERROR_SUCCESS) ? S_OK : HRESULT_FROM_WIN32(lRes);
}

STDAPI DllUnregisterServer(void)
{
	wchar_t keyPath[128];
	GetClsidKeyPath(keyPath, 128);

	LONG lRes = ::RegDeleteTreeW(HKEY_CURRENT_USER, keyPath);
	if (lRes != ERROR_SUCCESS && lRes != ERROR_FILE_NOT_FOUND)
		return HRESULT_FROM_WIN32(lRes);

	return S_OK;
}

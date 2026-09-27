# Building Ncompass on modern Windows

Ncompass remains a 32-bit MFC application. The modernized project preserves
the original HTML, networking, image, OLE, and ActiveX-control code paths.

## Requirements

- Visual Studio 2022 with the **Desktop development with C++** workload
- **C++ MFC for latest v143 build tools (x86 & x64)**
- A Windows 10 or Windows 11 SDK

The MFC component can be selected from the Visual Studio Installer under
**Individual components**.

## Build

Open `ncompass.sln` in Visual Studio 2022 and build either `Debug|Win32` or
`Release|Win32`.

From a Visual Studio Developer PowerShell:

```powershell
msbuild .\ncompass.vcxproj /t:Rebuild /p:Configuration=Release /p:Platform=Win32
```

The executable is written to `Release\ncompass.exe` (or
`Debug\ncompass.exe`).

## Notes

- The project intentionally uses the multibyte character set because the
  browser source predates Unicode Windows APIs.
- Static MFC and CRT linkage keeps the executable self-contained.
- `USES_OLE_CONTROLS` remains enabled to preserve the browser's historically
  significant OLE/ActiveX integration.
- Legacy CRT and Winsock deprecation warnings are suppressed. The remaining
  warnings describe original code behavior and do not prevent the build.

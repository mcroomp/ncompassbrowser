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

Open a URL directly from the command line with:

```powershell
.\Release\ncompass.exe http://127.0.0.1:8766/
```

For an automated end-to-end loading test, provide a URL and an expected
document-title substring:

```powershell
.\Release\ncompass-test.exe http://127.0.0.1:8766/ "Welcome to NCompass"
```

The test executable launches the real browser, waits up to 15 seconds for the
HTML loader to render the expected title, closes the browser, and returns zero
on success. An optional third argument changes the timeout in seconds. A fourth
argument keeps the browser open for that many seconds after the title matches,
allowing asynchronous image loads and Debug assertions to be validated.

## Automatic crash dumps

`ncompass-debug.exe` is a modern companion tool and is not linked into the
historical browser. It launches Ncompass under the Windows debugging API and
writes a minidump plus a text exception summary to the `dumps` directory beside
the helper if Ncompass has an unhandled exception or displays a Debug assertion.

Run the browser with automatic crash collection:

```powershell
.\Debug\ncompass-debug.exe https://www.google.com/
```

For an automated load check, add an expected document-title substring and an
optional timeout. A fourth argument captures the rendered browser client area
as a BMP for golden-image or hash comparison:

```powershell
.\Debug\ncompass-debug.exe https://www.google.com/ Google 60 google.bmp
```

Verify dump collection without crashing the browser:

```powershell
.\Debug\ncompass-debug.exe --self-test
```

Visible modal dialogs owned by Ncompass are recorded as both text and BMP files
in the same `dumps` directory, then dismissed so automation does not remain
blocked. Verify dialog capture with:

```powershell
.\Debug\ncompass-debug.exe --dialog-self-test
```

Open a captured dump in CDB with:

```powershell
cdb -z .\Debug\dumps\ncompass-*.dmp
```

## Direct parser testing

`ncompass-parse.exe` is a console test helper that compiles the original
`readhtml.cpp`, `bigstr.cpp`, `contain.cpp`, and `mutex.cpp` files directly.
Only MIME notification plumbing and JSON serialization are supplied by the
test project. The helper therefore exercises the browser's real HTML parser
without launching its document, layout, or window layers.

Parse a file in deliberately small network-style chunks:

```powershell
.\Debug\ncompass-parse.exe .\tests\parser\basic.html 1 `
    http://parser.test/pages/input.html
```

The output describes the load state, title, plain-text buffer, document
colors, background image, and typed tag stream as JSON. The optional chunk
size defaults to 4096 bytes, and the optional base URL is used to resolve
relative links and images.

Run the deterministic parser regression suite with:

```powershell
.\tools\test-parser.ps1 -Configuration Debug
```

The suite feeds each fixture in 1, 2, 7, 31, and 4096-byte chunks. It
normalizes only adjacent text records with the same formatting because the
historical streaming parser intentionally emits text records at input-buffer
boundaries; all other parser output must match.

## Browser layers

The browser keeps the original separation between transport and display:

1. `CDynamicLoad` selects the file or HTTP protocol and marshals asynchronous
   load notifications.
2. `CMimeDynamicLoad` dispatches response data to `CParseHTML`, `CGifPicture`,
   `CJpegPicture`, or another MIME object.
3. `CParseHTML` consumes arbitrary byte chunks and produces a plain-text
   buffer plus a typed `CTag` stream.
4. `CFormatHTML` converts those tags into positioned format items, including
   text, links, images, headings, lists, and tables.
5. `CViewhtmlView` draws visible format items through GDI and hosts OLE
   controls separately.
6. Image MIME objects decode asynchronously and notify the view to repaint
   affected regions.

This makes the parser helper a focused test of layer 3. `ncompass-debug.exe`
continues to provide full-stack browser and rendered-image testing.

## Notes

- The project intentionally uses the multibyte character set because the
  browser source predates Unicode Windows APIs.
- Static MFC and CRT linkage keeps the executable self-contained.
- `USES_OLE_CONTROLS` remains enabled to preserve the browser's historically
  significant OLE/ActiveX integration.
- HTTP and HTTPS use the Windows WinHTTP stack, providing HTTP/1.1, TLS,
  certificate validation, and system proxy support without third-party
  dependencies.
- Script bodies are ignored because the browser has no JavaScript engine;
  `noscript` fallback content is rendered.
- Legacy CRT and Winsock deprecation warnings are suppressed. The remaining
  warnings describe original code behavior and do not prevent the build.

# Building Ncompass on modern Windows

Ncompass remains a 32-bit MFC application. The modernized project preserves
the original HTML, networking, image, OLE, and ActiveX-control code paths.

## Requirements

- Visual Studio 2022 with the **Desktop development with C++** workload
- **C++ MFC for latest v143 build tools (x86 & x64)**
- A Windows 10 or Windows 11 SDK

The MFC component can be selected from the Visual Studio Installer under
**Individual components**.

## Test infrastructure

All modern test/harness code lives under [testing/](testing), clearly
separate from the historical browser at the repository root (`win32\`,
`cross_p\`, `jpeg\`, and `ncompass.vcxproj`). There are exactly two modern
executables:

- **`ncompass-debug.exe`** — a single native C++17 tool that is the one
  entry point for every kind of test: launching and monitoring the browser,
  multi-site regression, multi-page navigation, crash/timeout/dialog
  self-tests, isolated HTTP fetches, and generic process/job wrapping. It has
  no historical-code dependency and does not link into the browser.
- **`ncompass-parse.exe`** — the one exception, because it must directly
  compile the browser's own `readhtml.cpp`/`bigstr.cpp`/`contain.cpp`/
  `mutex.cpp` to exercise the *real* parser. That requires matching the
  browser's own build settings (multibyte character set, static MFC), which
  is incompatible with `ncompass-debug.exe`'s plain modern C++17 build, so it
  stays a separate binary.

## Build

Open `ncompass.sln` in Visual Studio 2022 and build either `Debug|Win32` or
`Release|Win32`. It has three projects: `ncompass` (the browser, at the
repository root), and `ncompass-debug`/`ncompass-parse` (under `testing\`).

From a Visual Studio Developer PowerShell:

```powershell
msbuild .\ncompass.sln /t:Rebuild /p:Configuration=Release /p:Platform=Win32 /m
```

The browser executable is written to `Release\ncompass.exe` (or
`Debug\ncompass.exe`); the test tools land alongside it in the same
`Release\`/`Debug\` directory.

All 49 browser, platform, and bundled JPEG translation units use the same
`cross_p.h` precompiled header, which contains the expensive MFC and Windows
headers through `stdafx.h`. Only `win32\stdafx.cpp` creates the PCH. Compiler
`/MP` parallelism is enabled in both configurations; use MSBuild `/m` as well
when building the solution. On the current 24-logical-processor machine, an
isolated clean Debug browser rebuild takes about 6.0 seconds and a no-change
incremental build about 0.48 seconds.

Open a URL directly from the command line, contained in a job so nothing is
left running if it's interrupted:

```powershell
.\Release\ncompass-debug.exe --wrap -- `
    .\Release\ncompass.exe http://127.0.0.1:8766/
```

## Automatic crash dumps

`ncompass-debug.exe` is a modern companion tool and is not linked into the
historical browser. Its default mode launches Ncompass under the Windows
debugging API and writes a minidump plus a text exception summary to the
`dumps` directory beside the helper if Ncompass has an unhandled exception or
triggers a Debug assertion. Debug assertions are converted to
non-continuable exceptions at startup instead of displaying the interactive
CRT assertion dialog, so unattended tests fail immediately and the helper can
collect the failing stack.

The timeout is enforced by an independent watchdog, including when the debug
event stream is busy. On timeout the watchdog suspends every target thread,
captures a bounded minidump, walks each thread's stack in-process with
DbgHelp (no external debugger dependency), and writes the result beside the
dump as `<dump>.dmp.stack.txt`, then terminates Ncompass. Successful
screenshot capture remains outside the deadline-critical timeout path because
`PrintWindow` can block on an unresponsive UI; it is bounded separately with
`WM_PRINT` and falls back to `BitBlt`.

Provide a URL, an expected document-title substring, and an optional timeout
in seconds for an automated load check. A fourth argument captures the
rendered browser client area as a BMP for golden-image or hash comparison:

```powershell
.\Debug\ncompass-debug.exe https://www.google.com/ Google 60 google.bmp
```

Verify dump collection without crashing the browser:

```powershell
.\Debug\ncompass-debug.exe --self-test
```

Verify the strict timeout, dump, and stack-trace path with:

```powershell
.\Debug\ncompass-debug.exe --timeout-self-test
```

Visible modal dialogs owned by Ncompass are recorded as both text and BMP files
in the same `dumps` directory, then dismissed so automation does not remain
blocked. Verify dialog capture with:

```powershell
.\Debug\ncompass-debug.exe --dialog-self-test
```

Open a captured dump in CDB (for manual, ad hoc analysis only; nothing in the
harness itself depends on CDB) with:

```powershell
cdb -z .\Debug\dumps\ncompass-*.dmp
```

## Multi-site crash testing

`ncompass-debug.exe --sites` is the native C++17 multi-site harness. It reads
`tests\sites.tsv`, launches each browser under the Windows debugging API, and
uses a strict 1,000 ms default deadline. Timeouts and crashes produce minidumps
and stack traces walked in-process with DbgHelp; successful loads produce
screenshots. Results are written to a machine-readable `summary.json`. The
PowerShell script is only a thin launcher for the native executable.

An expected title may list `|`-separated alternatives, matched against
whatever window title is actually reached (for example a captive portal or
CDN challenge page that legitimately intercepts a request without the
browser crashing or hanging). Hosts intentionally redirected to `127.0.0.1`
in the test machine's hosts file belong in `tests\sites-local.tsv` instead,
against a running fixture server, rather than the public `tests\sites.tsv`
manifest.

Run the complete Debug site set:

```powershell
.\testing\ncompass-harness.ps1 -Configuration Debug `
    -TimeoutMilliseconds 1000
```

Call the native harness directly:

```powershell
.\Debug\ncompass-debug.exe --sites .\tests\sites.tsv `
    .\site-results 1000 on-crash
```

Test one URL without editing the manifest:

```powershell
.\testing\ncompass-harness.ps1 -Mode Single -Name example `
    -Url https://example.com/ -ExpectedTitle "Example Domain"
```

The default `-PageHeap OnCrash` mode reruns only a classified crash with full
PageHeap enabled through `gflags.exe`. `-PageHeap Always` applies it to every
attempt, while `-PageHeap Off` never changes global debugger configuration.
The harness disables PageHeap in a `finally` block before continuing.

A title mismatch, timeout, modal dialog, assertion, or crash fails the command.
Unsupported CSS, JavaScript, PNG, or progressive JPEG rendering does not fail
when the expected title is reached and the browser exits cleanly.

## Multi-page navigation testing

`ncompass-debug.exe --navigate` drives one long-lived browser process through
a sequence of URLs and expected titles, reusing the same window, document,
and loader state across each hop instead of starting a fresh process per
page. This exercises cancellation, loader reuse, history/back-forward state,
and picture-cache eviction under repeated real navigation, not just
independent page loads.

```powershell
.\Debug\ncompass-debug.exe --navigate 30000 `
    https://www.google.com/ Google `
    https://www.wikipedia.org/ Wikipedia `
    https://www.amazon.com/ Amazon.com
```

Each hop is triggered through the browser's own, unmodified location bar:
the monitor sets the `IDC_LOCATION` edit control's text and sends the same
`ID_OPEN_LOCATION` command a user's Enter key would, calling
`CViewhtmlApp::OpenDocumentFile` exactly as interactive use does. No
test-only code exists in the browser for this. Because a UI action can miss
if the UI thread was briefly busy loading the previous page, the monitor
retries the same hop if the title hasn't changed within about two seconds;
`OpenDocumentFile` on an unchanged URL is idempotent, so a duplicate attempt
is harmless.

## Headless URL and parser race testing

`ncompass-parse.exe --stress-url` fetches a URL without creating browser UI,
then parses the same decoded bytes concurrently through independent instances
of the original `CParseHTML`. Workers rotate through 1, 2, 7, 31, and 4096-byte
chunks and compare normalized semantic fingerprints. This detects hangs,
crashes, shared-state races, and chunk-dependent parser output without mixing
in formatting, image loading, or window behavior.

Run 20 parses on four worker threads under the native one-second monitor:

```powershell
.\Debug\ncompass-debug.exe --command 1000 `
    .\Debug\ncompass-parse.exe --stress-url `
    https://www.google.com/ 20 4 1000
```

If the headless command exceeds one second, the monitor captures its dump and
in-process DbgHelp stack trace just as it does for the browser.

## Process cleanup

`ncompass-debug.exe --wrap` is a generic process wrapper for test and
debugging tools. It creates a Windows job object with
`JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE`, starts the requested command suspended,
assigns it to the job, and then resumes it. Every descendant remains in that
job, including Ncompass processes launched by `ncompass-debug.exe`'s other
modes and debuggees launched by CDB. Closing or terminating the wrapper
therefore removes the complete process tree.

Run CDB and its Ncompass debuggee inside one job:

```powershell
.\Debug\ncompass-debug.exe --wrap -- `
    cdb -o -g -G .\Debug\ncompass.exe https://www.amazon.com/
```

Run a local fixture server inside a job:

```powershell
.\Debug\ncompass-debug.exe --wrap -- `
    python -m http.server 8766 --bind 127.0.0.1 `
    --directory .\tests\parser
```

The wrapper also accepts `--cwd directory` before the command. Run its cleanup
regression with:

```powershell
.\testing\test-job.ps1 -Configuration Debug
```

## Direct HTTP testing

`ncompass-debug.exe --fetch` isolates the browser's WinINet request behavior
from the loader, parser, formatter, image decoders, and UI. It uses the
Ncompass user agent, HTTPS certificate validation, redirects, proxy
configuration, gzip/deflate decoding, and no-cache request flags.

Every invocation supervises the request in a re-exec'd worker process (itself,
launched with an internal `--fetch-worker` mode), reusing exactly the same
job/timeout/dump/in-process-stack-walk machinery as every other
`ncompass-debug.exe` mode. The optional deadline is a hard wall-clock limit; a
timed-out worker is dumped and terminated with exit code 124, and the dump
plus stack trace land in the usual `dumps` directory.

Fetch and decode a response with a two-second deadline and 16 MB size limit:

```powershell
.\Debug\ncompass-debug.exe --fetch https://www.amazon.com/ amazon.html 2000 16777216
```

Test transport and parsing as separate contracts:

```powershell
.\Debug\ncompass-debug.exe --fetch https://www.amazon.com/ amazon.html 2000
.\Debug\ncompass-parse.exe amazon.html 4096 https://www.amazon.com/
```

Run the HTTP contract test, including a deterministic timeout that must produce
both a dump and text stack:

```powershell
.\testing\test-http.ps1 -Configuration Debug -TimeoutMilliseconds 5000
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
.\testing\test-parser.ps1 -Configuration Debug
```

Standard Windows file URLs use `file:///C:/path` syntax. Ncompass also accepts
escaped characters and UNC file URLs through the Windows URL parser. Verify a
standard URL containing spaces and `#` with:

```powershell
.\testing\test-file-url.ps1 -Configuration Debug
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

## Thread model and its Debug assertions

The loader's original contract is a strict two-side handoff, guarded by
`CAccessLock` on each shared object:

- The **UI thread** calls `StartLoading`, `AbortLoading`, `DoNotifies`,
  `CNotifyObject::OnNotify` overrides, `OnPreLoading`, and
  `CFormatHTML::Format`.
- A **protocol worker thread** (`CProtocolFile`, `CProtocolHTTP`,
  `CProtocolSMTP`, or the trivial `CProtocolUnknown`) calls `OnBeginLoading`,
  `OnLoading`, `OnEndLoading`, and the `CMimeObject::OnReadData`/
  `OnEndOfFile` overrides that do the real parsing/decoding work.

Every loading path — including an unrecognized URL scheme and cancelling a
load that is still queued for a free connection slot — now always creates a
real protocol object and worker thread, even if that thread does nothing but
immediately call `OnEndLoading()` and signal completion
(`CProtocolUnknown`/`UnknownProtocolWorkerThread` in `protocol.cpp`). This
removes the previous special cases where the dynamic loader called
`OnEndLoading()` directly from whichever thread happened to call
`AbortLoading()` or `InvokeLoadingThread()`.

In Debug builds, `ASSERT_UI_THREAD()` and `ASSERT_WORKER_THREAD()`
(`mutex.h`/`mutex.cpp`) record the UI thread's ID once at startup
(`SetUIThreadID()`, called from `CViewhtmlApp::InitInstance`) and assert
immediately if a function reserved for one side of the loader is ever
reached from the other. Like `ASSERT_LOCKED`, a failing assertion is
converted to a non-continuable exception at startup, so a thread-model
violation crashes immediately with a dump under `ncompass-debug.exe` instead
of silently corrupting shared state.

## Notes

- The project intentionally uses the multibyte character set because the
  browser source predates Unicode Windows APIs.
- Static MFC and CRT linkage keeps the executable self-contained.
- `USES_OLE_CONTROLS` remains enabled to preserve the browser's historically
  significant OLE/ActiveX integration.
- HTTP and HTTPS use the Windows WinINet stack (`wininet.h`/`wininet.lib`,
  not the separate WinHTTP library), providing HTTP/1.1, TLS, certificate
  validation, and system proxy support without third-party dependencies.
- Script bodies are ignored because the browser has no JavaScript engine;
  `noscript` fallback content is rendered.
- Legacy CRT and Winsock deprecation warnings are suppressed. The remaining
  warnings describe original code behavior and do not prevent the build.
- A freshly built executable's first run may take much longer than usual
  under real-time antivirus scanning; this delay happens before the strict
  per-site deadline starts and is not a harness or browser defect.

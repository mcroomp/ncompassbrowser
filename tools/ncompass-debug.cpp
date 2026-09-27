#include <windows.h>
#include <dbghelp.h>

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>


struct WindowSearch
	{
	DWORD process_id;
	HWND browser_window;
	HWND assertion_window;
	HWND dialog_window;
	};


BOOL CALLBACK FindProcessWindows(HWND window, LPARAM lparam)
	{
	WindowSearch *search = reinterpret_cast<WindowSearch *>(lparam);
	DWORD process_id = 0;
	GetWindowThreadProcessId(window, &process_id);

	if (process_id != search->process_id || !IsWindowVisible(window))
		return TRUE;

	wchar_t title[256];
	GetWindowTextW(window, title, _countof(title));
	bool is_dialog = false;
	if (wcscmp(title, L"Microsoft Visual C++ Runtime Library") == 0)
		{
		search->assertion_window = window;
		is_dialog = true;
		}
	else
		{
		wchar_t class_name[64];
		GetClassNameW(window, class_name, _countof(class_name));
		if (wcscmp(class_name, L"#32770") == 0)
			{
			search->dialog_window = window;
			is_dialog = true;
			}
		}

	if (!is_dialog && !search->browser_window &&
		GetWindow(window, GW_OWNER) == NULL)
		search->browser_window = window;

	return TRUE;
	}


std::wstring GetModulePath()
	{
	wchar_t path[MAX_PATH];
	DWORD length = GetModuleFileNameW(NULL, path, _countof(path));
	if (length == 0 || length == _countof(path))
		return std::wstring();
	return std::wstring(path, length);
	}


std::wstring GetModuleDirectory()
	{
	std::wstring path = GetModulePath();
	size_t separator = path.find_last_of(L"\\/");
	if (separator == std::wstring::npos)
		return std::wstring();
	return path.substr(0, separator);
	}


std::wstring QuoteArgument(const std::wstring& argument)
	{
	return L"\"" + argument + L"\"";
	}


std::wstring MakeDumpBaseName(DWORD process_id)
	{
	SYSTEMTIME time;
	GetLocalTime(&time);

	wchar_t name[128];
	swprintf_s(name, _countof(name),
		L"ncompass-%04u%02u%02u-%02u%02u%02u-%03u-%lu",
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute,
		time.wSecond, time.wMilliseconds, process_id);
	return name;
	}


std::wstring GetOutputDirectory()
	{
	std::wstring directory = GetModuleDirectory() + L"\\dumps";
	if (!CreateDirectoryW(directory.c_str(), NULL) &&
		GetLastError() != ERROR_ALREADY_EXISTS)
		return std::wstring();
	return directory;
	}


bool WriteTextFile(const std::wstring& path, const std::wstring& text)
	{
	HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE)
		return false;

	int byte_count = WideCharToMultiByte(CP_UTF8, 0, text.c_str(),
		static_cast<int>(text.size()), NULL, 0, NULL, NULL);
	if (!byte_count)
		{
		CloseHandle(file);
		return false;
		}

	std::vector<char> bytes(byte_count);
	WideCharToMultiByte(CP_UTF8, 0, text.c_str(),
		static_cast<int>(text.size()), bytes.data(), byte_count, NULL, NULL);

	DWORD written = 0;
	const BYTE byte_order_mark[] = { 0xEF, 0xBB, 0xBF };
	BOOL result = WriteFile(file, byte_order_mark,
		sizeof(byte_order_mark), &written, NULL);
	if (result)
		result = WriteFile(file, bytes.data(),
			static_cast<DWORD>(bytes.size()), &written, NULL);
	CloseHandle(file);
	return result != FALSE;
	}


bool CaptureWindowBitmap(HWND window, const std::wstring& path)
	{
	RECT client;
	if (!GetClientRect(window, &client))
		return false;

	int width = client.right - client.left;
	int height = client.bottom - client.top;
	if (width <= 0 || height <= 0)
		return false;

	HDC window_dc = GetDC(window);
	HDC memory_dc = CreateCompatibleDC(window_dc);
	HBITMAP bitmap = CreateCompatibleBitmap(window_dc, width, height);
	HGDIOBJ old_bitmap = SelectObject(memory_dc, bitmap);

	BOOL rendered = PrintWindow(window, memory_dc, PW_CLIENTONLY);
	if (!rendered)
		rendered = BitBlt(memory_dc, 0, 0, width, height,
			window_dc, 0, 0, SRCCOPY);

	BITMAPINFO info = {};
	info.bmiHeader.biSize = sizeof(info.bmiHeader);
	info.bmiHeader.biWidth = width;
	info.bmiHeader.biHeight = height;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 24;
	info.bmiHeader.biCompression = BI_RGB;

	DWORD stride = (width * 3 + 3) & ~3;
	std::vector<BYTE> pixels(stride * height);
	BOOL copied = rendered && GetDIBits(memory_dc, bitmap, 0, height,
		pixels.data(), &info, DIB_RGB_COLORS);

	SelectObject(memory_dc, old_bitmap);
	DeleteObject(bitmap);
	DeleteDC(memory_dc);
	ReleaseDC(window, window_dc);

	if (!copied)
		return false;

	HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE)
		return false;

	BITMAPFILEHEADER file_header = {};
	file_header.bfType = 0x4D42;
	file_header.bfOffBits = sizeof(file_header) + sizeof(info.bmiHeader);
	file_header.bfSize = file_header.bfOffBits +
		static_cast<DWORD>(pixels.size());

	DWORD written = 0;
	BOOL result = WriteFile(file, &file_header, sizeof(file_header),
		&written, NULL);
	if (result)
		result = WriteFile(file, &info.bmiHeader, sizeof(info.bmiHeader),
			&written, NULL);
	if (result)
		result = WriteFile(file, pixels.data(),
			static_cast<DWORD>(pixels.size()), &written, NULL);
	CloseHandle(file);
	return result != FALSE;
	}


BOOL CALLBACK AppendChildWindowText(HWND window, LPARAM lparam)
	{
	std::wstring *text = reinterpret_cast<std::wstring *>(lparam);
	int length = GetWindowTextLengthW(window);
	if (!length)
		return TRUE;

	std::vector<wchar_t> buffer(length + 1);
	GetWindowTextW(window, buffer.data(), static_cast<int>(buffer.size()));
	if (!text->empty())
		*text += L"\r\n";
	*text += buffer.data();
	return TRUE;
	}


std::wstring CaptureDialog(HWND dialog, DWORD process_id)
	{
	std::wstring directory = GetOutputDirectory();
	if (directory.empty())
		return std::wstring();

	wchar_t title[512];
	GetWindowTextW(dialog, title, _countof(title));

	std::wstring text = L"Window: ";
	text += title;
	text += L"\r\n";
	EnumChildWindows(dialog, AppendChildWindowText,
		reinterpret_cast<LPARAM>(&text));
	text += L"\r\n";

	wchar_t suffix[64];
	swprintf_s(suffix, _countof(suffix), L"-dialog-%lu.txt",
		GetTickCount());
	std::wstring path = directory + L"\\" +
		MakeDumpBaseName(process_id) + suffix;
	if (!WriteTextFile(path, text))
		return std::wstring();

	std::wstring bitmap_path = path.substr(0, path.size() - 4) + L".bmp";
	CaptureWindowBitmap(dialog, bitmap_path);
	std::wcout << L"Dialog capture: " << path << L"\n";
	return path;
	}


void DismissDialog(HWND dialog)
	{
	static const int button_ids[] =
		{ IDOK, IDCANCEL, IDNO, IDABORT, IDIGNORE, IDYES };

	for (int i = 0; i < _countof(button_ids); i++)
		{
		HWND button = GetDlgItem(dialog, button_ids[i]);
		if (button && IsWindowEnabled(button))
			{
			PostMessageW(button, BM_CLICK, 0, 0);
			return;
			}
		}

	PostMessageW(dialog, WM_CLOSE, 0, 0);
	}


bool WriteCrashDump(HANDLE process, DWORD process_id, DWORD thread_id,
	const EXCEPTION_RECORD *exception_record, std::wstring& dump_path)
	{
	std::wstring dump_directory = GetOutputDirectory();
	if (dump_directory.empty())
		{
		std::wcerr << L"Could not create dump directory (error "
			<< GetLastError() << L")\n";
		return false;
		}

	std::wstring base_path = dump_directory + L"\\" +
		MakeDumpBaseName(process_id);
	dump_path = base_path + L".dmp";
	HANDLE dump_file = CreateFileW(dump_path.c_str(), GENERIC_WRITE,
		FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (dump_file == INVALID_HANDLE_VALUE)
		{
		std::wcerr << L"Could not create dump file (error "
			<< GetLastError() << L")\n";
		return false;
		}

	MINIDUMP_EXCEPTION_INFORMATION exception_info;
	MINIDUMP_EXCEPTION_INFORMATION *exception_info_pointer = NULL;
	EXCEPTION_RECORD record;
	CONTEXT context;
	EXCEPTION_POINTERS pointers;
	HANDLE thread = NULL;

	if (exception_record && thread_id)
		{
		record = *exception_record;
		ZeroMemory(&context, sizeof(context));
		context.ContextFlags = CONTEXT_ALL;
		thread = OpenThread(THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION,
			FALSE, thread_id);

		if (thread && GetThreadContext(thread, &context))
			{
			pointers.ExceptionRecord = &record;
			pointers.ContextRecord = &context;
			exception_info.ThreadId = thread_id;
			exception_info.ExceptionPointers = &pointers;
			exception_info.ClientPointers = FALSE;
			exception_info_pointer = &exception_info;
			}
		}

	MINIDUMP_TYPE dump_type = static_cast<MINIDUMP_TYPE>(
		MiniDumpWithDataSegs |
		MiniDumpWithHandleData |
		MiniDumpWithIndirectlyReferencedMemory |
		MiniDumpWithUnloadedModules |
		MiniDumpWithFullMemoryInfo |
		MiniDumpWithThreadInfo);

	BOOL result = MiniDumpWriteDump(process, process_id, dump_file,
		dump_type, exception_info_pointer, NULL, NULL);
	DWORD error = result ? ERROR_SUCCESS : GetLastError();

	if (thread)
		CloseHandle(thread);
	CloseHandle(dump_file);

	if (!result)
		{
		DeleteFileW(dump_path.c_str());
		std::wcerr << L"MiniDumpWriteDump failed (error " << error << L")\n";
		return false;
		}

	wchar_t summary[1024];
	if (exception_record)
		{
		swprintf_s(summary, _countof(summary),
			L"Ncompass crash dump\r\n"
			L"Process ID: %lu\r\n"
			L"Thread ID: %lu\r\n"
			L"Exception code: 0x%08lX\r\n"
			L"Exception address: 0x%p\r\n"
			L"Dump: %s\r\n",
			process_id, thread_id, exception_record->ExceptionCode,
			exception_record->ExceptionAddress, dump_path.c_str());
		}
	else
		{
		swprintf_s(summary, _countof(summary),
			L"Ncompass Debug assertion dump\r\n"
			L"Process ID: %lu\r\n"
			L"Dump: %s\r\n",
			process_id, dump_path.c_str());
		}
	WriteTextFile(base_path + L".txt", summary);

	std::wcout << L"Crash dump: " << dump_path << L"\n";
	return true;
	}


int MonitorProcess(const std::wstring& application,
	const std::wstring& arguments, const std::wstring& expected_title,
	DWORD timeout_seconds, const std::wstring& screenshot_path,
	bool dump_self_test, bool dialog_self_test)
	{
	std::wstring command_line = QuoteArgument(application);
	if (!arguments.empty())
		command_line += L" " + arguments;
	std::vector<wchar_t> command_buffer(command_line.begin(),
		command_line.end());
	command_buffer.push_back(L'\0');

	STARTUPINFOW startup_info = {};
	startup_info.cb = sizeof(startup_info);
	PROCESS_INFORMATION process = {};
	if (!CreateProcessW(application.c_str(), command_buffer.data(), NULL, NULL,
		FALSE, DEBUG_ONLY_THIS_PROCESS, NULL, NULL, &startup_info, &process))
		{
		std::wcerr << L"Could not start process (error " << GetLastError()
			<< L")\n";
		return 2;
		}

	DWORD started = GetTickCount();
	DWORD title_matched = 0;
	bool process_exited = false;
	bool dump_written = false;
	bool assertion_dumped = false;
	bool dialog_captured = false;
	bool timed_out = false;
	bool screenshot_written = false;
	DWORD exit_code = 0;
	HWND browser_window = NULL;
	std::vector<HWND> handled_dialogs;

	while (!process_exited)
		{
		DEBUG_EVENT event;
		DWORD wait_time = 100;
		while (WaitForDebugEvent(&event, wait_time))
			{
			wait_time = 0;
			DWORD continue_status = DBG_CONTINUE;

			switch(event.dwDebugEventCode)
				{
				case EXCEPTION_DEBUG_EVENT:
					if (event.u.Exception.dwFirstChance &&
						event.u.Exception.ExceptionRecord.ExceptionCode ==
							EXCEPTION_BREAKPOINT)
						{
						continue_status = DBG_CONTINUE;
						}
					else if (event.u.Exception.dwFirstChance)
						{
						continue_status = DBG_EXCEPTION_NOT_HANDLED;
						}
					else
						{
						std::wstring dump_path;
						dump_written = WriteCrashDump(process.hProcess,
							process.dwProcessId, event.dwThreadId,
							&event.u.Exception.ExceptionRecord, dump_path);
						continue_status = DBG_EXCEPTION_NOT_HANDLED;
						}
					break;

				case CREATE_PROCESS_DEBUG_EVENT:
					if (event.u.CreateProcessInfo.hFile)
						CloseHandle(event.u.CreateProcessInfo.hFile);
					break;

				case CREATE_THREAD_DEBUG_EVENT:
					if (event.u.CreateThread.hThread)
						CloseHandle(event.u.CreateThread.hThread);
					break;

				case LOAD_DLL_DEBUG_EVENT:
					if (event.u.LoadDll.hFile)
						CloseHandle(event.u.LoadDll.hFile);
					break;

				case EXIT_PROCESS_DEBUG_EVENT:
					exit_code = event.u.ExitProcess.dwExitCode;
					process_exited = true;
					break;
				}

			ContinueDebugEvent(event.dwProcessId, event.dwThreadId,
				continue_status);

			if (process_exited)
				break;
			}

		if (process_exited)
			break;

		WindowSearch search = { process.dwProcessId, NULL, NULL, NULL };
		EnumWindows(FindProcessWindows, reinterpret_cast<LPARAM>(&search));
		if (search.browser_window)
			browser_window = search.browser_window;

		if (search.assertion_window && !assertion_dumped)
			{
			CaptureDialog(search.assertion_window, process.dwProcessId);
			std::wstring dump_path;
			assertion_dumped = true;
			dump_written = WriteCrashDump(process.hProcess,
				process.dwProcessId, 0, NULL, dump_path);
			HWND abort_button = GetDlgItem(search.assertion_window, IDABORT);
			if (abort_button)
				PostMessageW(abort_button, BM_CLICK, 0, 0);
			}

		if (search.dialog_window &&
			std::find(handled_dialogs.begin(), handled_dialogs.end(),
				search.dialog_window) == handled_dialogs.end())
			{
			handled_dialogs.push_back(search.dialog_window);
			dialog_captured =
				!CaptureDialog(search.dialog_window,
					process.dwProcessId).empty();
			DismissDialog(search.dialog_window);
			}

		if (!expected_title.empty() && browser_window)
			{
			wchar_t title[512];
			GetWindowTextW(browser_window, title, _countof(title));
			if (wcsstr(title, expected_title.c_str()))
				{
				if (!title_matched)
					title_matched = GetTickCount();
				else if (GetTickCount() - title_matched >= 1000)
					{
					if (!screenshot_path.empty() && !screenshot_written)
						{
						screenshot_written =
							CaptureWindowBitmap(browser_window,
								screenshot_path);
						if (screenshot_written)
							std::wcout << L"Screenshot: "
								<< screenshot_path << L"\n";
						else
							std::wcerr << L"Could not capture screenshot\n";
						}
					PostMessageW(browser_window, WM_CLOSE, 0, 0);
					}
				}
			}

		if (!timed_out && timeout_seconds &&
			GetTickCount() - started >= timeout_seconds * 1000)
			{
			timed_out = true;
			std::wcerr << L"Timed out waiting for the process\n";
			if (browser_window && !screenshot_path.empty() &&
				!screenshot_written)
				{
				screenshot_written =
					CaptureWindowBitmap(browser_window, screenshot_path);
				if (screenshot_written)
					std::wcout << L"Timeout screenshot: "
						<< screenshot_path << L"\n";
				}
			if (browser_window)
				PostMessageW(browser_window, WM_CLOSE, 0, 0);
			TerminateProcess(process.hProcess, 2);
			}
		}

	CloseHandle(process.hThread);
	CloseHandle(process.hProcess);

	if (dump_self_test)
		{
		if (dump_written)
			{
			std::wcout << L"PASS: crash dump self-test\n";
			return 0;
			}
		std::wcerr << L"FAIL: crash dump was not written\n";
		return 1;
		}

	if (dialog_self_test)
		{
		if (dialog_captured)
			{
			std::wcout << L"PASS: dialog capture self-test\n";
			return 0;
			}
		std::wcerr << L"FAIL: dialog was not captured\n";
		return 1;
		}

	if (timed_out)
		return 1;

	if (!screenshot_path.empty() && !screenshot_written)
		return 1;

	if (dump_written)
		return 1;

	if (!expected_title.empty() && !title_matched)
		{
		std::wcerr << L"FAIL: expected title was not rendered\n";
		return 1;
		}

	if (title_matched)
		{
		std::wcout << L"PASS: " << expected_title << L"\n";
		return 0;
		}

	return static_cast<int>(exit_code);
	}


int wmain(int argc, wchar_t *argv[])
	{
	if (argc == 2 && wcscmp(argv[1], L"--crash-child") == 0)
		{
		volatile int *invalid = NULL;
		*invalid = 1;
		return 1;
		}

	if (argc == 2 && wcscmp(argv[1], L"--dialog-child") == 0)
		{
		MessageBoxW(NULL, L"Automatic dialog capture test message.",
			L"Ncompass Debug Dialog Test", MB_OK | MB_ICONERROR);
		return 0;
		}

	if (argc == 2 && wcscmp(argv[1], L"--self-test") == 0)
		return MonitorProcess(GetModulePath(), L"--crash-child",
			std::wstring(), 15, std::wstring(), true, false);

	if (argc == 2 && wcscmp(argv[1], L"--dialog-self-test") == 0)
		return MonitorProcess(GetModulePath(), L"--dialog-child",
			std::wstring(), 15, std::wstring(), false, true);

	if (argc < 2 || argc > 5)
		{
		std::wcerr
			<< L"Usage: ncompass-debug <url> [expected-title] "
				L"[timeout-seconds] [screenshot.bmp]\n"
			<< L"       ncompass-debug --self-test\n"
			<< L"       ncompass-debug --dialog-self-test\n";
		return 2;
		}

	DWORD timeout_seconds = argc >= 3 ? 60 : 0;
	if (argc == 4)
		{
		wchar_t *end = NULL;
		unsigned long value = wcstoul(argv[3], &end, 10);
		if (!argv[3][0] || *end || !value)
			{
			std::wcerr << L"timeout-seconds must be a positive integer\n";
			return 2;
			}
		timeout_seconds = value;
		}

	std::wstring browser = GetModuleDirectory() + L"\\ncompass.exe";
	if (GetFileAttributesW(browser.c_str()) == INVALID_FILE_ATTRIBUTES)
		{
		std::wcerr << L"Could not find ncompass.exe beside the helper\n";
		return 2;
		}

	std::wstring expected_title = argc >= 3 ? argv[2] : L"";
	std::wstring screenshot_path = argc == 5 ? argv[4] : L"";
	return MonitorProcess(browser, argv[1], expected_title,
		timeout_seconds, screenshot_path, false, false);
	}

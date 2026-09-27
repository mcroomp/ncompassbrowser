#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct
	{
	DWORD process_id;
	HWND window;
	HWND assertion_window;
	} WINDOW_SEARCH;


BOOL CALLBACK FindProcessWindow(HWND window, LPARAM lparam)
	{
	WINDOW_SEARCH *search = (WINDOW_SEARCH *)lparam;
	DWORD process_id = 0;
	GetWindowThreadProcessId(window, &process_id);

	if (process_id == search->process_id && IsWindowVisible(window))
		{
		char title[128];
		GetWindowText(window, title, sizeof(title));
		if (strcmp(title, "Microsoft Visual C++ Runtime Library") == 0)
			search->assertion_window = window;
		else if (search->window == NULL && GetWindow(window, GW_OWNER) == NULL)
			search->window = window;
		}

	return TRUE;
	}


BOOL CALLBACK PrintWindowText(HWND window, LPARAM lparam)
	{
	char text[1024];
	GetWindowText(window, text, sizeof(text));
	if (text[0] != '\0')
		fprintf(stderr, "%s\n", text);
	return TRUE;
	}


BOOL GetBrowserPath(char *path)
	{
	char *separator;
	DWORD length = GetModuleFileName(NULL, path, MAX_PATH);
	if (length == 0 || length == MAX_PATH)
		return FALSE;

	separator = strrchr(path, '\\');
	if (separator == NULL)
		return FALSE;

	strcpy(separator + 1, "ncompass.exe");
	return TRUE;
	}


void GetBrowserTitle(HWND window, char *title)
	{
	title[0] = '\0';
	if (window)
		GetWindowText(window, title, 512);
	}


BOOL CloseBrowser(PROCESS_INFORMATION *process, HWND window)
	{
	BOOL closed = TRUE;

	if (window)
		PostMessage(window, WM_CLOSE, 0, 0);

	if (WaitForSingleObject(process->hProcess, 10000) == WAIT_TIMEOUT)
		{
		closed = FALSE;
		TerminateProcess(process->hProcess, 1);
		WaitForSingleObject(process->hProcess, 3000);
		}

	CloseHandle(process->hThread);
	CloseHandle(process->hProcess);
	return closed;
	}


int main(int argc, char *argv[])
	{
	char browser_path[MAX_PATH];
	char command_line[2048];
	char last_title[512];
	DWORD timeout_seconds = 15;
	DWORD settle_seconds = 0;
	DWORD started;
	DWORD title_matched = 0;
	STARTUPINFO startup_info;
	PROCESS_INFORMATION process;
	HWND browser_window = NULL;

	if (argc < 3 || argc > 5)
		{
		fprintf(stderr,
			"Usage: ncompass-test <url> <expected-title> "
			"[timeout-seconds] [settle-seconds]\n");
		return 2;
		}

	if (argc >= 4)
		{
		char *end;
		unsigned long value = strtoul(argv[3], &end, 10);
		if (argv[3][0] == '\0' || *end != '\0' || value == 0)
			{
			fprintf(stderr, "timeout-seconds must be a positive integer\n");
			return 2;
			}
		timeout_seconds = value;
		}

	if (argc == 5)
		{
		char *end;
		unsigned long value = strtoul(argv[4], &end, 10);
		if (argv[4][0] == '\0' || *end != '\0')
			{
			fprintf(stderr, "settle-seconds must be a non-negative integer\n");
			return 2;
			}
		settle_seconds = value;
		}

	if (!GetBrowserPath(browser_path) ||
		GetFileAttributes(browser_path) == INVALID_FILE_ATTRIBUTES)
		{
		fprintf(stderr,
			"Could not find ncompass.exe beside the test executable\n");
		return 2;
		}

	if (strlen(browser_path) + strlen(argv[1]) + 5 >= sizeof(command_line))
		{
		fprintf(stderr, "Browser path or URL is too long\n");
		return 2;
		}

	sprintf(command_line, "\"%s\" %s", browser_path, argv[1]);
	memset(&startup_info, 0, sizeof(startup_info));
	startup_info.cb = sizeof(startup_info);
	memset(&process, 0, sizeof(process));

	if (!CreateProcess(browser_path, command_line, NULL, NULL, FALSE, 0,
		NULL, NULL, &startup_info, &process))
		{
		fprintf(stderr, "Could not start ncompass.exe (error %lu)\n",
			GetLastError());
		return 2;
		}

	started = GetTickCount();
	last_title[0] = '\0';

	while (GetTickCount() - started < timeout_seconds * 1000)
		{
		WINDOW_SEARCH search;

		if (WaitForSingleObject(process.hProcess, 0) == WAIT_OBJECT_0)
			{
			fprintf(stderr,
				"FAIL: ncompass.exe exited before loading completed\n");
			CloseHandle(process.hThread);
			CloseHandle(process.hProcess);
			return 1;
			}

		search.process_id = process.dwProcessId;
		search.window = NULL;
		search.assertion_window = NULL;
		EnumWindows(FindProcessWindow, (LPARAM)&search);
		if (search.assertion_window)
			{
			HWND abort_button = GetDlgItem(search.assertion_window, IDABORT);
			fprintf(stderr, "FAIL: ncompass.exe raised a Debug assertion\n");
			EnumChildWindows(search.assertion_window, PrintWindowText, 0);
			if (abort_button)
				SendMessage(abort_button, BM_CLICK, 0, 0);
			else
				PostMessage(search.assertion_window, WM_COMMAND,
					MAKEWPARAM(IDABORT, BN_CLICKED), 0);
			WaitForSingleObject(process.hProcess, 3000);
			CloseHandle(process.hThread);
			CloseHandle(process.hProcess);
			return 1;
			}

		if (search.window)
			{
			browser_window = search.window;
			GetBrowserTitle(search.window, last_title);
			if (strstr(last_title, argv[2]) != NULL)
				{
				if (title_matched == 0)
					title_matched = GetTickCount();
				if (GetTickCount() - title_matched >= settle_seconds * 1000)
					{
					if (CloseBrowser(&process, browser_window))
						{
						printf("PASS: %s\nTitle: %s\n", argv[1], last_title);
						return 0;
						}

					fprintf(stderr,
						"FAIL: ncompass.exe did not close after loading\n");
					return 1;
					}
				}
			}

		Sleep(100);
		}

	fprintf(stderr,
		"FAIL: expected title was not rendered within %lu seconds\n"
		"Expected: %s\nLast title: %s\n",
		timeout_seconds, argv[2], last_title);
	if (!CloseBrowser(&process, browser_window))
		fprintf(stderr, "FAIL: ncompass.exe did not close after loading\n");
	return 1;
	}

#include <windows.h>

#include <iostream>
#include <string>
#include <vector>


static std::wstring GetErrorMessage(DWORD error)
	{
	LPWSTR buffer = NULL;
	DWORD length = FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, error, 0, reinterpret_cast<LPWSTR>(&buffer), 0, NULL);
	std::wstring message;
	if (length && buffer)
		{
		message.assign(buffer, length);
		while (!message.empty() &&
			(message.back() == L'\r' || message.back() == L'\n'))
			message.pop_back();
		}
	if (buffer)
		LocalFree(buffer);
	return message;
	}


static std::wstring QuoteArgument(const std::wstring& argument)
	{
	if (argument.empty())
		return L"\"\"";

	if (argument.find_first_of(L" \t\n\v\"") == std::wstring::npos)
		return argument;

	std::wstring quoted = L"\"";
	size_t backslashes = 0;
	for (wchar_t c : argument)
		{
		if (c == L'\\')
			{
			backslashes++;
			continue;
			}

		if (c == L'"')
			quoted.append(backslashes * 2 + 1, L'\\');
		else
			quoted.append(backslashes, L'\\');

		backslashes = 0;
		quoted.push_back(c);
		}

	quoted.append(backslashes * 2, L'\\');
	quoted.push_back(L'"');
	return quoted;
	}


static void PrintUsage()
	{
	std::wcerr <<
		L"Usage: ncompass-job.exe [--cwd directory] [--] command [arguments...]\n";
	}


int wmain(int argc, wchar_t *argv[])
	{
	int command_index = 1;
	LPCWSTR working_directory = NULL;
	std::wstring working_directory_storage;

	if (command_index < argc &&
		wcscmp(argv[command_index], L"--cwd") == 0)
		{
		if (command_index + 2 >= argc)
			{
			PrintUsage();
			return 2;
			}
		working_directory_storage = argv[command_index + 1];
		working_directory = working_directory_storage.c_str();
		command_index += 2;
		}

	if (command_index < argc && wcscmp(argv[command_index], L"--") == 0)
		command_index++;

	if (command_index >= argc)
		{
		PrintUsage();
		return 2;
		}

	std::wstring command_line;
	for (int i = command_index; i < argc; i++)
		{
		if (!command_line.empty())
			command_line.push_back(L' ');
		command_line += QuoteArgument(argv[i]);
		}

	HANDLE job = CreateJobObjectW(NULL, NULL);
	if (!job)
		{
		DWORD error = GetLastError();
		std::wcerr << L"CreateJobObject failed: " <<
			GetErrorMessage(error) << L" (" << error << L")\n";
		return 1;
		}

	JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits = {};
	limits.BasicLimitInformation.LimitFlags =
		JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation,
		&limits, sizeof(limits)))
		{
		DWORD error = GetLastError();
		std::wcerr << L"SetInformationJobObject failed: " <<
			GetErrorMessage(error) << L" (" << error << L")\n";
		CloseHandle(job);
		return 1;
		}

	std::vector<wchar_t> mutable_command(
		command_line.begin(), command_line.end());
	mutable_command.push_back(L'\0');

	STARTUPINFOW startup = {};
	startup.cb = sizeof(startup);
	PROCESS_INFORMATION process = {};
	if (!CreateProcessW(NULL, mutable_command.data(), NULL, NULL, FALSE,
		CREATE_SUSPENDED, NULL, working_directory, &startup, &process))
		{
		DWORD error = GetLastError();
		std::wcerr << L"CreateProcess failed: " <<
			GetErrorMessage(error) << L" (" << error << L")\n";
		CloseHandle(job);
		return 1;
		}

	if (!AssignProcessToJobObject(job, process.hProcess))
		{
		DWORD error = GetLastError();
		std::wcerr << L"AssignProcessToJobObject failed: " <<
			GetErrorMessage(error) << L" (" << error << L")\n";
		TerminateProcess(process.hProcess, 1);
		CloseHandle(process.hThread);
		CloseHandle(process.hProcess);
		CloseHandle(job);
		return 1;
		}

	if (ResumeThread(process.hThread) == static_cast<DWORD>(-1))
		{
		DWORD error = GetLastError();
		std::wcerr << L"ResumeThread failed: " <<
			GetErrorMessage(error) << L" (" << error << L")\n";
		TerminateJobObject(job, 1);
		CloseHandle(process.hThread);
		CloseHandle(process.hProcess);
		CloseHandle(job);
		return 1;
		}

	CloseHandle(process.hThread);
	WaitForSingleObject(process.hProcess, INFINITE);

	DWORD exit_code = 1;
	if (!GetExitCodeProcess(process.hProcess, &exit_code))
		{
		DWORD error = GetLastError();
		std::wcerr << L"GetExitCodeProcess failed: " <<
			GetErrorMessage(error) << L" (" << error << L")\n";
		}

	CloseHandle(process.hProcess);
	CloseHandle(job);
	return static_cast<int>(exit_code);
	}

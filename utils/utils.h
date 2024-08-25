#pragma once
#include <Windows.h>
#include <string>
#include <TlHelp32.h>

namespace utils
{
	inline HWND hwndout;
	inline BOOL EnumWindowProcMy(HWND input, LPARAM lParam)
	{

		DWORD lpdwProcessId;
		GetWindowThreadProcessId(input, &lpdwProcessId);
		if (lpdwProcessId == lParam)
		{
			hwndout = input;
			return FALSE;
		}
		return true;
	}
	inline HWND get_hwnd_of_process_id(int target_process_id)
	{
		EnumWindows(EnumWindowProcMy, target_process_id);
		return hwndout;
	}
	inline std::string replace(std::string subject, std::string search, std::string replace) {
		size_t pos = 0;

		while ((pos = subject.find(search, pos)) != std::string::npos)
		{
			subject.replace(pos, search.length(), replace);
			pos += replace.length();
		}

		return subject;
	}
	inline int get_pid_from_name(const char* name)
	{
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(PROCESSENTRY32);
		Process32First(snapshot, &entry);
		do
		{
			if (strcmp(entry.szExeFile, name) == 0)
			{
				return entry.th32ProcessID;
			}

		} while (Process32Next(snapshot, &entry));

		return 0; // if not found
	}
}

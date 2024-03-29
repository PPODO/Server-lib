#pragma once
#include <iostream>
#include <Windows.h>
#include <DbgHelp.h>

class CMiniDump {
	typedef bool (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD ProcessID, HANDLE hFile, MINIDUMP_TYPE DumpType, const PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, const PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, const PMINIDUMP_CALLBACK_INFORMATION CallBackParam);
private:
	LPTOP_LEVEL_EXCEPTION_FILTER m_PrevExceptionFilter;

private:
	static LONG WINAPI UnHandledExceptionFilter(_EXCEPTION_POINTERS* ExceptionInfo) {
		HMODULE DllHandle = LoadLibrary(L"DBGHELP.DLL");

		if (DllHandle) {
			MINIDUMPWRITEDUMP Dump = (MINIDUMPWRITEDUMP)GetProcAddress(DllHandle, "MiniDumpWriteDump");

			if (Dump) {
				TCHAR DumpPath[MAX_PATH] = { L"\0" };
				SYSTEMTIME SysTime;

				GetLocalTime(&SysTime);

				swprintf_s(DumpPath, MAX_PATH, L"%d-%d-%d %d_%d_%d.dmp", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond);

				HANDLE FileHandle = CreateFile(DumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			
				if (FileHandle != INVALID_HANDLE_VALUE) {
					_MINIDUMP_EXCEPTION_INFORMATION MiniDumpExceptionInfo;

					MiniDumpExceptionInfo.ThreadId = GetCurrentThreadId();
					MiniDumpExceptionInfo.ExceptionPointers = ExceptionInfo;
					MiniDumpExceptionInfo.ClientPointers = NULL;

					bool Success = Dump(GetCurrentProcess(), GetCurrentProcessId(), FileHandle, MiniDumpNormal, &MiniDumpExceptionInfo, nullptr, nullptr);
					if (Success) {
						CloseHandle(FileHandle);
						return EXCEPTION_EXECUTE_HANDLER;
					}
				}
				CloseHandle(FileHandle);
			}
		}
		return EXCEPTION_CONTINUE_SEARCH;
	}

public:
	CMiniDump() {
		SetErrorMode(SEM_FAILCRITICALERRORS);

		m_PrevExceptionFilter = SetUnhandledExceptionFilter(CMiniDump::UnHandledExceptionFilter);
	}

	~CMiniDump() {
		SetUnhandledExceptionFilter(m_PrevExceptionFilter);
	}
};

static CMiniDump MiniDump;
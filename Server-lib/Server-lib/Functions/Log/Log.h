#pragma once
#include <iostream>
#include <fstream>
#include <Windows.h>
#include "../ThreadSync/ThreadSync.h"

#define MAX_BUFFER_LENGTH 256
#define MAX_DATETIME_LENGTH 32

class CLog : public CMultiThreadSync<CLog> {
private:
	inline static bool Write(bool bIsUnicode, const void* LogData) {
		SYSTEMTIME SysTime;

		GetLocalTime(&SysTime);
		if (bIsUnicode) {
			LPCTSTR Log = static_cast<LPCTSTR>(LogData);
			TCHAR CurrentDate[MAX_DATETIME_LENGTH] = { L"\0" };
			TCHAR CurrentFileName[MAX_PATH] = { L"\0" };
			TCHAR DebugLog[MAX_BUFFER_LENGTH] = { L"\0" };

			swprintf_s(CurrentDate, MAX_DATETIME_LENGTH, L"%d-%d-%d %d:%d:%d", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
			swprintf_s(CurrentFileName, MAX_PATH, L"LOG_U_%d-%d-%d %d.log", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour);

			std::wfstream FileStream(CurrentFileName, std::ios::app);
			if (!FileStream.is_open()) {
				return false;
			}

			FileStream << L'[' << CurrentDate << L"] " << reinterpret_cast<const char*>(Log) << '\n';
			swprintf_s(DebugLog, MAX_BUFFER_LENGTH, L"[%s] %S\n", CurrentDate, reinterpret_cast<const char*>(Log));

			FileStream.close();

			OutputDebugString(DebugLog);
			std::wcout << DebugLog;
			return true;
		}
		else {
			LPCSTR Log = static_cast<LPCSTR>(LogData);
			CHAR CurrentDate[MAX_DATETIME_LENGTH] = { "\0" };
			CHAR CurrentFileName[MAX_PATH] = { "\0" };
			CHAR DebugLog[MAX_BUFFER_LENGTH] = { "\0" };

			sprintf_s(CurrentDate, MAX_DATETIME_LENGTH, "%d-%d-%d %d:%d:%d", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
			sprintf_s(CurrentFileName, MAX_PATH, "LOG_M_%d-%d-%d %d.log", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour);

			std::fstream FileStream(CurrentFileName, std::ios::app);
			if (!FileStream.is_open()) {
				return false;
			}

			FileStream << '[' << CurrentDate << "] " << Log << '\n';
			sprintf_s(DebugLog, MAX_BUFFER_LENGTH, "[%s] %s\n", CurrentDate, Log);

			FileStream.close();

			OutputDebugStringA(DebugLog);
			std::cout << DebugLog;
			return true;
		}
		return false;
	}

public:
	static bool WriteLog(LPCTSTR Data, ...) {
		CThreadSync Sync;

		TCHAR Log[MAX_BUFFER_LENGTH] = { L"\0" };

		va_list ArgcList;
		va_start(ArgcList, Data);
		vswprintf_s(Log, Data, ArgcList);
		va_end(ArgcList);

		return Write(true, Log);
	}

	static bool WriteLog(LPCSTR Data, ...) {
		CThreadSync Sync;

		CHAR Log[MAX_BUFFER_LENGTH] = { "\0" };

		va_list ArgcList;
		va_start(ArgcList, Data);
		vsprintf_s(Log, Data, ArgcList);
		va_end(ArgcList);

		return Write(false, Log);
	}

};
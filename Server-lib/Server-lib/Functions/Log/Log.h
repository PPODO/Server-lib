#pragma once
#include <iostream>
#include <fstream>
#include <Windows.h>

#define MAX_BUFFER_LENGTH 256
#define MAX_DATETIME_LENGTH 32

class CLog {
private:
	inline static bool Write(bool bIsUnicode, const void* Data) {
		SYSTEMTIME SysTime;
		va_list ArgcList;

		GetLocalTime(&SysTime);
		if (bIsUnicode) {
			LPCTSTR ArgcData = static_cast<LPCTSTR>(Data);
			TCHAR CurrentDate[MAX_DATETIME_LENGTH] = { L"\0" };
			TCHAR CurrentFileName[MAX_PATH] = { L"\0" };
			TCHAR DebugLog[MAX_BUFFER_LENGTH] = { L"\0" };
			TCHAR Log[MAX_BUFFER_LENGTH] = { L"\0" };

			va_start(ArgcList, ArgcData);
			vswprintf_s(Log, ArgcData, ArgcList);
			va_end(ArgcList);

			swprintf_s(CurrentDate, MAX_DATETIME_LENGTH, L"%d-%d-%d %d:%d:%d", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
			swprintf_s(CurrentFileName, MAX_PATH, L"LOG_%d-%d-%d %d.log", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour);

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
			LPCSTR ArgcData = static_cast<LPCSTR>(Data);
			CHAR CurrentDate[MAX_DATETIME_LENGTH] = { "\0" };
			CHAR CurrentFileName[MAX_PATH] = { "\0" };
			CHAR DebugLog[MAX_BUFFER_LENGTH] = { "\0" };
			CHAR Log[MAX_BUFFER_LENGTH] = { "\0" };

			va_start(ArgcList, ArgcData);
			vsprintf_s(Log, ArgcData, ArgcList);
			va_end(ArgcList);

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
		return Write(true, Data);
	}

	static bool WriteLog(LPCSTR Data, ...) {
		return Write(false, Data);
	}

};
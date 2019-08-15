#pragma once
#include "../../Functions/Log/Log.h"
#include "../../Functions/SocketAddress/SocketAddress.h"
#include <vector>
#include <thread>

class CIOCP {
private:
	WSADATA m_WinSockData;

private:
	HANDLE m_hIOCP;
	HANDLE m_hWaitForInitalization;

private:
	std::vector<std::thread> m_WorkerThreads;

private:
	bool WorkerThreadProcess();

protected:
	virtual bool OnIOAccept(void* Object) = 0;
	virtual bool OnIODisconnect(void* Object) = 0;
	virtual bool OnIORead(void* Object, const USHORT& RecvBytes) = 0;
	virtual bool OnIOWrite(void* Object) = 0;

protected:
	void CreateWorkerThread();

protected:
	inline bool RegisterIOCompletionPort(const SOCKET& Socket, const ULONG_PTR& CompletionPort) {
		if (Socket == INVALID_SOCKET || m_hIOCP == INVALID_HANDLE_VALUE) {
			CLog::WriteLog(L"Register IO Completion Port : Invalid Socket or Invalid Handle Value!");
			return false;
		}

		if ((m_hIOCP = CreateIoCompletionPort(reinterpret_cast<HANDLE>(Socket), m_hIOCP, CompletionPort, 0)) == NULL) {
			CLog::WriteLog(L"Register IO Completion Port : Failed To Register Completion Port!");
			return false;
		}
		return true;
	}

public:
	CIOCP();
	CIOCP(const CIOCP& iocp) = delete;

public:
	virtual bool Initialize();
	virtual bool Destroy();

};
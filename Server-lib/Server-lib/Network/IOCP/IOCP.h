#pragma once
#include "../PacketSession/PacketSession.h"
#include <vector>
#include <thread>

class CIOCP {
private:
	WSADATA m_WinSockData;

private:
	HANDLE m_hIOCP;
	HANDLE m_hWaitForInitialize;

private:
	std::vector<std::thread> m_WorkerThread;

private:
	bool CreateWorkerThread();
	bool ProcessWorkerThread();

protected:
	virtual bool OnIOConnect(void* const Object) = 0;
	virtual bool OnIODisconnect(void* const Object) = 0;
	virtual bool OnIORead(void* const Object, const uint16_t& RecvBytes) = 0;
	virtual bool OnIOWrite(void* const Object) = 0;

protected:
	inline bool RegisterIOCompletionPort(const SOCKET& Socket, const ULONG_PTR& CompletionKey) {
		if (Socket == INVALID_SOCKET || m_hIOCP == NULL) {
			CLog::WriteLog(L"Register IO Completion Port : Invalid Socket or Invalid Handle Value!");
			return false;
		}

		if ((m_hIOCP = CreateIoCompletionPort(reinterpret_cast<HANDLE>(Socket), m_hIOCP, CompletionKey, 0)) == NULL) {
			CLog::WriteLog(L"Register IO Completion Port : Failed To Register Completion Port!");
			return false;
		}
		return true;
	}

protected:
	inline HANDLE GetIOCPHandle() const { return m_hIOCP; }

public:
	CIOCP();

public:
	virtual bool Initialize();
	virtual bool Destroy();

};
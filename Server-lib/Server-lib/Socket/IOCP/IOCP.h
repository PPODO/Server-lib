#pragma once
#include "../PacketBase/PacketBase.h"
#include <vector>

class CIOCP {
private:
	WSADATA m_WinSockData;
	HANDLE m_hIOCP;

private:
	std::vector<std::thread> m_WorkerThreads;

public:
	CIOCP();
	virtual ~CIOCP();

public:
	virtual bool Initialize();
	bool RegisterIOCompletionKey(const SOCKET& Socket, const ULONG_PTR& CompletionKey);
	void WorkerThreadCallback();
	virtual bool Destroy();

};
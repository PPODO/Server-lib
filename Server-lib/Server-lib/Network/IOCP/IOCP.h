#pragma once
#include "../PacketSession/PacketSession.h"
#include <vector>
#include <thread>

class CIOCP {
private:
	WSADATA m_WinSockData;
	HANDLE m_hIOCP;

protected:
	HANDLE m_hWaitInitializeThreadHandle;

public:
	CIOCP();
	~CIOCP();

private:
	std::vector<std::thread> m_WorkerThreads;

private:
	void WorkerThreadCallback();

private:
	bool CreateWorkerThreads();

protected:
	bool RegisterCompletionPort(const SOCKET& Socket, const ULONG_PTR& CompletionKey);

protected:
/*	virtual bool OnIOConnect() = 0;
	virtual bool OnIOAccept() = 0;
	virtual bool OnIORead() = 0;
	virtual bool OnIOWrite() = 0;
	virtual bool OnIODisconnect() = 0;*/

public:
	virtual bool Initialize();
	virtual bool Destroy();

};
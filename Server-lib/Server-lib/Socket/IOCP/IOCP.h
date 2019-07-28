#pragma once
#include "../PacketBase/PacketBase.h"
#include <vector>

class CIOCP {
private:
	WSADATA m_WinSockData;
	HANDLE m_hIOCP;

private:
	std::vector<std::thread> m_WorkerThreads;

protected:
	virtual void OnIOConnect(void* Object) = 0;
	virtual void OnIODisconnect(void* Object) = 0;
	virtual void OnIOReceive(void* Object, const UINT16& RecvBytes) = 0;
	virtual void OnIOWrite(void* Object) = 0;

protected:
	bool RegisterIOCompletionKey(const SOCKET& Socket, const ULONG_PTR& CompletionKey);

public:
	CIOCP();
	virtual ~CIOCP();

public:
	virtual bool Initialize();
	virtual bool Destroy();

public:
	void WorkerThreadCallback();

};
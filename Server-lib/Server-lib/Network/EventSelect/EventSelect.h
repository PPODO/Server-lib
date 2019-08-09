#pragma once
#include "../PacketSession/PacketSession.h"
#include <thread>

class CEventSelect {
private:
	const SOCKET& m_ListenSocket;

private:
	HANDLE m_hSelectHandle;
	HANDLE m_hDestroyHandle;
	HANDLE m_hWaitInitializeThreadHandle;

private:
	std::thread m_EventSelectThreadHandle;

private:
	void EventSelectThreadCallback();

protected:
	virtual bool OnIOConnect() = 0;
	virtual bool OnIOAccpet() = 0;
	virtual bool OnIORead() = 0;
	virtual bool OnIOWrite() = 0;
	virtual bool OnIODisconnect() = 0;

public:
	CEventSelect(const SOCKET& ListenSocket);

public:
	bool Initialize();
	bool Destroy();

};
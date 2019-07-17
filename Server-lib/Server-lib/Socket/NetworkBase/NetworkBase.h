#pragma once
#include "../../Functions/SocketAddress/SocketAddress.h"
#include "../../Functions/ThreadSync/ThreadSync.h"
#include "../../Functions/SocketUtil/SocketUtil.h"
#include "TCP_IP/TCPIPSocket.h"

class CNetworkBase : CMultiThreadSync<CNetworkBase> {
private:
	OVERLAPPED_EX m_AcceptOveralapped;
	OVERLAPPED_EX m_ReciveDataOverlapped;
	OVERLAPPED_EX m_SendDataOverlapped;

private:
	CTCPIPSocket m_TCPSocket;

public:
	CNetworkBase();
	~CNetworkBase();

public:
	virtual void Initialize();

	virtual void Destroy();

public:


};
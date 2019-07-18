#pragma once
#include "../../Functions/SocketAddress/SocketAddress.h"
#include "../../Functions/ThreadSync/ThreadSync.h"
#include "../../Functions/SocketUtil/SocketUtil.h"
#include "TCP_IP/TCPIPSocket.h"
#include "UDP_IP/UDPIPSocket.h"
#include <thread>

class CNetworkBase : public CMultiThreadSync<CNetworkBase> {
private:
	OVERLAPPED_EX m_AcceptOveralapped;
	OVERLAPPED_EX m_ReciveDataOverlapped;
	OVERLAPPED_EX m_SendDataOverlapped;

private:
	CTCPIPSocket m_TCPSocket;
	CUDPIPSocket m_UDPSocket;

private:
	std::thread m_ReliableUDPThread;

public:
	CNetworkBase();
	virtual ~CNetworkBase();

public:
	virtual void Initialize();
	virtual void Clear();

public:
	virtual void OnIOAccept() = 0;
	virtual void OnIODisconnect() = 0;
	virtual void OnIORecive() = 0;
	virtual void OnIOSend() = 0;

public:
	// For Server
	bool InitializeSocket(const IPPROTO& ProtocolType, const CSocketAddress& BindAddress);

	// For Client
	bool Accept(const SOCKET& ListenSocket);

public:
	CTCPIPSocket* const GetTCPIPSocket() { return &m_TCPSocket; }

};
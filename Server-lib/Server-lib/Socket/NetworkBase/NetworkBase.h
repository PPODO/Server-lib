#pragma once
#include "../../Functions/SocketAddress/SocketAddress.h"
#include "../../Functions/ThreadSync/ThreadSync.h"
#include "../../Functions/SocketUtil/SocketUtil.h"
#include "TCP_IP/TCPIPSocket.h"
#include "UDP_IP/UDPIPSocket.h"
#include <thread>

class CNetworkBase : public CMultiThreadSync<CNetworkBase> {
public:
	OVERLAPPED_EX m_AcceptOveralapped;
	OVERLAPPED_EX m_ReceiveDataOverlapped;
	OVERLAPPED_EX m_SendDataOverlapped;

public:
	CTCPIPSocket m_TCPSocket;
	CUDPIPSocket m_UDPSocket;

private:
	std::thread m_ReliableUDPThread;

public:
	CNetworkBase();
	virtual ~CNetworkBase();

protected:
	bool ReadIOCP(CHAR* InData, const UINT16& DataLength);
	bool ReadSelect(CHAR* InData, UINT16& RecvLength);

public:
	virtual void Initialize();
	virtual bool Initialize(const CNetworkBase* const ListenSocket);
	virtual void Clear();

public:
	virtual bool OnIOConnect();
	virtual bool OnIODisconnect() = 0;

public:
	bool InitializeSocket(const bool& bIsClient, const IPPROTO& ProtocolType, const CSocketAddress& Address);

public:
	SOCKET GetSocket(const IPPROTO& ProtocolType) { return (ProtocolType == IPPROTO_TCP ? m_TCPSocket.GetSocket() : (ProtocolType == IPPROTO_UDP ? m_UDPSocket.GetSocket() : INVALID_SOCKET)); }

};
#pragma once
#include "../../../Functions/SocketUtil/SocketUtil.h"
#include "../../../Functions/ThreadSync/ThreadSync.h"
#include "TCPIP/TCPIPSocket.h"
#include "UDPIP/UDPIPSocket.h"

class CNetworkSession : CMultiThreadSync<CNetworkSession> {
private:
	// Only use when IOCP
	CHAR m_ReceiveBuffer[MAX_RECEIVE_BUFFER_LENGTH];

private:
	std::unique_ptr<CTCPIPSocket> m_TCPSocket;
	std::unique_ptr<CUDPIPSocket> m_UDPSocket;

private:
	OVERLAPPED_EX m_AcceptOverlapped;
	OVERLAPPED_EX m_RecvOverlapped;
	OVERLAPPED_EX m_SendOverlapped;

public:
	CNetworkSession();
	CNetworkSession(CNetworkSession& NetworkSession) = delete;

public:
	// TCP
	inline bool Initialize() {
		if (m_TCPSocket) {
			return m_TCPSocket->Initialize();
		}
		return false;
	}
	inline bool Listen(const CSocketAddress& BindAddress, const USHORT& MaxBackLogCount = SOMAXCONN) {
		if (m_TCPSocket) {
			return m_TCPSocket->Listen(BindAddress, MaxBackLogCount);
		}
		return false;
	}
	inline bool Accept(const SOCKET& ListenSocket) {
		if (m_TCPSocket) {
			return m_TCPSocket->Accept(ListenSocket, m_AcceptOverlapped);
		}
		return false;
	}

public:
	// UDP
	inline bool Initialize(const USHORT& Port) {
		if (m_UDPSocket) {
			return m_UDPSocket->Initialize(Port);
		}
		return false;
	}

public:
	// Only use when IOCP
	bool GetBufferData(CHAR* OutBuffer, const USHORT& Length);

public:
	// TCP
	inline bool InitializeReceiveForIOCP() {
		CThreadSync Sync;
		if (m_TCPSocket) {
			return m_TCPSocket->InitializeReceiveForIOCP(m_ReceiveBuffer, m_RecvOverlapped);
		}
		return false;
	}
	inline bool ReceiveForEventSelect(CHAR* OutBuffer, USHORT& ReceiveLength) {
		CThreadSync Sync;
		if (m_TCPSocket) {
			return m_TCPSocket->ReceiveForEventSelect(OutBuffer, ReceiveLength, m_RecvOverlapped);
		}
		return false;
	}
	inline bool Write(const CHAR* SendData, const USHORT& DataLength) {
		CThreadSync Sync;
		if (m_TCPSocket) {
			return m_TCPSocket->Write(SendData, DataLength, m_SendOverlapped);
		}
		return false;
	}

public:
	// UDP
	inline bool InitializeReceiveFromForIOCP() {
		if (m_UDPSocket) {
			m_UDPSocket->InitializeReceiveFromForIOCP(m_ReceiveBuffer, m_AcceptOverlapped);
		}

		return false;
	}

};
#pragma once
#include "../../../Functions/ThreadSync/ThreadSync.h"
#include "TCPIP/TCPIPSocket.h"

class CNetworkSession : public CMultiThreadSync<CNetworkSession> {
public:
	CTCPIPSocket m_TCPSocket;

private:
	OVERLAPPED_EX m_AcceptOverlapped;
	OVERLAPPED_EX m_RecvOverlapped;
	OVERLAPPED_EX m_SendOverlapped;

protected:
	inline bool CopyIOCPBuffer(CHAR* OutDataBuffer, const USHORT& DataLength) {
		CThreadSync Sync;

		m_TCPSocket.CopyIOCPBuffer(OutDataBuffer, DataLength);
	}

public:
	virtual bool Initialize();
	virtual bool Destroy();

public:
	inline bool Listen(const CSocketAddress& BindAddress, const USHORT& BackLogCount = SOMAXCONN) { return m_TCPSocket.Listen(BindAddress, BackLogCount); }
	inline bool Connect(const CSocketAddress& ConnectionAddress) { return m_TCPSocket.Connect(ConnectionAddress); }
	inline bool Accept(const SOCKET& ListenSocket) { return m_TCPSocket.Accept(ListenSocket, m_AcceptOverlapped); }

public:
	inline bool ReadForIOCP() {
		CThreadSync Sync;

		m_TCPSocket.ReadForIOCP(m_RecvOverlapped);
	}

	inline bool Write(const CHAR* OutDataBuffer, const USHORT& DataLength) {
		CThreadSync Sync;
		 
		m_TCPSocket.Write(OutDataBuffer, DataLength, m_SendOverlapped);
	}

public:
	inline SOCKET GetSocket() const { return m_TCPSocket.GetSocket(); }

};
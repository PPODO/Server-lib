#pragma once
#include "../../../../Functions/SocketUtil/SocketUtil.h"

class CTCPIPSocket {
private:
	SOCKET m_Socket;

private:
	CHAR m_IOCPReceiveBuffer[MAX_RECEIVE_BUFFER_LENGTH];

public:
	CTCPIPSocket();

public:
	bool Initialize();
	bool Listen(const CSocketAddress& BindAddress, const USHORT& BackLogCount);
	bool Connect(const CSocketAddress& ConnectionAddress);
	bool Accept(const SOCKET& ListenSocket, struct OVERLAPPED_EX& AcceptOverlapped);
	bool Destroy();

public:
	bool ReadForIOCP(struct OVERLAPPED_EX& RecvOverlapped);
	bool CopyIOCPBuffer(CHAR* OutDataBuffer, const USHORT& DataLength);
	bool Write(const CHAR* OutDataBuffer, const USHORT& DataLength, struct OVERLAPPED_EX& SendOverlapped);

public:
	inline SOCKET GetSocket() const { return m_Socket; }

};
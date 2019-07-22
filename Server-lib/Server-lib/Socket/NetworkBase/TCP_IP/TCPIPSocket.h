#pragma once
#pragma comment(lib, "mswsock.lib")
#include "../../../Functions/SocketAddress/SocketAddress.h"
#include "../../../Functions/SocketUtil/SocketUtil.h"
#include <MSWSock.h>

class CTCPIPSocket {
private:
	SOCKET m_Socket;

private:
	CHAR m_ReciveBuffer[MAX_RECIVE_BUFFER_LENGTH];

public:
	CTCPIPSocket();
	~CTCPIPSocket();

public:
	bool BindTCP();
	bool Listen(const CSocketAddress& ListenAddress, const UINT16& BackLogCount);
	bool Connect(const CSocketAddress& ConnectionAddress);
	bool Accept(const CTCPIPSocket& ListenSocket, struct OVERLAPPED_EX& AcceptOverlapped);
	bool Shutdown();

public:
	bool InitializeRecvBuffer_IOCP(struct OVERLAPPED_EX& RecvOverlapped);
	bool CopyRecvBuffer_IOCP(CHAR* InData, const UINT16& DataLength);
	bool ReadRecvBuffer_Select(CHAR* InData, UINT16& RecvLength, struct OVERLAPPED_EX& RecvOverlapped);
	bool Write(const CHAR* OutData, const UINT16& DataLength, struct OVERLAPPED_EX& SendOverlapped);

public:
	SOCKET GetSocket() const { return m_Socket; }

};
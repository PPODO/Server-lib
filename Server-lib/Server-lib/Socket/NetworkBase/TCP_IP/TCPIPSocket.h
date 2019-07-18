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
	bool Connect(const CSocketAddress& ConnectionAddress);
	bool Listen(const CSocketAddress& BindAddress, const INT& BackLogCount);
	bool Accept(const SOCKET& ListenSocket, struct OVERLAPPED_EX& AcceptOverlapped);
	bool Read(struct OVERLAPPED_EX& ReciveOverlapped);
	bool Write(const CHAR* Data, const UINT16& DataLength, struct OVERLAPPED_EX& SendOverlapped);
	void Shutdown();

};
#pragma once
#include "../../../Functions/SocketAddress/SocketAddress.h"
#include "../../../Functions/SocketUtil/SocketUtil.h"

class CTCPIPSocket {
private:
	SOCKET m_Socket;

public:
	CTCPIPSocket();
	~CTCPIPSocket();

public:
	bool Connect(const CSocketAddress& ConnectionAddress);
	bool Listen(const CSocketAddress& BindAddress, const INT& BackLogCount);
	bool Accept(const SOCKET& ListenSocket, struct OVERLAPPED_EX& AcceptOverlapped);
	void Shutdown();

};
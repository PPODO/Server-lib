#pragma once
#include "../../../Functions/SocketAddress/SocketAddress.h"
#include "../../../Functions/SocketUtil/SocketUtil.h"

class CUDPIPSocket {
private:
	SOCKET m_Socket;

private:
	CSocketAddress m_RecivedAddress;

private:
	CHAR m_ReciveBuffer[MAX_RECIVE_BUFFER_LENGTH];

public:
	CUDPIPSocket();
	~CUDPIPSocket();

public:
	bool BindUDP(const CSocketAddress& BindAddress);
	bool SendTo(struct OVERLAPPED_EX& SendOverlapped);
	bool RecvFrom(struct OVERLAPPED_EX& ReciveOverlapped);
	void Shutdown();

public:
	SOCKET GetSocket() const { return m_Socket; }

};
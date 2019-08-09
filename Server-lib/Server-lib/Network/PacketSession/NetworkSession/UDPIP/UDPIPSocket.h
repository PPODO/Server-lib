#pragma once
#include "../../../../Functions/SocketAddress/SocketAddress.h"

class CUDPIPSocket {
private:
	SOCKET m_Socket;
	CSocketAddress m_RemoteAddress;

public:
	bool Initialize(const USHORT& Port);
	bool Destroy();

public:
	bool InitializeReceiveFromForIOCP(CHAR* RecvData, struct OVERLAPPED_EX& RecvOverlapped);
	//bool ReceiveFromForEventSelect();

public:
	SOCKET GetSocket() const { return m_Socket; }

};
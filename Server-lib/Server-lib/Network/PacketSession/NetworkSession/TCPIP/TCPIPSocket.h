#pragma once
#include "../../../../Functions/SocketAddress/SocketAddress.h"

class CTCPIPSocket {
private:
	SOCKET m_Socket;
	// ���� ������ ���۰� �־�� �ϳ�?
	// NetworkSession�� �־ ��� ���� �� ����.

public:
	CTCPIPSocket();

public:
	bool Initialize();
	bool Destroy();
	bool Listen(const CSocketAddress& BindAddress, const USHORT& MaxBackLogCount);
	bool Accept(const SOCKET& ListenSocket, struct OVERLAPPED_EX& AcceptOverlapped);

public:
	bool InitializeReceiveForIOCP(CHAR* RecvData, struct OVERLAPPED_EX& RecvOverlapped);
	bool ReceiveForEventSelect(CHAR* RecvData, USHORT& DataLength, struct OVERLAPPED_EX& RecvOverlapped);
	bool Write(const CHAR* SendData, const USHORT& DataLength, struct OVERLAPPED_EX& SendOverlapped);

public:
	SOCKET GetSocket() const { return m_Socket; }

};
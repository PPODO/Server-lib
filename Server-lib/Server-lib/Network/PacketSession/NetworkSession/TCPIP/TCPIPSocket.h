#pragma once
#include "../../../../Functions/SocketAddress/SocketAddress.h"

class CTCPIPSocket {
private:
	SOCKET m_Socket;
	// ���� ������ ���۰� �־�� �ϳ�?
	// NetworkSession�� �־ ��� ���� �� ����.

public:
	bool Initialize();
	bool Listen(const CSocketAddress& BindAddress, const USHORT& MaxBackLogCount);
	bool Accept(const SOCKET& ListenSocket, struct OVERLAPPED_EX& AcceptOverlapped);

public:
	inline bool InitializeReceiveForIOCP(CHAR* RecvData, struct OVERLAPPED_EX& RecvOverlapped);
	inline bool ReceiveForEventSelect(CHAR* RecvData, USHORT& DataLength, struct OVERLAPPED_EX& RecvOverlapped);
	inline bool Write(const CHAR* SendData, const USHORT& DataLength, struct OVERLAPPED_EX& SendOverlapped);

};
#pragma once
#include "../SocketAddress/SocketAddress.h"
#include "../../Socket/NetworkBase/TCP_IP/TCPIPSocket.h"
//#include "../../Socket/NetworkBase/UDP_IP/"

enum class EIOTYPE : UCHAR {
	EIOTYPE_ACCEPT,
	EIOTYPE_READ,
	EIOTYPE_WRITE,
};

struct OVERLAPPED_EX {
	WSAOVERLAPPED m_Overlapped;
	EIOTYPE m_IOType;
	void* Owner;
};

class CSocketUtil {
public:
	static SOCKET CreateTCPSocket() {
		SOCKET NewSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, 0);
		if (NewSocket != INVALID_SOCKET) {
			return NewSocket;
		}
		return INVALID_SOCKET;
	}

};
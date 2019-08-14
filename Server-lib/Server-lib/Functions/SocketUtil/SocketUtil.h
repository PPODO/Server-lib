#pragma once

const size_t MAX_RECEIVE_BUFFER_LENGTH = 4096;

#include "../SocketAddress/SocketAddress.h"
#include "../../Network/PacketSession/NetworkSession/TCPIP/TCPIPSocket.h"

enum class EIOTYPE : UCHAR {
	EIOTYPE_NONE,
	EIOTYPE_ACCEPT,
	EIOTYPE_READ,
	EIOTYPE_WRITE,
};

struct OVERLAPPED_EX {
	WSAOVERLAPPED m_Overlapped;
	EIOTYPE m_IOType;
	void* m_Owner;

public:
	OVERLAPPED_EX() : m_Owner(nullptr), m_IOType(EIOTYPE::EIOTYPE_NONE) { ZeroMemory(&m_Overlapped, sizeof(WSAOVERLAPPED)); }
};

class CSocketUtil {
public:
	static SOCKET CreateTCPSocket() {
		SOCKET NewSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		if (NewSocket != INVALID_SOCKET) {
			return NewSocket;
		}
		return INVALID_SOCKET;
	}

	static SOCKET CreateUDPSocket() {
		SOCKET NewSocket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, nullptr,0, WSA_FLAG_OVERLAPPED);
		if (NewSocket != INVALID_SOCKET) {
			return NewSocket;
		}
		return INVALID_SOCKET;
	}

};
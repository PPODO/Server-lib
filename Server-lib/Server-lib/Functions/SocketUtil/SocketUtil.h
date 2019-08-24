#pragma once
#include "../SocketAddress/SocketAddress.h"

const size_t MAX_RECEIVE_BUFFER_LENGTH = 1024;

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
	static SOCKET CreateSocket(bool bIsTCP) {
		SOCKET NewSocket = bIsTCP ? WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED) : WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		if (NewSocket != INVALID_SOCKET) {
			return NewSocket;
		}
		return INVALID_SOCKET;
	}
};
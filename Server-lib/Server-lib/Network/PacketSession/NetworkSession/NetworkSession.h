#pragma once
#include "NetworkProtocol/NetworkProtocol.h"
#include "../../../Functions/SocketUtil/SocketUtil.h"

using namespace PROTOCOL;

enum class EINITFLAG : uint8_t {
	EIF_NONE,
	EIF_TCP,
	EIT_UDP,
	EIT_BOTH
};

class CNetworkSession {
private:
	OVERLAPPED_EX m_AcceptOverlapped;
	OVERLAPPED_EX m_SendOverlapped;
	OVERLAPPED_EX m_RecvOverlapped;

private:
	TCPIP::CTCPIPSocket* m_TCPIPSocket;
	UDPIP::CUDPIPSocket* m_UDPIPSocket;

public:
	CNetworkSession();

public:
	virtual bool Initialize();
	virtual bool Destroy();

public:
	inline TCPIP::CTCPIPSocket* const GetTCPSocket() const { return m_TCPIPSocket; }
	inline UDPIP::CUDPIPSocket* const GetUDPSocket() const { return m_UDPIPSocket; }
	inline OVERLAPPED_EX* GetOverlappedByIOType(const EIOTYPE& Type = EIOTYPE::EIOTYPE_NONE) {
		switch (Type) {
		case EIOTYPE::EIOTYPE_ACCEPT:
			return &m_AcceptOverlapped;
		case EIOTYPE::EIOTYPE_READ:
			return &m_RecvOverlapped;
		case EIOTYPE::EIOTYPE_WRITE:
			return &m_SendOverlapped;
		default:
			return nullptr;
		}
	}

};
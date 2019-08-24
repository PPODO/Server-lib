#include "NetworkSession.h"

CNetworkSession::CNetworkSession() : m_TCPIPSocket(new TCPIP::CTCPIPSocket), m_UDPIPSocket(new UDPIP::CUDPIPSocket) {
}

bool CNetworkSession::Initialize() {
	ZeroMemory(&m_AcceptOverlapped, sizeof(OVERLAPPED_EX));
	ZeroMemory(&m_SendOverlapped, sizeof(OVERLAPPED_EX));
	ZeroMemory(&m_RecvOverlapped, sizeof(OVERLAPPED_EX));

	m_AcceptOverlapped.m_IOType = EIOTYPE::EIOTYPE_ACCEPT;
	m_SendOverlapped.m_IOType = EIOTYPE::EIOTYPE_WRITE;
	m_RecvOverlapped.m_IOType = EIOTYPE::EIOTYPE_READ;

	m_AcceptOverlapped.m_Owner = m_RecvOverlapped.m_Owner = m_SendOverlapped.m_Owner = this;

	// ¼öÁ¤
	return CSocketSystem::InitializeSocket(m_TCPIPSocket);
}

bool CNetworkSession::Destroy() {
	if (m_TCPIPSocket) {
		m_TCPIPSocket->Destroy();
		delete m_TCPIPSocket;
		m_TCPIPSocket = nullptr;
	}

	if (m_UDPIPSocket) {
		m_UDPIPSocket->Destroy();
		delete m_UDPIPSocket;
		m_UDPIPSocket = nullptr;
	}
	return true;
}
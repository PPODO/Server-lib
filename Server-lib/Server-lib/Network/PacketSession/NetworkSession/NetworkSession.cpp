#include "NetworkSession.h"

bool CNetworkSession::Initialize() {
	CThreadSync Sync;

	ZeroMemory(&m_AcceptOverlapped, sizeof(OVERLAPPED_EX));
	ZeroMemory(&m_RecvOverlapped, sizeof(OVERLAPPED_EX));
	ZeroMemory(&m_SendOverlapped, sizeof(OVERLAPPED_EX));

	m_AcceptOverlapped.m_IOType = EIOTYPE::EIOTYPE_ACCEPT;
	m_RecvOverlapped.m_IOType = EIOTYPE::EIOTYPE_READ;
	m_SendOverlapped.m_IOType = EIOTYPE::EIOTYPE_WRITE;

	return m_TCPSocket.Initialize();
}

bool CNetworkSession::Destroy() {
	CThreadSync Sync;

	if (!m_TCPSocket.Destroy()) {
		return false;
	}
	return true;
}
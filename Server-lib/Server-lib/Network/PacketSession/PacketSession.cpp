#include "PacketSession.h"

CPacketSession::CPacketSession() : m_CurrentReadBytes(0), m_PacketSize(0) {
}

bool CPacketSession::Initialize() {
	CThreadSync Sync;
	if (CNetworkSession::Initialize()) {
		return false;
	}

	ZeroMemory(m_PacketBuffer, MAX_RECEIVE_BUFFER_LENGTH);
	return true;
}

bool CPacketSession::Destroy() {
	CThreadSync Sync;

	return CNetworkSession::Destroy();
}

bool CPacketSession::PacketAnalysis() {
	CThreadSync Sync;

	if (m_PacketSize <= 0) {
		m_PacketSize = GetPacketSize();
	}
	else if(m_CurrentReadBytes >= m_PacketSize) {
		// deserialize

	}
	return true;
}
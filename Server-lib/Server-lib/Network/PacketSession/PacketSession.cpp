#include "PacketSession.h"

CPacketSession::CPacketSession() : m_CurrentReadBytes(0), m_PacketSize(0) {
}

bool CPacketSession::Initialize() {
	CThreadSync Sync;
	if (!CNetworkSession::Initialize()) {
		return false;
	}

	ZeroMemory(m_PacketBuffer, MAX_RECEIVE_BUFFER_LENGTH);
	return true;
}

bool CPacketSession::Destroy() {
	CThreadSync Sync;

	return CNetworkSession::Destroy();
}

CBasePacket* CPacketSession::PacketAnalysis() {
	CThreadSync Sync;

	if (m_PacketSize <= 0 && m_CurrentReadBytes >= sizeof(USHORT)) {
		m_PacketSize = *reinterpret_cast<USHORT*>(m_PacketBuffer);
		if (m_PacketSize <= 0) {
			m_PacketSize = 0;
			CLog::WriteLog(L"Packet Analysis : Wrong Packet!");
			return nullptr;
		}
		m_CurrentReadBytes -= sizeof(USHORT);
	}
	if(m_CurrentReadBytes >= m_PacketSize) {
		CBasePacket* NewPacket = FindPacketType(m_PacketBuffer, m_PacketSize);
		MoveMemory(m_PacketBuffer, m_PacketBuffer + m_PacketSize, m_CurrentReadBytes - m_PacketSize);

		return NewPacket;
	}
	return nullptr;
}
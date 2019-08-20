#include "PacketSession.h"

CPacketSession::CPacketSession() : m_CurrentReadBytes(0), m_PacketInformation(0, 0) {
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

	if (m_CurrentReadBytes <= 0) {
		return nullptr;
	}

	if (m_PacketInformation.m_PacketSize <= 0 && m_CurrentReadBytes >= PACKET_INFORMATION::GetSize()) {
		m_PacketInformation = *reinterpret_cast<PACKET_INFORMATION*>(m_PacketBuffer);
		if (m_PacketInformation.m_PacketSize <= 0) {
			ZeroMemory(&m_PacketInformation, PACKET_INFORMATION::GetSize());
			CLog::WriteLog(L"Packet Analysis : Wrong Packet!");
			return nullptr;
		}

		MoveBufferMemory(PACKET_INFORMATION::GetSize());
		m_CurrentReadBytes -= PACKET_INFORMATION::GetSize();
	}
	if (m_CurrentReadBytes >= m_PacketInformation.m_PacketSize) {
		CBasePacket* NewPacket = GetPacket(m_PacketInformation.m_PacketType, m_PacketBuffer, m_PacketInformation.m_PacketSize);

		m_CurrentReadBytes -= m_PacketInformation.m_PacketSize;
		MoveBufferMemory(m_PacketInformation.m_PacketSize);
		ZeroMemory(&m_PacketInformation, PACKET_INFORMATION::GetSize());

		return NewPacket;
	}
	return nullptr;
}
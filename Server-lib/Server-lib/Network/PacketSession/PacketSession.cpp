#include "PacketSession.h"
#include "../../Functions/SocketUtil/SocketUtil.h"

CPacketSession::CPacketSession() : m_CurrentReceiveBytes(0) {
	ZeroMemory(m_PacketBuffer, MAX_RECEIVE_BUFFER_LENGTH);
	ZeroMemory(&m_PacketInformation, sizeof(PACKET_INFORMATION));
}

bool CPacketSession::Initialize() {
	if (!CNetworkSession::Initialize()) {
		return false;
	}

	return true;
}

bool CPacketSession::Destroy() {
	return CNetworkSession::Destroy();
}

PACKET::DETAIL::CBasePacket* CPacketSession::PacketAnalysis() {

	while (m_CurrentReceiveBytes > 0) {
		if (m_PacketInformation.m_PacketSize == 0 && m_CurrentReceiveBytes >= PACKET_INFORMATION::GetSize()) {
			if (PACKET_INFORMATION* Info = reinterpret_cast<PACKET_INFORMATION*>(m_PacketBuffer)) {
				if (Info->m_PacketSize <= 0) { return nullptr; }

				m_PacketInformation = *Info;
				MoveMemory(m_PacketBuffer, m_PacketBuffer + PACKET_INFORMATION::GetSize(), m_CurrentReceiveBytes);
				m_CurrentReceiveBytes -= PACKET_INFORMATION::GetSize();
			}
			else {
				return nullptr;
			}
		}

		if (m_CurrentReceiveBytes >= m_PacketInformation.m_PacketSize) {
			std::cout << m_CurrentReceiveBytes << std::endl;
			PACKET::DETAIL::CBasePacket* NewPacket = GetPacketObjectByInformation(m_PacketInformation, m_PacketBuffer);

			MoveMemory(m_PacketBuffer, m_PacketBuffer + m_PacketInformation.m_PacketSize, m_CurrentReceiveBytes - m_PacketInformation.m_PacketSize);
			m_CurrentReceiveBytes -= m_PacketInformation.m_PacketSize;
			ZeroMemory(&m_PacketInformation, sizeof(PACKET_INFORMATION));

			return NewPacket;
		}

	}
	return nullptr;
}
#include "PacketBase.h"

void CPacketBase::Initialize() {
	CNetworkBase::Initialize();

	ZeroMemory(m_PacketBuffer, ::MAX_PACKET_BUFFER_LENGTH);
}

void CPacketBase::Clear() {
	CNetworkBase::Clear();

	ZeroMemory(m_PacketBuffer, ::MAX_PACKET_BUFFER_LENGTH);
}

void CPacketBase::OnIOAccept() {
}

void CPacketBase::OnIODisconnect() {
}

void CPacketBase::OnIORecive() {
}

void CPacketBase::OnIOSend() {
}

#include "PacketBase.h"

void CPacketBase::GetPacket() {
}

void CPacketBase::Initialize() {
	CNetworkBase::Initialize();

	ZeroMemory(m_PacketBuffer, ::MAX_PACKET_BUFFER_LENGTH);
}

bool CPacketBase::Initialize(const CNetworkBase* const ListenSocket) {
	if (!CNetworkBase::Initialize(ListenSocket)) {
		return false;
	}

	return true;
}

void CPacketBase::Clear() {
	CNetworkBase::Clear();

	ZeroMemory(m_PacketBuffer, ::MAX_PACKET_BUFFER_LENGTH);
}

bool CPacketBase::OnIOConnect() {
	
	return CNetworkBase::OnIOConnect();
}

bool CPacketBase::OnIODisconnect() {

	return true;
}
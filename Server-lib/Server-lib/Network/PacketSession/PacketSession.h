#pragma once
#include "NetworkSession/NetworkSession.h"
#include "../../Functions/BasePacket/BasePacket.h"

using namespace PACKET;

class CPacketSession : public CNetworkSession {
private:
	char m_PacketBuffer[MAX_RECEIVE_BUFFER_LENGTH];

private:
	PACKET_INFORMATION m_PacketInformation;
	uint16_t m_CurrentReceiveBytes;

protected:
	virtual DETAIL::CBasePacket* GetPacketObjectByInformation(const PACKET_INFORMATION& PacketInfo, const char* PacketBuffer) = 0;

public:
	CPacketSession();

public:
	virtual bool Initialize() override;
	virtual bool Destroy() override;

public:
	PACKET::DETAIL::CBasePacket* PacketAnalysis();

public:
	inline bool CopyReceiveBuffer(const uint16_t& RecvBytes) {
		if (RecvBytes <= 0) {
			return false;
		}

		bool Succeed = CSocketSystem::CopyReceiveBuffer(GetTCPSocket(), m_PacketBuffer + m_CurrentReceiveBytes, RecvBytes);
		m_CurrentReceiveBytes += RecvBytes;
		return Succeed;
	}

};
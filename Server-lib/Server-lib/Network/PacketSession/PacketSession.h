#pragma once
#include "NetworkSession/NetworkSession.h"
#include "../../Functions/BasePacket/BasePacket.h"
#include "../../Functions/CircularQueue/CircularQueue.h"

using namespace PACKET;

class CPacketSession : public CNetworkSession {
private:
	char m_PacketBuffer[MAX_RECEIVE_BUFFER_LENGTH];

private:
	CCircularQueue<BUFFER_DATA*> m_WriteQueue;
	CCircularQueue<BUFFER_DATA*> m_WaitQueue;

private:
	PACKET_INFORMATION m_PacketInformation;
	uint16_t m_CurrentReceiveBytes;
	uint16_t m_LastReceivedPacketNumber;

protected:
	virtual DETAIL::CBasePacket* GetPacketObjectByInformation(const PACKET_INFORMATION& PacketInfo, const char* PacketBuffer) = 0;

public:
	CPacketSession();

public:
	virtual bool Initialize(const EPROTOCOLTYPE& ProtocolType) override;
	virtual bool Destroy() override;

public:
	PACKET_DATA* PacketAnalysis();
	bool Write(const PACKET_INFORMATION& PacketInfo, const char* const DataBuffer, const uint16_t& DataLength);
	bool WriteCompletion();

public:
	// IOCP Only
	inline bool CopyReceiveBuffer(PROTOCOL::CProtocol* const Socket, uint16_t& RecvBytes) {
		if (RecvBytes <= 0) {
			return false;
		}

		bool Succeed = CSocketSystem::CopyReceiveBuffer(Socket, m_PacketBuffer + m_CurrentReceiveBytes, RecvBytes);
		m_CurrentReceiveBytes += RecvBytes;
		return Succeed;
	}
	//inline bool CopyReceiveFromBuffer();

	// EventSelect Only
	inline bool ReceiveEventSelect() {
		uint16_t RecvBytes = 0;
		
		if (CSocketSystem::ReceiveEventSelect(GetTCPSocket(), m_PacketBuffer, RecvBytes)) {
			m_CurrentReceiveBytes += RecvBytes;

			return true;
		}
		return false;
	}
	inline bool ReceiveFromEventSelect() {
		uint16_t RecvBytes = 0;

		if (CSocketSystem::ReceiveFromEventSelect(GetUDPSocket(), m_PacketBuffer, RecvBytes)) {
			m_CurrentReceiveBytes += RecvBytes;

			return true;
		}
		return false;
	}

public:
	inline uint16_t GetLastReceivedPacketNumber() const { return m_LastReceivedPacketNumber; }

};
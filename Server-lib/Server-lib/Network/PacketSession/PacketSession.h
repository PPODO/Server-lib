#pragma once
#include "NetworkSession/NetworkSession.h"
#include "../../Functions/BasePacket/BasePacket.h"

class CPacketSession : public CNetworkSession {
private:
	CHAR m_PacketBuffer[MAX_RECEIVE_BUFFER_LENGTH];
	size_t m_CurrentReadBytes;

private:
	USHORT m_PacketSize;

protected:
	virtual CBasePacket* FindPacketType(const CHAR* Buffer, const size_t& PacketSize) = 0;

public:
	CPacketSession();

public:
	virtual bool Initialize() override;
	virtual bool Destroy() override;

public:
	inline bool CopyIOCPBuffer(const USHORT& DataLength) {
		CThreadSync Sync;

		if (!CNetworkSession::CopyIOCPBuffer(m_PacketBuffer + m_CurrentReadBytes, DataLength)) {
			return false;
		}
		m_CurrentReadBytes += DataLength;
		return true;
	}

public:
	CBasePacket* PacketAnalysis();

};
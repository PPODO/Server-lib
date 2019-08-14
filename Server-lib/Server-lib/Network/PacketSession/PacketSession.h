#pragma once
#include "NetworkSession/NetworkSession.h"

class CPacketSession : public CNetworkSession {
private:
	CHAR m_PacketBuffer[MAX_RECEIVE_BUFFER_LENGTH];
	size_t m_CurrentReadBytes;

private:
	size_t m_PacketSize;

private:
	inline size_t GetPacketSize() {
		return reinterpret_cast<uint8_t>(m_PacketBuffer);
	}

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
	bool PacketAnalysis();

};
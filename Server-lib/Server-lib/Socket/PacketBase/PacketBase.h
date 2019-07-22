#pragma once
#include "../NetworkBase/NetworkBase.h"

static const size_t MAX_PACKET_BUFFER_LENGTH = 8192;

class CPacketBase : public CNetworkBase {
private:
	UINT16 m_ReadDataLength;
	UINT16 m_LastRecivePacketNumber;

private:
	CHAR m_PacketBuffer[MAX_PACKET_BUFFER_LENGTH];

public:
	inline bool CopyRecvBuffer_IOCP(const UINT16& RecvBytes) {
	//	CThreadSync Sync(this);

		if (!CNetworkBase::ReadIOCP(m_PacketBuffer + m_ReadDataLength, RecvBytes)) {
			return false;
		}
		m_ReadDataLength += RecvBytes;
		return true;
	}

public:
	virtual void GetPacket();

public:
	CPacketBase() : m_ReadDataLength(0), m_LastRecivePacketNumber(0) { ZeroMemory(m_PacketBuffer, sizeof(m_PacketBuffer)); };

public:
	virtual void Initialize() override;
	virtual bool Initialize(const CNetworkBase* const ListenSocket) override;
	virtual void Clear() override;

public:
	virtual bool OnIOConnect();
	virtual bool OnIODisconnect();

};
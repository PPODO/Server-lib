#pragma once
#include "../NetworkBase/NetworkBase.h"

static const size_t MAX_PACKET_BUFFER_LENGTH = 8192;

class CPacketBase : public CNetworkBase {
private:


private:
	CHAR m_PacketBuffer[MAX_PACKET_BUFFER_LENGTH];

public:
	virtual void Initialize() override;
	virtual void Clear() override;

public:
	virtual void OnIOAccept() override;
	virtual void OnIODisconnect() override;
	virtual void OnIORecive() override;
	virtual void OnIOSend() override;
	
};
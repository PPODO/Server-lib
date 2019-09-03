#pragma once
#include "../EventSelect.h"

template<typename SESSIONTYPE>
class CEventSelectTCP : public CEventSelect {
private:

protected:
	virtual bool OnIORead() override;

public:
	virtual bool Initialize(const CSocketAddress& ConnectionAddress, const EPROTOCOLTYPE& ProtocolType = EPROTOCOLTYPE::EPT_NONE) override;
	virtual bool Destroy() override;

};

template<typename SESSIONTYPE>
inline bool CEventSelectTCP<SESSIONTYPE>::Initialize(const CSocketAddress& ConnectionAddress, const EPROTOCOLTYPE& ProtocolType) {
	SetSessionSocketObject(new SESSIONTYPE);
	if (CPacketSession * TempSession = GetSessionSocket<CPacketSession>()) {
		if (TempSession->Initialize(EPROTOCOLTYPE::EPT_TCP) && CSocketSystem::Connect(TempSession->GetTCPSocket(), ConnectionAddress)) {
			return CEventSelect::Initialize(ConnectionAddress, EPROTOCOLTYPE::EPT_TCP);
		}
	}
	return false;
}

template<typename SESSIONTYPE>
inline bool CEventSelectTCP<SESSIONTYPE>::Destroy() {

	return CEventSelect::Destroy();
}

template<typename SESSIONTYPE>
inline bool CEventSelectTCP<SESSIONTYPE>::OnIORead() {
	if (CPacketSession * TempSession = GetSessionSocket<CPacketSession>()) {
		if (TempSession->ReceiveEventSelect()) {
			if (PACKET_DATA * NewPacketData = TempSession->PacketAnalysis()) {

				return true;
			}
		}
	}
	return false;
}
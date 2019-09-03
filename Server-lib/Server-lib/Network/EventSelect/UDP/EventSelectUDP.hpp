#pragma once
#include "../EventSelect.h"

template<typename SESSIONTYPE>
class CEventSelectUDP : public CEventSelect {
private:

protected:
	virtual bool OnIORead() override;

public:
	virtual bool Initialize(const CSocketAddress& ConnectionAddress = CSocketAddress(), const EPROTOCOLTYPE& ProtocolType = EPROTOCOLTYPE::EPT_NONE) override;
	virtual bool Destroy() override;

};

template<typename SESSIONTYPE>
inline bool CEventSelectUDP<SESSIONTYPE>::Initialize(const CSocketAddress& ConnectionAddress, const EPROTOCOLTYPE& ProtocolType) {
	SetSessionSocketObject(new SESSIONTYPE);
	if (CPacketSession * TempSession = GetSessionSocket<CPacketSession>()) {
		if (TempSession->Initialize(EPROTOCOLTYPE::EPT_UDP) && CEventSelect::Initialize(ConnectionAddress, EPROTOCOLTYPE::EPT_UDP)) {
			return true;
		}
	}
	CLog::WriteLog(L"");
	return false;
}

template<typename SESSIONTYPE>
inline bool CEventSelectUDP<SESSIONTYPE>::Destroy() {
	return CEventSelect::Destroy();
}

template<typename SESSIONTYPE>
inline bool CEventSelectUDP<SESSIONTYPE>::OnIORead() {
	if (CPacketSession * TempSession = GetSessionSocket<CPacketSession>()) {
		if (TempSession->ReceiveFromEventSelect()) {
			if (PACKET_DATA * NewPacketData = TempSession->PacketAnalysis()) {

				return true;
			}
		}
	}
	return false;
}
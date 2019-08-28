#pragma once
#include "../IOCP.h"

template<typename SESSIONTYPE>
class CIOCPUDP : public CIOCP {
private:

protected:
	virtual bool OnIOConnect(void* const Object) override;
	virtual bool OnIODisconnect(void* const Object) override;
	virtual bool OnIORead(void* const Object, const uint16_t& RecvBytes) override;
	virtual bool OnIOWrite(void* const Object) override;

public:
	virtual bool Initialize() override;
	virtual bool Destroy() override;

};

template<typename SESSIONTYPE>
inline bool CIOCPUDP<SESSIONTYPE>::Initialize() {
	if (!std::is_base_of<CNetworkSession, SESSIONTYPE>()) {
		CLog::WriteLog(L"Initialize IOCP : Only Classes That Inherit Sessions Are Supported!");
		return false;
	}

	if (!CIOCP::Initialize()) {
		CLog::WriteLog(L"Initialize IOCP : Failed To Initialization IOCP!");
		return false;
	}

	SetListenSesisonObject(new SESSIONTYPE);
	if (CPacketSession* TempSession = GetListenSession<CPacketSession>()) {
		CSocketAddress BindAddress("127.0.0.1", 3550);

		if (!TempSession->Initialize() || !CSocketSystem::Bind(TempSession->GetUDPSocket(), BindAddress) || !RegisterIOCompletionPort(CSocketSystem::GetSocketByClass(TempSession->GetUDPSocket()), reinterpret_cast<ULONG_PTR&>(*GetListenSession<SESSIONTYPE>()))) {
			CLog::WriteLog(L"Initialize IOCP : Failed To Create Listen Socket!");
			return false;
		}
	}

	return true;
}

template<typename SESSIONTYPE>
inline bool CIOCPUDP<SESSIONTYPE>::Destroy() {
	return CIOCP::Destroy();
}

template<typename SESSIONTYPE>
inline bool CIOCPUDP<SESSIONTYPE>::OnIOConnect(void* const Object) {
	return true;
}

template<typename SESSIONTYPE>
inline bool CIOCPUDP<SESSIONTYPE>::OnIODisconnect(void* const Object) {
	return true;
}

template<typename SESSIONTYPE>
inline bool CIOCPUDP<SESSIONTYPE>::OnIORead(void* const Object, const uint16_t& RecvBytes) {
	if (CPacketSession * Client = reinterpret_cast<CPacketSession*>(Object)) {
		if (Client->CopyReceiveBuffer(Client->GetUDPSocket(), RecvBytes)) {
			if (PACKET_DATA * NewPacketData = Client->PacketAnalysis()) {
				AddNewPacketAtQueue(NewPacketData);
			}
		}
		return CSocketSystem::ReceiveFrom(Client->GetUDPSocket(), Client->GetOverlappedByIOType(EIOTYPE::EIOTYPE_READ));
	}
}

template<typename SESSIONTYPE>
inline bool CIOCPUDP<SESSIONTYPE>::OnIOWrite(void* const Object) {
	return CIOCP::OnIOWrite(Object);
}
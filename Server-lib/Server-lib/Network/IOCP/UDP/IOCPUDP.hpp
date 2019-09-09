#pragma once
#include "../IOCP.hpp"

template<typename SESSIONTYPE>
class CIOCPUDP : public CIOCP<SESSIONTYPE> {
private:

protected:
	virtual bool OnIOConnect(void* const Object) override;
	virtual bool OnIODisconnect(void* const Object) override;
	virtual bool OnIORead(void* const Object, uint16_t& RecvBytes) override;
	virtual bool OnIOWrite(void* const Object) override;

public:
	virtual bool Initialize() override;
	virtual bool Destroy() override;

};

template<typename SESSIONTYPE>
inline bool CIOCPUDP<SESSIONTYPE>::Initialize() {
	if (!CIOCP<SESSIONTYPE>::Initialize()) {
		CLog::WriteLog(L"Initialize IOCP : Failed To Initialization IOCP!");
		return false;
	}

	if (CPacketSession* TempSession = this->GetListenSession<CPacketSession>()) {
		CSocketAddress BindAddress("127.0.0.1", 3550);

		if (!TempSession->Initialize(EPROTOCOLTYPE::EPT_UDP) || !CSocketSystem::Bind(TempSession->GetUDPSocket(), BindAddress) || !this->RegisterIOCompletionPort(CSocketSystem::GetSocketByClass(TempSession->GetUDPSocket()), reinterpret_cast<ULONG_PTR&>(*this->GetListenSession<SESSIONTYPE>()))) {
			CLog::WriteLog(L"Initialize IOCP : Failed To Create Listen Socket!");
			return false;
		}
		return CSocketSystem::ReceiveFrom(TempSession->GetUDPSocket(), TempSession->GetOverlappedByIOType(EIOTYPE::EIOTYPE_READ));
	}
	return false;
}

template<typename SESSIONTYPE>
inline bool CIOCPUDP<SESSIONTYPE>::Destroy() {
	return CIOCP<SESSIONTYPE>::Destroy();
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
inline bool CIOCPUDP<SESSIONTYPE>::OnIORead(void* const Object, uint16_t& RecvBytes) {
	if (CPacketSession * Client = reinterpret_cast<CPacketSession*>(Object)) {
		if (Client->CopyReceiveBuffer(Client->GetUDPSocket(), RecvBytes)) {
			if (PACKET_DATA * NewPacketData = Client->PacketAnalysis()) {
				this->AddNewDataAtPacketQueue(NewPacketData);
			}
		}
		return CSocketSystem::ReceiveFrom(Client->GetUDPSocket(), Client->GetOverlappedByIOType(EIOTYPE::EIOTYPE_READ));
	}
}

template<typename SESSIONTYPE>
inline bool CIOCPUDP<SESSIONTYPE>::OnIOWrite(void* const Object) {
	return CIOCP<SESSIONTYPE>::OnIOWrite(Object);
}
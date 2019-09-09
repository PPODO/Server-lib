#pragma once
#include "../IOCP.hpp"

template<typename SESSIONTYPE, size_t MAX_CLIENT_COUNT = 256>
class CIOCPTCP : public CIOCP<SESSIONTYPE> {
private:
	std::vector<SESSIONTYPE*> m_Clients;

private:
	virtual bool ProcessBroadCast(BROADCAST_DATA<SESSIONTYPE>& Data) override;

protected:
	virtual bool OnIOConnect(void* const Object) override;
	virtual bool OnIODisconnect(void* const Object) override;
	virtual bool OnIORead(void* const Object, uint16_t& RecvBytes) override;
	virtual bool OnIOWrite(void* const Object) override;

public:
	virtual bool Initialize() override;
	virtual bool Destroy() override;

};

template<typename SESSIONTYPE, size_t MAX_CLIENT_COUNT>
inline bool CIOCPTCP<SESSIONTYPE, MAX_CLIENT_COUNT>::Initialize() {
	if (!CIOCP<SESSIONTYPE>::Initialize()) {
		CLog::WriteLog(L"Initialize IOCP : Failed To Initialization IOCP!");
		return false;
	}

	if (CPacketSession* TempSession = this->GetListenSession<CPacketSession>()) {
		CSocketAddress BindAddress("127.0.0.1", 3550);

		if (!TempSession->Initialize(EPROTOCOLTYPE::EPT_TCP) || !CSocketSystem::Bind(TempSession->GetTCPSocket(), BindAddress) || !this->RegisterIOCompletionPort(CSocketSystem::GetSocketByClass(TempSession->GetTCPSocket()), reinterpret_cast<ULONG_PTR&>(*this->GetListenSession<SESSIONTYPE>()))) {
			CLog::WriteLog(L"Initialize IOCP : Failed To Create Listen Socket!");
			return false;
		}

		for (size_t i = 0; i < MAX_CLIENT_COUNT; i++) {
			SESSIONTYPE* Client = new SESSIONTYPE;
			if (CPacketSession * TempClient = reinterpret_cast<CPacketSession*>(Client)) {
				if (TempClient->Initialize(EPROTOCOLTYPE::EPT_TCP) && CSocketSystem::Accept(TempClient->GetTCPSocket(), CSocketSystem::GetSocketByClass(TempSession->GetTCPSocket()), TempClient->GetOverlappedByIOType(EIOTYPE::EIOTYPE_ACCEPT))) {
					m_Clients.push_back(Client);
				}
				else {
					CLog::WriteLog(L"Initialize IOCP : Failed To Create Client Socket!");
					return false;
				}
			}
		}
	}

	return true;
}

template<typename SESSIONTYPE, size_t MAX_CLIENT_COUNT>
inline bool CIOCPTCP<SESSIONTYPE, MAX_CLIENT_COUNT>::Destroy() {
	for (auto& Iterator : m_Clients) {
		if (CPacketSession * TempClient = reinterpret_cast<CPacketSession*>(Iterator)) {
			TempClient->Destroy();
			delete TempClient;
		}
		else {
			break;
		}
	}
	m_Clients.clear();

	return CIOCP<SESSIONTYPE>::Destroy();
}

template<typename SESSIONTYPE, size_t MAX_CLIENT_COUNT>
inline bool CIOCPTCP<SESSIONTYPE, MAX_CLIENT_COUNT>::OnIOConnect(void* const Object) {
	if (CPacketSession * Client = reinterpret_cast<CPacketSession*>(Object)) {
		if (this->RegisterIOCompletionPort(CSocketSystem::GetSocketByClass(Client->GetTCPSocket()), reinterpret_cast<ULONG_PTR>(Client)) && CSocketSystem::Receive(Client->GetTCPSocket(), Client->GetOverlappedByIOType(EIOTYPE::EIOTYPE_READ))) {
			CLog::WriteLog(L"On IO Accept : The Client Has been Accepted!");
			return true;
		}
	}
	CLog::WriteLog(L"On IO Accept : Failed To Aceept Client!");
	return false;
}

template<typename SESSIONTYPE, size_t MAX_CLIENT_COUNT>
inline bool CIOCPTCP<SESSIONTYPE, MAX_CLIENT_COUNT>::OnIODisconnect(void* const Object) {
	if (CPacketSession * Client = reinterpret_cast<CPacketSession*>(Object)) {
		Client->Destroy();

		if (Client->Initialize(EPROTOCOLTYPE::EPT_TCP) && CSocketSystem::Accept(Client->GetTCPSocket(), CSocketSystem::GetSocketByClass(this->GetListenSession<CPacketSession>()->GetTCPSocket()), Client->GetOverlappedByIOType(EIOTYPE::EIOTYPE_ACCEPT))) {
			CLog::WriteLog(L"On IO Disconnect : Client Disconnect Successful!");
			return true;
		}
	}
	CLog::WriteLog(L"On IO Disconnect : Failed To Disconnect Client!");
	return false;
}

template<typename SESSIONTYPE, size_t MAX_CLIENT_COUNT>
inline bool CIOCPTCP<SESSIONTYPE, MAX_CLIENT_COUNT>::OnIORead(void* const Object, uint16_t& RecvBytes) {
	if (CPacketSession * Client = reinterpret_cast<CPacketSession*>(Object)) {
		if (Client->CopyReceiveBuffer(Client->GetTCPSocket(), RecvBytes)) {
			if (PACKET_DATA * NewPacketData = Client->PacketAnalysis()) {
				 this->AddNewDataAtPacketQueue(NewPacketData);
			}
		}
		return CSocketSystem::Receive(Client->GetTCPSocket(), Client->GetOverlappedByIOType(EIOTYPE::EIOTYPE_READ));
	}
	return false;
}

template<typename SESSIONTYPE, size_t MAX_CLIENT_COUNT>
inline bool CIOCPTCP<SESSIONTYPE, MAX_CLIENT_COUNT>::OnIOWrite(void* const Object) {
	return CIOCP<SESSIONTYPE>::OnIOWrite(Object);
}

template<typename SESSIONTYPE, size_t MAX_CLIENT_COUNT>
inline bool CIOCPTCP<SESSIONTYPE, MAX_CLIENT_COUNT>::ProcessBroadCast(BROADCAST_DATA<SESSIONTYPE>& Data) {
	if (CPacketSession * Owner = reinterpret_cast<CPacketSession*>(Data.m_Owner)) {
		if (Data.m_bIsBroadCast && Data.m_BroadCastData) {
			// Lock?
			for (auto& Iterator : Data.m_BroadCastList) {
				if (CPacketSession* Client = reinterpret_cast<CPacketSession*>(Iterator)) {
					if (!Data.m_OwnerData || Client != Owner) {
						Client->Write(Data.m_BroadCastData);
					}
					else {
						Owner->Write(Data.m_OwnerData);
					}
				}
			}
			return true;
		}
		else if(Data.m_OwnerData) {
			return Owner->Write(Data.m_OwnerData);
		}
	}
	return false;
}
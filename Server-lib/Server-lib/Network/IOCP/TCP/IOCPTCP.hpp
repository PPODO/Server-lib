#pragma once
#include "../IOCP.h"

template<typename SESSIONTYPE, size_t MAX_CLIENT_COUNT = 256>
class CIOCPTCP : public CIOCP {
private:
	std::vector<SESSIONTYPE*> m_Clients;

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

		if (!TempSession->Initialize(EPROTOCOLTYPE::EPT_TCP) || !CSocketSystem::Bind(TempSession->GetTCPSocket(), BindAddress) || !RegisterIOCompletionPort(CSocketSystem::GetSocketByClass(TempSession->GetTCPSocket()), reinterpret_cast<ULONG_PTR&>(*GetListenSession<SESSIONTYPE>()))) {
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

	return CIOCP::Destroy();
}

template<typename SESSIONTYPE, size_t MAX_CLIENT_COUNT>
inline bool CIOCPTCP<SESSIONTYPE, MAX_CLIENT_COUNT>::OnIOConnect(void* const Object) {
	if (CPacketSession * Client = reinterpret_cast<CPacketSession*>(Object)) {
		if (RegisterIOCompletionPort(CSocketSystem::GetSocketByClass(Client->GetTCPSocket()), reinterpret_cast<ULONG_PTR>(Client)) && CSocketSystem::Receive(Client->GetTCPSocket(), Client->GetOverlappedByIOType(EIOTYPE::EIOTYPE_READ))) {
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

		if (Client->Initialize(EPROTOCOLTYPE::EPT_TCP) && CSocketSystem::Accept(Client->GetTCPSocket(), CSocketSystem::GetSocketByClass(GetListenSession<CPacketSession>()->GetTCPSocket()), Client->GetOverlappedByIOType(EIOTYPE::EIOTYPE_ACCEPT))) {
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
				AddNewPacketAtQueue(NewPacketData);
			}
		}
		return CSocketSystem::Receive(Client->GetTCPSocket(), Client->GetOverlappedByIOType(EIOTYPE::EIOTYPE_READ));
	}
	return false;
}

template<typename SESSIONTYPE, size_t MAX_CLIENT_COUNT>
inline bool CIOCPTCP<SESSIONTYPE, MAX_CLIENT_COUNT>::OnIOWrite(void* const Object) {
	return CIOCP::OnIOWrite(Object);
}
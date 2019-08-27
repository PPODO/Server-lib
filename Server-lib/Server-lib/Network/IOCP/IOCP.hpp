#pragma once
#include "../PacketSession/PacketSession.h"
#include "../../Functions/CircularQueue/CircularQueue.h"
#include <vector>
#include <thread>
#include <array>

template<typename SESSIONTYPE, size_t MAX_CLIENT_COUNT = 256>
class CIOCP {
private:
	WSADATA m_WinSockData;

private:
	HANDLE m_hIOCP;
	HANDLE m_hWaitForInitialize;

private:
	SESSIONTYPE* m_ListenSession;
	std::vector<SESSIONTYPE*> m_Clients;

private:
	std::vector<std::thread> m_WorkerThread;
	std::vector<std::thread> m_PacketThread;

private:
	CCircularQueue<PACKET_DATA*> m_PacketQueue;

private:
	bool CreateWorkerThread();
	bool ProcessWorkerThread();
	bool ProcessPacketThread();

protected:
	virtual bool OnIOConnect(void* const Object);
	virtual bool OnIODisconnect(void* const Object);
	virtual bool OnIORead(void* const Object, const uint16_t& RecvBytes);
	virtual bool OnIOWrite(void* const Object);
	virtual bool ProcessPacket(PACKET_DATA& PacketData) = 0;

protected:
	inline bool RegisterIOCompletionPort(const SOCKET& Socket, const ULONG_PTR& CompletionKey) {
		if (Socket == INVALID_SOCKET || m_hIOCP == NULL) {
			CLog::WriteLog(L"Register IO Completion Port : Invalid Socket or Invalid Handle Value!");
			return false;
		}

		if ((m_hIOCP = CreateIoCompletionPort(reinterpret_cast<HANDLE>(Socket), m_hIOCP, CompletionKey, 0)) == NULL) {
			CLog::WriteLog(L"Register IO Completion Port : Failed To Register Completion Port!");
			return false;
		}
		return true;
	}

protected:
	inline HANDLE GetIOCPHandle() const { return m_hIOCP; }

public:
	CIOCP() : m_hIOCP(INVALID_HANDLE_VALUE), m_hWaitForInitialize(INVALID_HANDLE_VALUE), m_ListenSession(nullptr) { m_Clients.resize(MAX_CLIENT_COUNT); };

public:
	virtual bool Initialize();
	virtual bool Destroy();

};

template<typename SESSIONTYPE, size_t MAX_CLIENT_COUNT>
bool CIOCP<SESSIONTYPE, MAX_CLIENT_COUNT>::Initialize() {
	if (!std::is_base_of<CNetworkSession, SESSIONTYPE>()) {
		throw "";
	}

	if (WSAStartup(WINSOCK_VERSION, &m_WinSockData) == SOCKET_ERROR) {
		CLog::WriteLog(L"Initialize IOCP : WSAStartup Failure!");
		return false;
	}

	if ((m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL) {
		CLog::WriteLog(L"Initialize IOCP : Failed To Create Completion Port!");
		return false;
	}

	if ((m_hWaitForInitialize = CreateEvent(nullptr, false, false, nullptr)) == NULL) {
		CLog::WriteLog(L"Initialize IOCP : Failed To Create Wait Event!");
		return false;
	}

	m_ListenSession = new SESSIONTYPE;
	if (CPacketSession * TempSession = reinterpret_cast<CPacketSession*>(m_ListenSession)) {
		CSocketAddress BindAddress("127.0.0.1", 3550);

		if (!TempSession->Initialize() || !CSocketSystem::Listen(TempSession->GetTCPSocket(), BindAddress) || !RegisterIOCompletionPort(CSocketSystem::GetSocketByClass(TempSession->GetTCPSocket()), reinterpret_cast<ULONG_PTR>(m_ListenSession))) {
			CLog::WriteLog(L"Initialize FHIOCP : Failed To Create Listen Socket!");
			return false;
		}

		for (auto& Iterator : m_Clients) {
			SESSIONTYPE* Client = new SESSIONTYPE;
			if (CPacketSession * TempClient = reinterpret_cast<CPacketSession*>(Client)) {
				if (TempClient->Initialize() && CSocketSystem::Accept(TempClient->GetTCPSocket(), CSocketSystem::GetSocketByClass(TempSession->GetTCPSocket()), TempClient->GetOverlappedByIOType(EIOTYPE::EIOTYPE_ACCEPT))) {
					Iterator = Client;
				}
				else {
					CLog::WriteLog(L"Initialize FHIOCP : Failed To Create Client Socket!");
					return false;
				}
			}
		}
	}

	m_PacketThread.push_back(std::thread(&CIOCP::ProcessPacketThread, this));

	if (!CreateWorkerThread()) {
		CLog::WriteLog(L"Create Worker Thread : Failed To Create Worker Thread!");
		return false;
	}

	WaitForSingleObject(m_hWaitForInitialize, INFINITE);

	CLog::WriteLog(L"Initialize FHIOCP : Initialization Successful!");
	return true;
}

template<typename SESSIONTYPE, size_t MAX_CLIENT_COUNT>
bool CIOCP<SESSIONTYPE, MAX_CLIENT_COUNT>::Destroy() {
	for (auto& Iterator : m_WorkerThread) {
		PostQueuedCompletionStatus(m_hIOCP, 0, 0, nullptr);
		Iterator.join();
	}
	m_WorkerThread.clear();

	for (auto& Iterator : m_PacketThread) {
		Iterator.join();
	}
	m_PacketThread.clear();

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

	if (CPacketSession * TempSession = reinterpret_cast<CPacketSession*>(m_ListenSession)) {
		TempSession->Destroy();
		delete TempSession;
		TempSession = nullptr;
	}

	if (m_hWaitForInitialize) {
		CloseHandle(m_hWaitForInitialize);
		m_hWaitForInitialize = INVALID_HANDLE_VALUE;
	}

	if (m_hIOCP) {
		CloseHandle(m_hIOCP);
		m_hIOCP = INVALID_HANDLE_VALUE;
	}

	WSACleanup();
	return true;
}

template<typename SESSIONTYPE, size_t MAX_CLIENT_COUNT>
bool CIOCP<SESSIONTYPE, MAX_CLIENT_COUNT>::CreateWorkerThread() {
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);

	for (size_t i = 0; i < SysInfo.dwNumberOfProcessors * 2; i++) {
		m_WorkerThread.push_back(std::thread(&CIOCP::ProcessWorkerThread, this));
	}
	CLog::WriteLog(L"Create Worker Thread : Worker Thread Creation Successful! - %d", SysInfo.dwNumberOfProcessors * 2);
	return true;
}

template<typename SESSIONTYPE, size_t MAX_CLIENT_COUNT>
bool CIOCP<SESSIONTYPE, MAX_CLIENT_COUNT>::ProcessWorkerThread() {
	DWORD RecvBytes = 0;
	LPOVERLAPPED Overlapped;
	void* CompletionKey = nullptr;

	SetEvent(m_hWaitForInitialize);
	while (true) {
		bool Succeed = GetQueuedCompletionStatus(m_hIOCP, &RecvBytes, reinterpret_cast<PULONG_PTR>(&CompletionKey), &Overlapped, INFINITE);

		if (!CompletionKey) {
			CLog::WriteLog(L"Worker Thread : Worker Threads Stopped!");
			return false;
		}

		if (OVERLAPPED_EX * OverlappedEx = reinterpret_cast<OVERLAPPED_EX*>(Overlapped)) {
			if (!Succeed || (Succeed && RecvBytes <= 0)) {
				switch (OverlappedEx->m_IOType) {
				case EIOTYPE::EIOTYPE_ACCEPT:
					OnIOConnect(OverlappedEx->m_Owner);
					break;
				default:
					OnIODisconnect(OverlappedEx->m_Owner);
					break;
				}
				continue;
			}

			switch (OverlappedEx->m_IOType) {
			case EIOTYPE::EIOTYPE_READ:
				OnIORead(OverlappedEx->m_Owner, RecvBytes);
				break;
			case EIOTYPE::EIOTYPE_WRITE:
				OnIOWrite(OverlappedEx->m_Owner);
				break;
			default:
				CLog::WriteLog(L"");
				break;
			}
		}
	}
	return true;
}

template<typename SESSIONTYPE, size_t MAX_CLIENT_COUNT>
inline bool CIOCP<SESSIONTYPE, MAX_CLIENT_COUNT>::ProcessPacketThread() {
	while (true) {
		if (!m_PacketQueue.IsEmpty()) {
			PACKET_DATA* Data = nullptr;
			if (m_PacketQueue.Pop(Data) && Data) {
				ProcessPacket(*Data);

				delete Data;
			}
		}
	}
	return false;
}

template<typename SESSIONTYPE, size_t MAX_CLIENT_COUNT>
bool CIOCP<SESSIONTYPE, MAX_CLIENT_COUNT>::OnIOConnect(void* const Object) {
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
bool CIOCP<SESSIONTYPE, MAX_CLIENT_COUNT>::OnIODisconnect(void* const Object) {
	if (CPacketSession * Client = reinterpret_cast<CPacketSession*>(Object)) {
		Client->Destroy();

		if (Client->Initialize() && CSocketSystem::Accept(Client->GetTCPSocket(), CSocketSystem::GetSocketByClass(m_ListenSession->GetTCPSocket()), Client->GetOverlappedByIOType(EIOTYPE::EIOTYPE_ACCEPT))) {
			CLog::WriteLog(L"On IO Disconnect : Client Disconnect Successful!");
			return true;
		}
	}
	CLog::WriteLog(L"On IO Disconnect : Failed To Disconnect Client!");
	return false;
}

template<typename SESSIONTYPE, size_t MAX_CLIENT_COUNT>
bool CIOCP<SESSIONTYPE, MAX_CLIENT_COUNT>::OnIORead(void* const Object, const uint16_t& RecvBytes) {
	if (CPacketSession* Client = reinterpret_cast<CPacketSession*>(Object)) {
		if (Client->CopyReceiveBuffer(RecvBytes)) {
			if (PACKET_DATA* NewPacketData = Client->PacketAnalysis()) {
				m_PacketQueue.Push(NewPacketData);
			}
		}
		return CSocketSystem::Receive(Client->GetTCPSocket(), Client->GetOverlappedByIOType(EIOTYPE::EIOTYPE_READ));
	}
	return false;
}

template<typename SESSIONTYPE, size_t MAX_CLIENT_COUNT>
bool CIOCP<SESSIONTYPE, MAX_CLIENT_COUNT>::OnIOWrite(void* const Object) {
	if (CPacketSession * Client = reinterpret_cast<CPacketSession*>(Object)) {
		Client->WriteCompletion();
	}
	return false;
}
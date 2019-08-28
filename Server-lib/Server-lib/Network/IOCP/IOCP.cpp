#include "IOCP.h"

bool CIOCP::Initialize() {
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

	if (!CreateWorkerThread()) {
		CLog::WriteLog(L"Create Worker Thread : Failed To Create Worker Thread!");
		return false;
	}

	WaitForSingleObject(m_hWaitForInitialize, INFINITE);

	CLog::WriteLog(L"Initialize FHIOCP : Initialization Successful!");
	return true;
}

bool CIOCP::Destroy() {
	if (m_ListenSession) {
		m_ListenSession->Destroy();
		delete m_ListenSession;
		m_ListenSession = nullptr;
	}

	for (auto& Iterator : m_WorkerThread) {
		PostQueuedCompletionStatus(m_hIOCP, 0, 0, nullptr);
		Iterator.join();
	}
	m_WorkerThread.clear();

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

bool CIOCP::CreateWorkerThread() {
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);

	size_t ProcessorCount = SysInfo.dwNumberOfProcessors * 2;
	for (size_t i = 0; i < ProcessorCount; i++) {
		m_WorkerThread.push_back(std::thread(&CIOCP::ProcessWorkerThread, this));
	}
	CLog::WriteLog(L"Create Worker Thread : Worker Thread Creation Successful! - %d", SysInfo.dwNumberOfProcessors * 2);
	return true;
}

bool CIOCP::ProcessWorkerThread() {
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

inline bool CIOCP::ProcessPacketThread() {
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

bool CIOCP::OnIOWrite(void* const Object) {
	if (CPacketSession * Client = reinterpret_cast<CPacketSession*>(Object)) {
		Client->WriteCompletion();
	}
	return false;
}
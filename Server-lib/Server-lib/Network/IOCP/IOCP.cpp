#include "IOCP.h"
#include "../../Functions/SocketUtil/SocketUtil.h"

CIOCP::CIOCP() : m_hIOCP(INVALID_HANDLE_VALUE), m_hWaitForInitalization(INVALID_HANDLE_VALUE) {
}

bool CIOCP::Initialize() {
	if (WSAStartup(WINSOCK_VERSION, &m_WinSockData) == SOCKET_ERROR) {
		CLog::WriteLog(L"Initialize IOCP : WSAStartup Failure!");
		return false;
	}

	if ((m_hWaitForInitalization = CreateEvent(nullptr, false, false, nullptr)) == NULL) {
		CLog::WriteLog(L"Initialize IOCP : ");
		return false;
	}

	if ((m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0)) == NULL) {
		CLog::WriteLog(L"Initialize IOCP : Failed To Create Completion Port!");
		return false;
	}
	CreateWorkerThread();

	WaitForSingleObject(m_hWaitForInitalization, INFINITE);
	CLog::WriteLog(L"Initialize IOCP : Initialization Successful!");
	return true;
}

bool CIOCP::Destroy() {
	for (auto& Iterator : m_WorkerThreads) {
		PostQueuedCompletionStatus(m_hIOCP, 0, 0, nullptr);
		Iterator.join();
	}

	if (m_hWaitForInitalization != INVALID_HANDLE_VALUE) {
		CloseHandle(m_hWaitForInitalization);
		m_hWaitForInitalization = INVALID_HANDLE_VALUE;
	}

	if (m_hIOCP != INVALID_HANDLE_VALUE) {
		CloseHandle(m_hIOCP);
		m_hIOCP = INVALID_HANDLE_VALUE;
	}

	WSACleanup();
	return true;
}

void CIOCP::CreateWorkerThread() {
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);
	
	for (size_t i = 0; i < SysInfo.dwNumberOfProcessors * 2; i++) {
		m_WorkerThreads.push_back(std::thread(&CIOCP::WorkerThreadProcess, this));
	}
	CLog::WriteLog(L"Create Worker Thread : Worker Thread Creation Successful! - %d", SysInfo.dwNumberOfProcessors * 2);
}

bool CIOCP::WorkerThreadProcess() {
	DWORD RecvBytes = 0;
	LPOVERLAPPED Overlapped = nullptr;
	void* CompletionPort = nullptr;

	SetEvent(m_hWaitForInitalization);

	while (true) {
		bool Succeed = GetQueuedCompletionStatus(m_hIOCP, &RecvBytes, reinterpret_cast<PULONG_PTR>(&CompletionPort), &Overlapped, INFINITE);

		if (!CompletionPort) {
			return false;
		}

		if (OVERLAPPED_EX* OverlappedEX = reinterpret_cast<OVERLAPPED_EX*>(Overlapped)) {
			if (!Succeed || (Succeed && RecvBytes == 0)) {
				switch (OverlappedEX->m_IOType) {
				case EIOTYPE::EIOTYPE_ACCEPT:
					if (!OnIOAccept(OverlappedEX->m_Owner)) {
						OnIODisconnect(OverlappedEX->m_Owner);
					}
					break;
				case EIOTYPE::EIOTYPE_NONE:
					CLog::WriteLog(L"Unknown IO Type!");
					break;
				default:
					CLog::WriteLog(L"Disconnect!");
					OnIODisconnect(OverlappedEX->m_Owner);
					break;
				}
				continue;
			}

			switch (OverlappedEX->m_IOType) {
			case EIOTYPE::EIOTYPE_READ:
				OnIORead(OverlappedEX->m_Owner, RecvBytes);
				break;
			case EIOTYPE::EIOTYPE_WRITE:
				OnIOWrite(OverlappedEX->m_Owner);
				break;
			default:
				CLog::WriteLog(L"Unknown IO Type!");
				break;
			}
		}
	}
	return true;
}
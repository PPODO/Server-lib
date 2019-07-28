#include "IOCP.h"
#include "../../Functions/Log/Log.h"

void WorkerThreadCallbackFunction(CIOCP* const Value) {
	if (Value) {
		Value->WorkerThreadCallback();
	}
	else{
		CLog::WriteLog(L"Worker Thrad Callback Function Param Is Invalid!");
	}
}

CIOCP::CIOCP() : m_hIOCP(INVALID_HANDLE_VALUE) {
	if (WSAStartup(WINSOCK_VERSION, &m_WinSockData) == SOCKET_ERROR) {
		CLog::WriteLog(L"Start WinSock Failure!");
	}
}

CIOCP::~CIOCP() {
	WSACleanup();
}

bool CIOCP::Initialize() {
	if (m_hIOCP != INVALID_HANDLE_VALUE) {
		CLog::WriteLog(L"IOCP is already Initialized!");
		return false;
	}
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	if (m_hIOCP == INVALID_HANDLE_VALUE) {
		CLog::WriteLog(L"Failed Create IOCP handle");
		return false;
	}

	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);

	for (size_t i = 0; i < SystemInfo.dwNumberOfProcessors * 2; i++) {
		m_WorkerThreads.push_back(std::thread(WorkerThreadCallbackFunction, this));
	}

	CLog::WriteLog(L"Initialize IOCP is Successful!");
	return true;
}

bool CIOCP::RegisterIOCompletionKey(const SOCKET& Socket, const ULONG_PTR& CompletionKey) {
	if (m_hIOCP == INVALID_HANDLE_VALUE) {
		return false;
	}
	m_hIOCP = CreateIoCompletionPort(reinterpret_cast<HANDLE>(Socket), m_hIOCP, CompletionKey, 0);
	return (m_hIOCP == INVALID_HANDLE_VALUE ? false : true);
}

bool CIOCP::Destroy() {
	for (auto& Iterator : m_WorkerThreads) {
		PostQueuedCompletionStatus(m_hIOCP, 0, 0, nullptr);
	}
	for (auto& Iterator : m_WorkerThreads) {
		Iterator.join();
	}

	if (m_hIOCP != INVALID_HANDLE_VALUE) {
		CloseHandle(m_hIOCP);
		m_hIOCP = INVALID_HANDLE_VALUE;
	}
	m_WorkerThreads.clear();
	return true;
}

void CIOCP::WorkerThreadCallback() {
	DWORD RecvBytes = 0;
	OVERLAPPED_EX* Overlapped;
	void* CompletionKey = nullptr;

	while (true) {
		bool Succeed = GetQueuedCompletionStatus(m_hIOCP, &RecvBytes, reinterpret_cast<PULONG_PTR>(&CompletionKey), reinterpret_cast<LPOVERLAPPED*>(&Overlapped), INFINITE);

		// Destroy 함수의 PostQueuedCompletionStatus에서 nullptr 값이 넘어옴. 
		if (!CompletionKey) {
			CLog::WriteLog(L"Completion Key is Invalid! - %d", WSAGetLastError());
			return;
		}

		if (!Succeed || (Succeed && RecvBytes <= 0)) {
			if (Overlapped->m_IOType == EIOTYPE::EIOTYPE_ACCEPT) {
				OnIOConnect(Overlapped->m_Owner);
				CLog::WriteLog(L"Accept New Client!");
			}
			else {
				OnIODisconnect(Overlapped->m_Owner);
				CLog::WriteLog(L"Disconnect Client!");
			}
			continue;
		}

		switch (Overlapped->m_IOType) {
		case EIOTYPE::EIOTYPE_READ:
			OnIOReceive(Overlapped->m_Owner, RecvBytes);
			break;
		case EIOTYPE::EIOTYPE_WRITE:
			OnIOWrite(Overlapped->m_Owner);
			break;
		}
	}
}
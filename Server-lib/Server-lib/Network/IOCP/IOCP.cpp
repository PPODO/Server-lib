#include "IOCP.h"
#include "../../Functions/Log/Log.h"

CIOCP::CIOCP() : m_hIOCP(INVALID_HANDLE_VALUE) {
	WSAStartup(WINSOCK_VERSION, &m_WinSockData);
}

CIOCP::~CIOCP() {
	WSACleanup();
}

bool CIOCP::Initialize() {
	if ((m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, 0, 0)) == INVALID_HANDLE_VALUE) {
		CLog::WriteLog(L"IOCP Initialize : Initialize IO Completion Port is Failure!");
		return false;
	}

	if ((m_hWaitInitializeThreadHandle = CreateEvent(nullptr, false, false, nullptr)) == INVALID_HANDLE_VALUE) {
		CLog::WriteLog(L"IOCP Initialize : Create Wait Initialize Thread Handle is Failure!");
		return false;
	}
	return CreateWorkerThreads();
}

bool CIOCP::CreateWorkerThreads() {
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);

	for (size_t i = 0; i < SystemInfo.dwNumberOfProcessors * 2; i++) {
		m_WorkerThreads.push_back(std::thread(&CIOCP::WorkerThreadCallback, this));
	}
	CLog::WriteLog(L"Create Worker Thread is Successful!");
	return true;
}

bool CIOCP::Destroy() {
	for (auto& Iterator : m_WorkerThreads) {
		PostQueuedCompletionStatus(m_hIOCP, 0, 0, nullptr);
		Iterator.join();
	}

	if (m_hIOCP) {
		CloseHandle(m_hIOCP);
	}
	if (m_hWaitInitializeThreadHandle) {
		CloseHandle(m_hWaitInitializeThreadHandle);
	}
	return true;
}

void CIOCP::WorkerThreadCallback() {
	DWORD RecvBytes = 0;
	LPOVERLAPPED Overlapped = nullptr;
	void* CompletionObject = nullptr;

	WaitForSingleObject(m_hWaitInitializeThreadHandle, INFINITE);
	while (true) {
		bool Succeed = GetQueuedCompletionStatus(m_hIOCP, &RecvBytes, reinterpret_cast<PULONG_PTR>(&CompletionObject), &Overlapped, INFINITE);

		if (!Succeed) {
			std::cout << "Failre! - " << WSAGetLastError() << std::endl;
			// Error
		}
		else if (Succeed && RecvBytes == 0) {
			std::cout << "Disconnect!\n";
			// disconnect
		}
		else if (Succeed) {
			std::cout << "Process!\n";
			// Process
		}
	}
}

bool CIOCP::RegisterCompletionPort(const SOCKET& Socket, const ULONG_PTR& CompletionKey) {
	if (Socket == INVALID_SOCKET) {
		CLog::WriteLog(L"Register IO Completion Port : Socket is Invalid!");
		return false;
	}
	if ((m_hIOCP = CreateIoCompletionPort(reinterpret_cast<HANDLE>(Socket), m_hIOCP, CompletionKey, 0)) == INVALID_HANDLE_VALUE) {
		CLog::WriteLog(L"Register IO Completion Port : Create IO Completion Port is Failure!");
		return false;
	}
	return true;
}
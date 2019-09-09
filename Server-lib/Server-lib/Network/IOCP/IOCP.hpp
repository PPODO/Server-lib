#pragma once
#include "../PacketSession/PacketSession.h"
#include "../../Functions/CircularQueue/CircularQueue.h"
#include <vector>
#include <thread>
#include <array>

template<typename SESSIONTYPE>
class CIOCP {
private:
	WSADATA m_WinSockData;

private:
	HANDLE m_hIOCP;
	HANDLE m_hWaitForInitialize;

private:
	CPacketSession* m_ListenSession;

private:
	std::vector<std::thread> m_WorkerThread;

private:
	CCircularQueue<PACKET_DATA*> m_PacketQueue;
	CCircularQueue<BROADCAST_DATA<SESSIONTYPE>*> m_SendQueue;

private:
	bool CreateWorkerThread();
	bool ProcessWorkerThread();
	bool ProcessPacketThread();
	bool ProcessPacketSendThread();

protected:
	virtual bool OnIOConnect(void* const Object) = 0;
	virtual bool OnIODisconnect(void* const Object) = 0;
	virtual bool OnIORead(void* const Object, uint16_t& RecvBytes) = 0;
	virtual bool OnIOWrite(void* const Object);
	virtual bool ProcessPacket(PACKET_DATA& PacketData) = 0;
	virtual bool ProcessBroadCast(BROADCAST_DATA<SESSIONTYPE>& Data) = 0;

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
	inline bool AddNewDataAtPacketQueue(PACKET_DATA* const NewPacketData) {
		if (NewPacketData) {
			m_PacketQueue.Push(NewPacketData);
			return true;
		}
		return false;
	}
	inline bool AddNewDataAtBroadCastQueue(BROADCAST_DATA<SESSIONTYPE>* const NewData) {
		if (NewData) {
			m_SendQueue.Push(NewData);
			return true;
		}
		return false;
	}

protected:
	inline HANDLE GetIOCPHandle() const { return m_hIOCP; }

protected:
	template<typename T>
	T* const GetListenSession() const { return reinterpret_cast<T* const>(m_ListenSession); };

public:
	CIOCP() : m_hIOCP(INVALID_HANDLE_VALUE), m_hWaitForInitialize(INVALID_HANDLE_VALUE), m_ListenSession(nullptr) {};

public:
	virtual bool Initialize();
	virtual bool Destroy();
	virtual bool UpdateMainThread();

};

template<typename SESSIONTYPE>
bool CIOCP<SESSIONTYPE>::Initialize() {
	if (!std::is_base_of<CNetworkSession, SESSIONTYPE>()) {
		CLog::WriteLog(L"Initialize IOCP : Only Classes That Inherit Sessions Are Supported!");
		return false;
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

	if (!CreateWorkerThread()) {
		CLog::WriteLog(L"Create Worker Thread : Failed To Create Worker Thread!");
		return false;
	}

	m_ListenSession = new SESSIONTYPE;
	if (!m_ListenSession) {
		CLog::WriteLog(L"Initialize IOCP : Failed To Create Listen Session!");
		return false;
	}

	WaitForSingleObject(m_hWaitForInitialize, INFINITE);

	CLog::WriteLog(L"Initialize IOCP : Initialization Successful!");
	return true;
}

template<typename SESSIONTYPE>
bool CIOCP<SESSIONTYPE>::Destroy() {
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

	if (m_ListenSession) {
		m_ListenSession->Destroy();
		delete m_ListenSession;
		m_ListenSession = nullptr;
	}
	WSACleanup();
	return true;
}

template<typename SESSIONTYPE>
bool CIOCP<SESSIONTYPE>::UpdateMainThread() {
	while (true) {
		ProcessPacketThread();
		ProcessPacketSendThread();
	}
	return true;
}

template<typename SESSIONTYPE>
bool CIOCP<SESSIONTYPE>::CreateWorkerThread() {
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);

	size_t ProcessorCount = SysInfo.dwNumberOfProcessors * 2;
	for (size_t i = 0; i < ProcessorCount; i++) {
		m_WorkerThread.push_back(std::thread(&CIOCP::ProcessWorkerThread, this));
	}
	CLog::WriteLog(L"Create Worker Thread : Worker Thread Creation Successful! - %d", SysInfo.dwNumberOfProcessors * 2);
	return true;
}

template<typename SESSIONTYPE>
bool CIOCP<SESSIONTYPE>::ProcessWorkerThread() {
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
				case EIOTYPE::EIOTYPE_WRITE:
					break;
				default:
					OnIODisconnect(OverlappedEx->m_Owner);
					break;
				}
				continue;
			}

			switch (OverlappedEx->m_IOType) {
			case EIOTYPE::EIOTYPE_READ:
				OnIORead(OverlappedEx->m_Owner, reinterpret_cast<uint16_t&>(RecvBytes));
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

template<typename SESSIONTYPE>
bool CIOCP<SESSIONTYPE>::ProcessPacketThread() {
	while (!m_PacketQueue.IsEmpty()) {
		PACKET_DATA* Data = nullptr;
		if (m_PacketQueue.Pop(Data) && Data) {
			ProcessPacket(*Data);

			delete Data;
		}
	}
	return false;
}

template<typename SESSIONTYPE>
bool CIOCP<SESSIONTYPE>::ProcessPacketSendThread() {
	while (!m_SendQueue.IsEmpty()) {
		BROADCAST_DATA<SESSIONTYPE>* Data = nullptr;
		if (m_SendQueue.Pop(Data) && Data) {
			ProcessBroadCast(*Data);
			delete Data;
		}
	}
	return false;
}

template<typename SESSIONTYPE>
bool CIOCP<SESSIONTYPE>::OnIOWrite(void* const Object) {
	if (CPacketSession * Client = reinterpret_cast<CPacketSession*>(Object)) {
		Client->WriteCompletion();
	}
	return false;
}
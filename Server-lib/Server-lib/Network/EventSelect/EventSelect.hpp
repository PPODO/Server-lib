#pragma once
#include "../PacketSession/PacketSession.h"
#include <thread>

template<typename SESSIONTYPE>
class CEventSelect {
private:
	WSADATA m_WinSockData;

private:
	SESSIONTYPE* m_Session;
	SOCKET m_SessionSocket;

private:
	std::thread m_EventSelectThread;

private:
	HANDLE m_hSelectHandle;
	HANDLE m_hWaitForInitialize;
	HANDLE m_hStopEvent;

private:
	bool ProcessEventSelect();

protected:
	//virtual bool OnIOAccept();
	//virtual bool OnIOConnect();
	//virtual bool OnIODisconnect();
	virtual bool OnIORead();
	//virtual bool OnIOWrite();

public:
	CEventSelect() : m_Session(nullptr), m_hSelectHandle(INVALID_HANDLE_VALUE), m_hStopEvent(INVALID_HANDLE_VALUE), m_hWaitForInitialize(INVALID_HANDLE_VALUE) {};

public:
	virtual bool Initialize(const CSocketAddress& ConnectionAddress);
	virtual bool Destroy();

};

template<typename SESSIONTYPE>
bool CEventSelect<SESSIONTYPE>::Initialize(const CSocketAddress& ConnectionAddress) {
	if (!std::is_base_of<CNetworkSession, SESSIONTYPE>()) {
		CLog::WriteLog(L"Initialize Event Select : Only Classes That Inherit Sessions Are Supported!");
		return false;
	}

	if (WSAStartup(WINSOCK_VERSION, &m_WinSockData) == SOCKET_ERROR) {
		CLog::WriteLog(L"Initialize Event Select : WSAStartup Failure!");
		return false;
	}

	m_Session = new SESSIONTYPE;
	if (CPacketSession * TempSession = reinterpret_cast<CPacketSession*>(m_Session)) {
		if (!TempSession->Initialize(EPROTOCOLTYPE::EPT_TCP) || CSocketSystem::Connect(TempSession->GetTCPSocket(), ConnectionAddress)) {
			return false;
		}
		m_SessionSocket = CSocketSystem::GetSocketByClass(TempSession->GetTCPSocket());

		if (!(m_hSelectHandle = WSACreateEvent())) {
			CLog::WriteLog(L"Initialize Event Select : WSACreateEvent Failure!");
			return false;
		}

		if (WSAEventSelect(m_SessionSocket, m_hSelectHandle, FD_ACCEPT | FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR) {
			CLog::WriteLog(L"Initialize Event Select : WSAEventSelect Failure!");
			return false;
		}

		if (!(m_hStopEvent = CreateEvent(nullptr, false, false, nullptr))) {
			CLog::WriteLog(L"Initialize IOCP : Failed To Create Stop Event!");
			return false;
		}

		if (!(m_hWaitForInitialize = CreateEvent(nullptr, false, false, nullptr))) {
			CLog::WriteLog(L"Initialize IOCP : Failed To Create Wait Event!");
			return false;
		}
	}

	m_EventSelectThread = std::thread(&CEventSelect::ProcessEventSelect, this);

	WaitForSingleObject(m_hWaitForInitialize, INFINITE);
	return true;
}

template<typename SESSIONTYPE>
bool CEventSelect<SESSIONTYPE>::Destroy() {
	SetEvent(m_hStopEvent);

	if (m_EventSelectThread.joinable()) {
		m_EventSelectThread.join();
	}

	m_SessionSocket = INVALID_SOCKET;

	if (m_hWaitForInitialize) {
		CloseHandle(m_hWaitForInitialize);
		m_hWaitForInitialize = INVALID_HANDLE_VALUE;
	}

	if (m_hStopEvent) {
		CloseHandle(m_hStopEvent);
		m_hStopEvent = INVALID_HANDLE_VALUE;
	}

	if (WSACloseEvent(m_hSelectHandle)) {
		m_hSelectHandle = INVALID_HANDLE_VALUE;
	}

	if (CPacketSession* TempSession = reinterpret_cast<CPacketSession*>(m_Session)) {
		TempSession->Destroy();
		delete m_Session;
		m_Session = nullptr;
	}
	WSACleanup();
	return true;
}

template<typename SESSIONTYPE>
bool CEventSelect<SESSIONTYPE>::ProcessEventSelect() {
	HANDLE SelectEvents[2] = { m_hStopEvent, m_hSelectHandle };
	DWORD SelectResult = 0;
	WSANETWORKEVENTS Events;

	SetEvent(m_hWaitForInitialize);
	while (true) {
		SelectResult = WaitForMultipleObjects(2, SelectEvents, false, INFINITE);

		switch (SelectResult) {
		case WAIT_OBJECT_0:
			return false;

		case WAIT_OBJECT_0 + 1:
			WSAEnumNetworkEvents(m_SessionSocket, m_hSelectHandle, &Events);

			if (Events.lNetworkEvents | FD_ACCEPT) {
			//	OnIOAccept();
			}
			else if (Events.lNetworkEvents | FD_CONNECT) {
			//	OnIOConnect();
			}
			else if (Events.lNetworkEvents | FD_CLOSE) {
			//	OnIODisconnect();
			}
			else if (Events.lNetworkEvents | FD_READ) {
				OnIORead();
			}
			else if (Events.lNetworkEvents | FD_WRITE) {
				// 사용하지 않음
				//OnIOWrite();
			}
			else {
				CLog::WriteLog(L"Select Event : Unknown Network Event!");
			}
			break;
		default:
			break;
		}
	}
	return true;
}

template<typename SESSIONTYPE>
bool CEventSelect<SESSIONTYPE>::OnIORead() {
	if (CPacketSession * TempSession = reinterpret_cast<CPacketSession*>(m_Session)) {
		if (TempSession->ReceiveEventSelect()) {
			if (PACKET_DATA* NewPacketData = TempSession->PacketAnalysis()) {
				
				return true;
			}
		}
	}
	return false;
}
#include "EventSelect.h"

bool CEventSelect::Initialize(const CSocketAddress& ConnectionAddress, const EPROTOCOLTYPE& ProtocolType) {
	if (ProtocolType == EPROTOCOLTYPE::EPT_TCP) { m_SessionSocket = CSocketSystem::GetSocketByClass(m_Session->GetTCPSocket()); }
	else if (ProtocolType == EPROTOCOLTYPE::EPT_UDP) { m_SessionSocket = CSocketSystem::GetSocketByClass(m_Session->GetUDPSocket()); }
	else { CLog::WriteLog(L"Initialize Event Select : Protocol Type is NONE! "); return false; }

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

	m_EventSelectThread = std::thread(&CEventSelect::ProcessEventSelect, this);

	WaitForSingleObject(m_hWaitForInitialize, INFINITE);
	return true;
}

bool CEventSelect::Destroy() {
	SetEvent(m_hStopEvent);

	if (m_EventSelectThread.joinable()) {
		m_EventSelectThread.join();
	}

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

	m_SessionSocket = INVALID_SOCKET;
	if (CPacketSession * TempSession = reinterpret_cast<CPacketSession*>(m_Session)) {
		TempSession->Destroy();
		delete m_Session;
		m_Session = nullptr;
	}
	WSACleanup();
	return true;
}

bool CEventSelect::ProcessEventSelect() {
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

			if (Events.lNetworkEvents & FD_ACCEPT) {
				//	OnIOAccept();
			}
			else if (Events.lNetworkEvents & FD_CONNECT) {
				//	OnIOConnect();
			}
			else if (Events.lNetworkEvents & FD_CLOSE) {
				//	OnIODisconnect();
			}
			else if (Events.lNetworkEvents & FD_READ) {
				OnIORead();
			}
			else if (Events.lNetworkEvents & FD_WRITE) {
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
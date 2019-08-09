#include "EventSelect.h"
#include "../../Functions/Log/Log.h"

CEventSelect::CEventSelect(const SOCKET& ListenSocket) : m_ListenSocket(ListenSocket), m_hSelectHandle(INVALID_HANDLE_VALUE), m_hDestroyHandle(INVALID_HANDLE_VALUE), m_hWaitInitializeThreadHandle(INVALID_HANDLE_VALUE) {
}

bool CEventSelect::Initialize() {
	if (m_ListenSocket == INVALID_SOCKET) {
		CLog::WriteLog(L"Initialize EventSelect : Listen Socket is Invalid!");
		return false;
	}

	if ((m_hSelectHandle = WSACreateEvent()) == WSA_INVALID_EVENT) {
		CLog::WriteLog(L"Initialize EventSelect : WSA Create Event is Failure!");
		return false;
	}

	if (WSAEventSelect(m_ListenSocket, m_hSelectHandle, FD_CONNECT | FD_ACCEPT | FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR) {
		CLog::WriteLog(L"Initialize EventSelect : Initialize WSAEventSelect is Failure!");
		return false;
	}

	if ((m_hDestroyHandle = CreateEvent(nullptr, false, false, nullptr)) == INVALID_HANDLE_VALUE) {
		CLog::WriteLog(L"Initialize EventSelect : Create Destroy Event is Failure!");
		return false;
	}

	if ((m_hWaitInitializeThreadHandle = CreateEvent(nullptr, false, false, nullptr)) == INVALID_HANDLE_VALUE) {
		CLog::WriteLog(L"Initialize EventSelect : Create Wait Initialize Thread Event is Failure!");
		return false;
	}

	m_EventSelectThreadHandle = std::thread(&CEventSelect::EventSelectThreadCallback, this);

	WaitForSingleObject(m_hWaitInitializeThreadHandle, INFINITE);
	return true;
}

bool CEventSelect::Destroy() {
	SetEvent(m_hDestroyHandle);
	m_EventSelectThreadHandle.join();

	if (m_hWaitInitializeThreadHandle) {
		CloseHandle(m_hWaitInitializeThreadHandle);
	}
	if (m_hDestroyHandle) {
		CloseHandle(m_hDestroyHandle);
	}
	if (m_hSelectHandle) {
		CloseHandle(m_hSelectHandle);
	}
	return true;
}

void CEventSelect::EventSelectThreadCallback() {
	WSANETWORKEVENTS Events;
	HANDLE EventHandle[2] = { m_hDestroyHandle, m_hSelectHandle };
	DWORD EventID = 0;

	if (m_ListenSocket == INVALID_SOCKET) {
		CLog::WriteLog(L"Initialize EventSelect : Listen Socket is Invalid!");
		return;
	}

	SetEvent(m_hWaitInitializeThreadHandle);
	while (true) {
		EventID = WaitForMultipleObjects(2, EventHandle, false, INFINITE);

		switch (EventID) {
		case WAIT_OBJECT_0: // Destroy Event
			return;
		case WAIT_OBJECT_0 + 1: // Select Event;
			WSAEnumNetworkEvents(m_ListenSocket, m_hSelectHandle, &Events);

			if (Events.lNetworkEvents | FD_CONNECT) {
				OnIOConnect();
			}
			else if (Events.lNetworkEvents | FD_ACCEPT) {
				OnIOAccpet();
			}
			else if (Events.lNetworkEvents | FD_READ) {
				OnIORead();
			}
			else if (Events.lNetworkEvents | FD_WRITE) {
				OnIOWrite();
			}
			else if (Events.lNetworkEvents | FD_CLOSE) {
				OnIODisconnect();
			}
			break;
		default:
			return;
		}
	}
}
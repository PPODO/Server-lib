#include "TCPIPSocket.h"
#include "../../../Functions/Log/Log.h"

CTCPIPSocket::CTCPIPSocket() : m_Socket(INVALID_SOCKET) {}

CTCPIPSocket::~CTCPIPSocket() {}

bool CTCPIPSocket::BindTCP() {
	if (m_Socket != INVALID_SOCKET) {
		CLog::WriteLog(L"TCP Socket is already exist!");
		return false;
	}

	m_Socket = CSocketUtil::CreateTCPSocket();
	if (m_Socket == INVALID_SOCKET) {
		CLog::WriteLog(L"Bind TCP Socket Failure!");
		return false;
	}

	ZeroMemory(m_ReciveBuffer, MAX_RECIVE_BUFFER_LENGTH);

	CLog::WriteLog(L"Bind TCP Socket Successful!");
	return true;
}

bool CTCPIPSocket::Connect(const CSocketAddress& ConnectionAddress) {
	if (m_Socket == INVALID_SOCKET) {
		CLog::WriteLog(L"Connection Failure! - Socket is invalid");
		return false;
	}

	INT Result =  WSAConnect(m_Socket, &ConnectionAddress.m_Address, ConnectionAddress.GetSize(), nullptr, nullptr, nullptr, nullptr);
	if (Result != WSAEWOULDBLOCK) {
		return false;
	}

	CLog::WriteLog(L"Connection Successful!");
	return true;
}

bool CTCPIPSocket::Listen(const CSocketAddress& BindAddress, const INT& BackLogCount) {
	if (m_Socket == INVALID_SOCKET && BackLogCount <= 0) {
		CLog::WriteLog(L"Listen Failure! - Socket is invalid or BackLog Count is less or same than 0");
		return false;
	}

	if (bind(m_Socket, &BindAddress.m_Address, BindAddress.GetSize()) == SOCKET_ERROR) {
		CLog::WriteLog(L"bind Failure! - %d", WSAGetLastError());
		return false;
	}

	if (listen(m_Socket, BackLogCount) == SOCKET_ERROR) {
		CLog::WriteLog(L"listen Failure! - %d", WSAGetLastError());
		return false;
	}

	CLog::WriteLog(L"Listen Successful!");
	return true;
}

bool CTCPIPSocket::Accept(const SOCKET & ListenSocket, OVERLAPPED_EX& AcceptOverlapped) {
	if (ListenSocket == INVALID_SOCKET) {
		CLog::WriteLog(L"Accept Failure! - Listen Socket is invalid");
		return false;
	}
	if (m_Socket != INVALID_SOCKET) {
		CLog::WriteLog(L"Accept Failure! - Socket is exist");
		return false;
	}

	m_Socket = CSocketUtil::CreateTCPSocket();

	if (!AcceptEx(ListenSocket, m_Socket, nullptr, 0, CSocketAddress::GetSize() + 16, CSocketAddress::GetSize() + 16, nullptr, &AcceptOverlapped.m_Overlapped)) {
		if (WSAGetLastError() != WSAEWOULDBLOCK && WSAGetLastError() != WSA_IO_PENDING) {
			CLog::WriteLog(L"Accept Failure! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

bool CTCPIPSocket::Read(struct OVERLAPPED_EX& ReciveOverlapped) {
	if (m_Socket == INVALID_SOCKET) {
		return false;
	}

	DWORD RecvBytes = 0, Flag = 0;

	WSABUF RecvBuffer;
	RecvBuffer.buf = reinterpret_cast<CHAR*>(m_ReciveBuffer);
	RecvBuffer.len = MAX_RECIVE_BUFFER_LENGTH;

	if (WSARecv(m_Socket, &RecvBuffer, 1, &RecvBytes, &Flag, &ReciveOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			return true;
		}
	}
	return false;
}

bool CTCPIPSocket::Write(const CHAR* Data, const UINT16& DataLength, struct OVERLAPPED_EX& SendOverlapped) {
	if (m_Socket == INVALID_SOCKET || !Data || DataLength <= 0) {
		return false;
	}

	DWORD SendBytes = 0;

	WSABUF SendBuffer;
	SendBuffer.buf = const_cast<CHAR*>(Data);
	SendBuffer.len = DataLength;

	if (WSASend(m_Socket, &SendBuffer, 1, &SendBytes, 0, &SendOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			return true;
		}
	}
	return false;
}

void CTCPIPSocket::Shutdown() {
	if (m_Socket != INVALID_SOCKET) {
		shutdown(m_Socket, SD_BOTH);
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}

	ZeroMemory(m_ReciveBuffer, MAX_RECIVE_BUFFER_LENGTH);
}
#include "TCPIPSocket.h"
#include "../../../Functions/Log/Log.h"
#include <MSWSock.h>

CTCPIPSocket::CTCPIPSocket() : m_Socket() {}

CTCPIPSocket::~CTCPIPSocket() {}

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

void CTCPIPSocket::Shutdown() {
	if (m_Socket != INVALID_SOCKET) {
		shutdown(m_Socket, SD_BOTH);
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}
}
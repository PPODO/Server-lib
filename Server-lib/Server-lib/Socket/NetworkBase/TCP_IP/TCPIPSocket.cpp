#include "TCPIPSocket.h"
#include "../../../Functions/Log/Log.h"

CTCPIPSocket::CTCPIPSocket() : m_Socket(INVALID_SOCKET) { ZeroMemory(m_ReceiveBuffer, MAX_Receive_BUFFER_LENGTH); }

CTCPIPSocket::~CTCPIPSocket() {}

bool CTCPIPSocket::BindTCP() {
	if (m_Socket != INVALID_SOCKET) {
		CLog::WriteLog(L"Bind - TCP Socket Is Already Exist!");
		return false;
	}
	if ((m_Socket = CSocketUtil::CreateTCPSocket()) == INVALID_SOCKET) {
		CLog::WriteLog(L"Bind - Create TCP Socket Is Failure!");
		return false;
	}

	CLog::WriteLog(L"Bind - Create TCP Socket Is Successful!");
	return true;
}

bool CTCPIPSocket::Listen(const CSocketAddress & ListenAddress, const UINT16 & BackLogCount) {
	if (m_Socket == INVALID_SOCKET || BackLogCount <= 0) {
		CLog::WriteLog(L"Listen - Listen Socket is Invalid or BackLog Count less than 0");
		return false;
	}
	if (bind(m_Socket, &ListenAddress.m_Address, ListenAddress.GetSize()) == SOCKET_ERROR) {
		CLog::WriteLog(L"Listen - Bind Socket Is Failure!");
		return false;
	}
	if (listen(m_Socket, BackLogCount) == SOCKET_ERROR) {
		CLog::WriteLog(L"Listen - Listen Socket Is Failure!");
		return false;
	}

	CLog::WriteLog(L"Listen - Listen Socket Is Successful!");
	return true;
}

bool CTCPIPSocket::Connect(const CSocketAddress & ConnectionAddress) {
	if (m_Socket == INVALID_SOCKET) {
		CLog::WriteLog(L"Connect - Socket Is Invalid!");
		return false;
	}

	if (WSAConnect(m_Socket, &ConnectionAddress.m_Address, ConnectionAddress.GetSize(), nullptr, nullptr, 0, 0) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"Connect - Connection Failure : %d", WSAGetLastError());
			return false;
		}
	}

	CLog::WriteLog(L"Connet - Succeed To Connect the Server");
	return true;
}

bool CTCPIPSocket::Accept(const CTCPIPSocket & ListenSocket, OVERLAPPED_EX & AcceptOverlapped) {
	if (ListenSocket.m_Socket == INVALID_SOCKET) {
		CLog::WriteLog(L"Accept - Listen Socket Is Invalid!");
		return false;
	}
	if (m_Socket != INVALID_SOCKET) {
		CLog::WriteLog(L"Accept - Accept Socket Is Already Initialized!");
		return false;
	}
	
	if ((m_Socket = CSocketUtil::CreateTCPSocket()) == INVALID_SOCKET) {
		CLog::WriteLog(L"Accept - Create Accept Socket Is Failure!");
		return false;
	}

	if (!AcceptEx(ListenSocket.m_Socket, m_Socket, m_ReceiveBuffer, 0, CSocketAddress::GetSize() + 16, CSocketAddress::GetSize() + 16, nullptr, &AcceptOverlapped.m_Overlapped)) {
		if (WSAGetLastError() != WSAEWOULDBLOCK && WSAGetLastError() != WSA_IO_PENDING) {
			CLog::WriteLog(L"Accept - AcceptEx Failure! : %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

bool CTCPIPSocket::Shutdown() {
	if (m_Socket != INVALID_SOCKET) {
		shutdown(m_Socket, SD_BOTH);
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		CLog::WriteLog(L"HI");
	}
	return true;
}

bool CTCPIPSocket::InitializeRecvBuffer_IOCP(struct OVERLAPPED_EX& RecvOverlapped) {
	DWORD RecvBytes = 0, Flag = 0;
	WSABUF RecvBuffer;
	RecvBuffer.buf = m_ReceiveBuffer;
	RecvBuffer.len = MAX_Receive_BUFFER_LENGTH;

	if (WSARecv(m_Socket, &RecvBuffer, 1, &RecvBytes, &Flag, &RecvOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"Recv - WSARecv Failure! : %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

bool CTCPIPSocket::CopyRecvBuffer_IOCP(CHAR * InData, const UINT16 & DataLength) {
	if (!InData || DataLength <= 0) {
		CLog::WriteLog(L"Recv - InData Buffer Is Invalid or DataLength <= 0");
		return false;
	}

	CopyMemory(InData, m_ReceiveBuffer, DataLength);
	return true;
}

bool CTCPIPSocket::ReadRecvBuffer_Select(CHAR * InData, UINT16 & RecvLength, struct OVERLAPPED_EX& RecvOverlapped) {
	if (!InData) {
		CLog::WriteLog(L"Recv - InData Buffer Is Invalid");
		return false;
	}

	DWORD RecvBytes = 0, Flag = 0;
	WSABUF RecvBuffer;
	RecvBuffer.buf = InData;
	RecvBuffer.len = MAX_Receive_BUFFER_LENGTH;

	if (WSARecv(m_Socket, &RecvBuffer, 1, &RecvBytes, &Flag, &RecvOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"Recv - WSARecv Failure! : %d", WSAGetLastError());
			return false;
		}
	}

	CopyMemory(InData, m_ReceiveBuffer, RecvBytes);
	RecvLength = RecvBytes;
	return true;
}

bool CTCPIPSocket::Write(const CHAR * OutData, const UINT16 & DataLength, struct OVERLAPPED_EX& SendOverlapped) {
	if (!OutData || DataLength <= 0) {
		CLog::WriteLog(L"Send - OutData Buffer Is Invalid or DataLength <= 0");
		return false;
	}

	DWORD SendBytes = 0;
	WSABUF SendBuffer;
	SendBuffer.buf = const_cast<CHAR*>(OutData);
	SendBuffer.len = DataLength;

	if (WSASend(m_Socket, &SendBuffer, 1, &SendBytes, 0, &SendOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"Send - WSASend Failure! : %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}
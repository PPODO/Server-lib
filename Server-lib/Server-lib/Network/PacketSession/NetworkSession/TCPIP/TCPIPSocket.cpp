#include "TCPIPSocket.h"
#include "../../../../Functions/Log/Log.h"
#include <MSWSock.h>
#pragma comment(lib, "mswsock.lib")

CTCPIPSocket::CTCPIPSocket() : m_Socket(INVALID_SOCKET) {
	ZeroMemory(m_IOCPReceiveBuffer, MAX_RECEIVE_BUFFER_LENGTH);
}

bool CTCPIPSocket::Initialize() {
	if (m_Socket != INVALID_SOCKET) {
		CLog::WriteLog(L"Initialize Socket : Socket	Already Initialized!");
		return false;
	}
	if ((m_Socket = CSocketUtil::CreateTCPSocket()) == INVALID_SOCKET) {
		CLog::WriteLog(L"Initialize Socket : Socket Creation Failure!");
		return false;
	}
	return true;
}

bool CTCPIPSocket::Listen(const CSocketAddress& BindAddress, const USHORT& BackLogCount) {
	if (m_Socket == INVALID_SOCKET) {
		CLog::WriteLog(L"Listen Socket : Invalid Socket!");
		return false;
	}

	if (bind(m_Socket, &BindAddress.m_Address, BindAddress.GetSize()) == SOCKET_ERROR) {
		CLog::WriteLog(L"Listen Socket : Bind Failure!");
		return false;
	}
	
	if (listen(m_Socket, BackLogCount) == SOCKET_ERROR) {
		CLog::WriteLog(L"Listen Socket : Listen Failure!");
		return false;
	}
	return true;
}

bool CTCPIPSocket::Connect(const CSocketAddress& ConnectionAddress) {
	if (m_Socket == INVALID_SOCKET) {
		CLog::WriteLog(L"Connect To Server : Invalid Socket!");
		return false;
	}

	if (WSAConnect(m_Socket, &ConnectionAddress.m_Address, ConnectionAddress.GetSize(), nullptr, nullptr, nullptr, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"Connect To Server : Connection Failure! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

bool CTCPIPSocket::Accept(const SOCKET& ListenSocket, OVERLAPPED_EX& AcceptOverlapped) {
	if (ListenSocket == INVALID_SOCKET) {
		CLog::WriteLog(L"Accept Client : Invalid Listen Socket!");
		return false;
	}

	if (m_Socket != INVALID_SOCKET) {
		int Flag = 0;
		socklen_t Size = sizeof(Flag);
		if (getsockopt(m_Socket, SOL_SOCKET, SO_ACCEPTCONN, reinterpret_cast<char*>(&Flag), &Size) == SOCKET_ERROR) {
			CLog::WriteLog(L"Accept Client : Failed To Get Socket Option!");
			return false;
		}
		if (Flag == 1) {
			CLog::WriteLog(L"Accept Client : The Socket is Listening!");
			return false;
		}
	}
	else {
		if ((m_Socket = CSocketUtil::CreateTCPSocket()) == INVALID_SOCKET) {
			CLog::WriteLog(L"Accept Client : Socket Creation Failure!");
			return false;
		}
	}

	CHAR Temp;
	if (!AcceptEx(ListenSocket, m_Socket, &Temp, 0, CSocketAddress::GetSize() + 16, CSocketAddress::GetSize() + 16, nullptr, &AcceptOverlapped.m_Overlapped)) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"Accept Client : Accept Failure! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

bool CTCPIPSocket::Destroy() {
	if (m_Socket != INVALID_SOCKET) {
		shutdown(m_Socket, SD_BOTH);
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}
	return true;
}

bool CTCPIPSocket::ReadForIOCP(OVERLAPPED_EX & RecvOverlapped) {
	if (m_Socket == INVALID_SOCKET) {
		CLog::WriteLog(L"Read IOCP : Invalid Socket!");
		return false;
	}

	DWORD RecvBytes = 0, Flags = 0;
	WSABUF RecvBuffer;
	RecvBuffer.buf = m_IOCPReceiveBuffer;
	RecvBuffer.len = MAX_RECEIVE_BUFFER_LENGTH;

	if (WSARecv(m_Socket, &RecvBuffer, 1, &RecvBytes, &Flags, &RecvOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"Read IOCP : Receive Failure! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

bool CTCPIPSocket::CopyIOCPBuffer(CHAR * OutDataBuffer, const USHORT & DataLength) {
	if (!OutDataBuffer) {
		CLog::WriteLog(L"Copy IOCP Buffer : DataBuffer Is nullptr!");
		return false;
	}

	CopyMemory(OutDataBuffer, m_IOCPReceiveBuffer, DataLength);
	return true;
}

bool CTCPIPSocket::Write(const CHAR * OutDataBuffer, const USHORT & DataLength, OVERLAPPED_EX & SendOverlapped) {
	if (m_Socket == INVALID_SOCKET) {
		CLog::WriteLog(L"Write : Invalid Socket!");
		return false;
	}

	if (!OutDataBuffer && DataLength <= 0) {
		CLog::WriteLog(L"Write : DataBuffer Is nullptr Or Data Length Is Less Than Zero!");
		return false;
	}

	DWORD SendBytes = 0;
	WSABUF SendBuffer;
	SendBuffer.buf = const_cast<CHAR*>(OutDataBuffer);
	SendBuffer.len = DataLength;

	if (WSASend(m_Socket, &SendBuffer, 1, &SendBytes, 1, &SendOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"Write : Send Failure! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}
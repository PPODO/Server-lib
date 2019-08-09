#include "TCPIPSocket.h"
#include "../../../../Functions/SocketUtil/SocketUtil.h"
#include "../../../../Functions/Log/Log.h"
#include <MSWSock.h>
#pragma comment(lib, "mswsock.lib")

CTCPIPSocket::CTCPIPSocket() : m_Socket(INVALID_SOCKET) {
}

bool CTCPIPSocket::Initialize() {
	if (m_Socket != INVALID_SOCKET) {
		CLog::WriteLog(L"Initialize TCP : TCP Socket is Already Initialized!");
		return false;
	}

	if ((m_Socket = CSocketUtil::CreateTCPSocket()) == INVALID_SOCKET) {
		CLog::WriteLog(L"Initialize TCP : Create TCP Socket is Failure! - %d", WSAGetLastError());
		return false;
	}
	return true;
}

bool CTCPIPSocket::Destroy() {
	if (m_Socket) {
		shutdown(m_Socket, SD_BOTH);
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}
	return true;
}

bool CTCPIPSocket::Listen(const CSocketAddress& BindAddress, const USHORT& MaxBackLogCount) {
	if (MaxBackLogCount <= 0) {
		CLog::WriteLog("Listen TCP : Backlog Count Cannot Be Less Than Zero!");
		return false;
	}

	if (bind(m_Socket, &BindAddress.m_Address, BindAddress.GetSize()) == SOCKET_ERROR) {
		CLog::WriteLog(L"Bind TCP : TCP Socket Bind is Failure! - %d", WSAGetLastError());
		return false;
	}

	if (listen(m_Socket, MaxBackLogCount) == SOCKET_ERROR) {
		CLog::WriteLog(L"Listen TCP : TCP Socket Listen is Failure! - %d", WSAGetLastError());
		return false;
	}
	return true;
}

bool CTCPIPSocket::Accept(const SOCKET& ListenSocket, OVERLAPPED_EX& AcceptOverlapped) {
	if (m_Socket != INVALID_SOCKET) {
		CLog::WriteLog(L"Accept TCP : TCP Socket is Already Initialized!");
		return false;
	}
	
	if ((m_Socket = CSocketUtil::CreateTCPSocket()) == INVALID_SOCKET) {
		CLog::WriteLog(L"Accept TCP : Create TCP Socket is Failure! - %d", WSAGetLastError());
		return false;
	}

	if (AcceptEx(ListenSocket, m_Socket, nullptr, 0, CSocketAddress::GetSize() + 16, CSocketAddress::GetSize() + 16, nullptr, &AcceptOverlapped.m_Overlapped) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"Accept TCP  : AcceptEx is Failure! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

bool CTCPIPSocket::InitializeReceiveForIOCP(CHAR* RecvData, OVERLAPPED_EX & RecvOverlapped) {
	if (!RecvData) {
		CLog::WriteLog(L"Recv For IOCP : Recv Data Buffer is nullptr!");
		return false;
	}

	DWORD RecvBytes = 0, Flags = 0;
	WSABUF RecvBuffer;
	RecvBuffer.buf = RecvData;
	RecvBuffer.len = MAX_RECEIVE_BUFFER_LENGTH;

	if (WSARecv(m_Socket, &RecvBuffer, 1, &RecvBytes, &Flags, &RecvOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"Recv For IOCP : WSARecv is Failure! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

bool CTCPIPSocket::ReceiveForEventSelect(CHAR * RecvData, USHORT & DataLength, OVERLAPPED_EX & RecvOverlapped) {
	if (!m_Socket) {
		CLog::WriteLog(L"Recv For Event Select : Socket Is Failure!");
		return false;
	}
	if (!RecvData || DataLength <= 0) {
		CLog::WriteLog(L"Recv For Event Select : Recv Data Buffer is nullptr!");
		return false;
	}

	DWORD RecvBytes = 0, Flags = 0;
	WSABUF RecvBuffer;
	RecvBuffer.buf = RecvData;
	RecvBuffer.len = MAX_RECEIVE_BUFFER_LENGTH;

	if (WSARecv(m_Socket, nullptr, 1, &RecvBytes, &Flags, &RecvOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"Send Data : WSASend is Failure! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

bool CTCPIPSocket::Write(const CHAR* SendData, const USHORT& DataLength, OVERLAPPED_EX& SendOverlapped) {
	if (!SendData || DataLength <= 0) {
		CLog::WriteLog(L"Send Data : Send Data Buffer is nullptr or Data Length is Less Than Zero!");
		return false;
	}

	DWORD SendBytes = 0;
	WSABUF SendBuffer;
	SendBuffer.buf = const_cast<CHAR*>(SendData);
	SendBuffer.len = DataLength;

	if (WSASend(m_Socket, &SendBuffer, 1, &SendBytes, 0, &SendOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"Send Data : WSASend is Failure! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}
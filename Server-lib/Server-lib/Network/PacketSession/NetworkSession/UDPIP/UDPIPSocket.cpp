#include "UDPIPSocket.h"
#include "../../../../Functions/Log/Log.h"
#include "../../../../Functions/SocketUtil/SocketUtil.h"

bool CUDPIPSocket::Initialize(const USHORT& Port) {
	if (m_Socket != INVALID_SOCKET) {
		CLog::WriteLog(L"Initialize UDP : UDP Socket is Already Initialized!");
		return false;
	}

	if ((m_Socket = CSocketUtil::CreateUDPSocket()) == INVALID_SOCKET) {
		CLog::WriteLog(L"Initialize UDP : Create TCP Socket is Failure! - %d", WSAGetLastError());
		return false;
	}

	CSocketAddress BindAddress(Port);
	if (bind(m_Socket, &BindAddress.m_Address, BindAddress.GetSize()) == SOCKET_ERROR) {
		CLog::WriteLog(L"Bind UDP : Bind UDP Socket is Failure! - %d", WSAGetLastError());
		return false;
	}
	return true;
}

bool CUDPIPSocket::Destroy() {
	if (m_Socket) {
		shutdown(m_Socket, SD_BOTH);
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}
	return false;
}

bool CUDPIPSocket::InitializeReceiveFromForIOCP(CHAR * RecvData, OVERLAPPED_EX & RecvOverlapped) {
	if (!RecvData) {
		CLog::WriteLog(L"RecvFrom For IOCP : Recv Data Buffer is nullptr!");
		return false;
	}

	DWORD RecvBytes = 0, Flags = 0;
	INT AddressSize = m_RemoteAddress.GetSize();
	WSABUF RecvBuffer;
	RecvBuffer.buf = RecvData;
	RecvBuffer.len = MAX_RECEIVE_BUFFER_LENGTH;
	if (WSARecvFrom(m_Socket, &RecvBuffer, 1, &RecvBytes, &Flags, &m_RemoteAddress.m_Address, &AddressSize, &RecvOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"RecvFrom For IOCP : WSARecv is Failure! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

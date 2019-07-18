#include "UDPIPSocket.h"

CUDPIPSocket::CUDPIPSocket() {
}

CUDPIPSocket::~CUDPIPSocket() {
}

bool CUDPIPSocket::BindUDP(const CSocketAddress& BindAddress) {
	if (m_Socket != INVALID_SOCKET) {
		return false;
	}
	m_Socket = CSocketUtil::CreateUDPSocket();
	if (m_Socket == INVALID_SOCKET) {
		return false;
	}

	if (bind(m_Socket, &BindAddress.m_Address, BindAddress.GetSize()) == SOCKET_ERROR) {
		Shutdown();
		return false;
	}
	return true;
}

bool CUDPIPSocket::SendTo(struct OVERLAPPED_EX& SendOverlapped) {
	DWORD SendBytes = 0;

	WSABUF SendToBuffer;
	SendToBuffer.buf;
	SendToBuffer.len;

//	if (WSASendTo(m_Socket, &SendToBuffer, 1, &SendBytes, 0, , , &SendOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			return true;
		}
//	}
	return false;
}

bool CUDPIPSocket::RecvFrom(OVERLAPPED_EX & ReciveOverlapped) {
	DWORD RecvBytes = 0, Flag = 0;

	WSABUF RecvFromBuffer;
	RecvFromBuffer.buf = m_ReciveBuffer;
	RecvFromBuffer.len = MAX_RECIVE_BUFFER_LENGTH;

	INT AddressPtr = CSocketAddress::GetSize();

	if (WSARecvFrom(m_Socket, &RecvFromBuffer, 1, &RecvBytes, &Flag, &m_RecivedAddress.m_Address, &AddressPtr, &ReciveOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			return true;
		}
	}
	return false;
}

void CUDPIPSocket::Shutdown() {
	if (m_Socket != INVALID_SOCKET) {
		shutdown(m_Socket, SD_BOTH);
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}
}
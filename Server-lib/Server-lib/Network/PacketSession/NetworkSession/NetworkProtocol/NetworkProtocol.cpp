#include "NetworkProtocol.h"
#include "../../PacketSession.h"
#include <MSWSock.h>
#pragma comment(lib, "mswsock.lib")

using namespace PROTOCOL;

CProtocol::CProtocol() : m_Socket(INVALID_SOCKET), m_ProtocolType(EPROTOCOLTYPE::EPT_NONE) {
	ZeroMemory(m_ReceiveBuffer, MAX_RECEIVE_BUFFER_LENGTH);
}

bool CProtocol::CopyReceiveBufferForIOCP(char* InBuffer, const uint16_t& Length) {
	if (!InBuffer || Length <= 0) {
		CLog::WriteLog(L"Copy Receive Buffer For IOCP : InBuffer is nullptr or Buffer Length is less than zero!");
		return false;
	}

	CopyMemory(InBuffer, m_ReceiveBuffer, Length);
	return true;
}

TCPIP::CTCPIPSocket::CTCPIPSocket() {
	SetProtocolType(EPROTOCOLTYPE::EPT_TCP);
}

bool TCPIP::CTCPIPSocket::Initialize() {
	SOCKET Socket = INVALID_SOCKET;
	if (Socket != INVALID_SOCKET) {
		CLog::WriteLog(L"Initialize TCP Socket : TCP Socket Already Initialized!");
		return false;
	}

	if ((Socket = CSocketUtil::CreateSocket(true)) == INVALID_SOCKET) {
		CLog::WriteLog(L"Initialize TCP Socket : Failed To Create TCP Socket!");
		return false;
	}
	SetSocket(Socket);

	return true;
}

bool TCPIP::CTCPIPSocket::Listen(const CSocketAddress& BindAddress, const uint16_t& BackLogCount) {
	SOCKET Socket = GetSocket();
	if (Socket == INVALID_SOCKET) {
		CLog::WriteLog(L"Listen : Socket is Invalid!");
		return false;
	}

	if (bind(Socket, reinterpret_cast<const sockaddr*>(&BindAddress), BindAddress.GetSize()) == SOCKET_ERROR) {
		CLog::WriteLog(L"Bind : Failed To Bind Socket!");
		return false;
	}

	if (listen(Socket, BackLogCount) == SOCKET_ERROR) {
		CLog::WriteLog(L"Listen : Failed To Listen Socket!");
		return false;
	}
	return true;
}

bool TCPIP::CTCPIPSocket::Connect(const CSocketAddress& ConnectionAddress) {
	SOCKET Socket = GetSocket();
	if (Socket == INVALID_SOCKET) {
		CLog::WriteLog(L"Connect : Socket is Invalid!");
		return false;
	}

	if (WSAConnect(Socket, reinterpret_cast<const sockaddr*>(&ConnectionAddress), ConnectionAddress.GetSize(), nullptr, nullptr, nullptr, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"Connect : Failed To Connect To Server! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

bool TCPIP::CTCPIPSocket::Accept(const SOCKET& ListenSocket, OVERLAPPED_EX& AcceptOverlapped) {
	if (ListenSocket == INVALID_SOCKET) {
		CLog::WriteLog(L"Accept : Listen Socket is Invalid!");
		return false;
	}

	SOCKET Socket = GetSocket();
	if (Socket != INVALID_SOCKET) {
		int Flag = 0;
		socklen_t Size = sizeof(int);
		if (getsockopt(Socket, SOL_SOCKET, SO_ACCEPTCONN, reinterpret_cast<char*>(&Flag), &Size) == SOCKET_ERROR) {
			CLog::WriteLog(L"Accept : Failed To Get Socket Option!");
			return false;
		}

		if (Flag == 1) {
			CLog::WriteLog(L"Accept : Already Initialized Socket!");
			return false;
		}
	}
	else if ((Socket = CSocketUtil::CreateSocket(true)) == INVALID_SOCKET) {
		CLog::WriteLog(L"Accept : Failed To Create TCP Socket");
		return false;
	}
	SetSocket(Socket);

	if (!AcceptEx(ListenSocket, Socket, GetReceiveBuffer(), 0, CSocketAddress::GetSize() + 16, CSocketAddress::GetSize() + 16, nullptr, &AcceptOverlapped.m_Overlapped)) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"Accept : Failed To Accept - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

bool TCPIP::CTCPIPSocket::Destroy() {
	SOCKET Socket = GetSocket();
	if (Socket != INVALID_SOCKET) {
		closesocket(Socket);
		SetSocket(INVALID_SOCKET);
	}
	// 소켓재활ㅇ요용
	return true;
}

bool TCPIP::CTCPIPSocket::ReceiveForEventSelect(char* InBuffer, uint16_t& BufferLength, OVERLAPPED_EX& ReceiveOverlapped) {
	DWORD RecvBytes = 0, Flags = 0;
	WSABUF ReceiveBuffer;
	ReceiveBuffer.buf = InBuffer;
	ReceiveBuffer.len = MAX_RECEIVE_BUFFER_LENGTH;

	if (WSARecv(GetSocket(), &ReceiveBuffer, 1, &RecvBytes, &Flags, &ReceiveOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"WSA Recv : Failed To Receive! - %d", WSAGetLastError());
			return false;
		}
	}

	BufferLength = RecvBytes;
	return true;
}

bool TCPIP::CTCPIPSocket::InitializeReceiveForIOCP(OVERLAPPED_EX& ReceiveOverlapped) {
	DWORD RecvBytes = 0, Flags = 0;
	WSABUF ReceiveBuffer;
	ReceiveBuffer.buf = GetReceiveBuffer();
	ReceiveBuffer.len = MAX_RECEIVE_BUFFER_LENGTH;

	if (WSARecv(GetSocket(), &ReceiveBuffer, 1, &RecvBytes, &Flags, &ReceiveOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"WSA Recv : Failed To Receive! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

bool TCPIP::CTCPIPSocket::Write(const PACKET::PACKET_INFORMATION& PacketInfo, const char* OutBuffer, const uint16_t& DataLength, OVERLAPPED_EX& SendOverlapped) {
	DWORD SendBytes = 0;

	WSABUF InfoBuffer;
	InfoBuffer.buf = const_cast<char*>(reinterpret_cast<const char*>(&PacketInfo));
	InfoBuffer.len = PACKET::PACKET_INFORMATION::GetSize();

	WSABUF SendBuffer;
	SendBuffer.buf = const_cast<char*>(OutBuffer);
	SendBuffer.len = DataLength;
	
	
	if (WSASend(GetSocket(), &InfoBuffer, 1, &SendBytes, 0, nullptr, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"WSA Send : Failed To Send! - %d", WSAGetLastError());
			return false;
		}
	}

	if (WSASend(GetSocket(), &SendBuffer, 1, &SendBytes, 0, &SendOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"WSA Send : Failed To Send! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

UDPIP::CUDPIPSocket::CUDPIPSocket() {
	SetProtocolType(EPROTOCOLTYPE::EPT_UDP);
}

bool UDPIP::CUDPIPSocket::Initialize() {
	SOCKET Socket = INVALID_SOCKET;
	if (Socket != INVALID_SOCKET) {
		CLog::WriteLog(L"");
		return false;
	}

	if ((Socket = CSocketUtil::CreateSocket(false)) == INVALID_SOCKET) {
		CLog::WriteLog(L"");
		return false;
	}
	SetSocket(Socket);

	return true;
}

bool UDPIP::CUDPIPSocket::Destroy() {
	m_ReliableThread.joinable();

	return true;
}

bool UDPIP::CUDPIPSocket::InitializeReceiveFromForIOCP(CSocketAddress& ReceiveAddress, OVERLAPPED_EX& ReceiveOverlapped) {
	DWORD RecvBytes = 0, Flags = 0;
	WSABUF ReceiveBuffer;
	ReceiveBuffer.buf = GetReceiveBuffer();
	ReceiveBuffer.len = MAX_RECEIVE_BUFFER_LENGTH;

	int AddressLen = CSocketAddress::GetSize();
	if (WSARecvFrom(GetSocket(), &ReceiveBuffer, 1, &RecvBytes, &Flags, reinterpret_cast<sockaddr*>(&ReceiveAddress), &AddressLen, &ReceiveOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			return false;
		}
	}
	
	// Reliable
	return true;
}

bool UDPIP::CUDPIPSocket::ReceiveFromForEventSelect(char* InBuffer, CSocketAddress& ReceiveAddress, uint16_t& DataLength, OVERLAPPED_EX& ReceiveOverlapped) {
	DWORD RecvBytes = 0, Flags = 0;
	WSABUF ReceiveBuffer;
	ReceiveBuffer.buf = InBuffer;
	ReceiveBuffer.len = MAX_RECEIVE_BUFFER_LENGTH;

	int AddressLen = CSocketAddress::GetSize();
	if (WSARecvFrom(GetSocket(), &ReceiveBuffer, 1, &RecvBytes, &Flags, reinterpret_cast<sockaddr*>(&ReceiveAddress), &AddressLen, &ReceiveOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			return false;
		}
	}
	DataLength = RecvBytes;

	// Reliable
	return true;
}

bool UDPIP::CUDPIPSocket::WriteTo(const CSocketAddress& SendAddress, const char* OutBuffer, const uint16_t& DataLength, OVERLAPPED_EX& SendOverlapped) {
	DWORD SendBytes = 0;
	WSABUF SendBuffer;
	SendBuffer.buf = const_cast<char*>(OutBuffer);
	SendBuffer.len = DataLength;

	if (WSASendTo(GetSocket(), &SendBuffer, 1, &SendBytes, 0, reinterpret_cast<const sockaddr*>(&SendAddress), CSocketAddress::GetSize(), &SendOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			return false;
		}
	}
	return true;
}
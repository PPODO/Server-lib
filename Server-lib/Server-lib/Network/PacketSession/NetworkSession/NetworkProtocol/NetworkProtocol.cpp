#include "NetworkProtocol.h"
#include "../../NetworkSession/NetworkSession.h"
#include <MSWSock.h>
#include <utility>
#pragma comment(lib, "mswsock.lib")

using namespace PROTOCOL;

CProtocol::CProtocol() : m_Socket(INVALID_SOCKET), m_ProtocolType(EPROTOCOLTYPE::EPT_NONE) {
	ZeroMemory(m_ReceiveBuffer, MAX_RECEIVE_BUFFER_LENGTH);
}

bool CProtocol::CopyReceiveBufferForIOCP(char* InBuffer, uint16_t& Length) {
	if (!InBuffer || Length <= 0) {
		CLog::WriteLog(L"Copy Receive Buffer For IOCP : InBuffer is nullptr or Buffer Length is less than zero!");
		return false;
	}

	CopyMemory(InBuffer, m_ReceiveBuffer, Length);

	CheckAck(Length);
	return true;
}

bool CProtocol::CheckAck(uint16_t& Length) {
	if (UDPIP::CUDPIPSocket * UDPSocket = reinterpret_cast<UDPIP::CUDPIPSocket*>(this)) {
		int Ack = *reinterpret_cast<int*>(m_ReceiveBuffer);

		if (Ack == 0) {
			MoveMemory(m_ReceiveBuffer, m_ReceiveBuffer + sizeof(int), Length);
			Length -= sizeof(int);

			CSocketSystem::WriteTo(UDPSocket, true, UDPSocket->GetLastRemoteAddress(), PACKET::PACKET_INFORMATION(), nullptr, 0, nullptr);
		}
		else if (Ack == 9999) {
			CLog::WriteLog(L"Succeed!");
			ZeroMemory(m_ReceiveBuffer, Length);
			return false;
		}
	}
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

bool TCPIP::CTCPIPSocket::Listen(const CSocketAddress& BindAddress, const uint32_t& BackLogCount) {
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

bool TCPIP::CTCPIPSocket::ReceiveForEventSelect(char* InBuffer, uint16_t& BufferLength) {
	DWORD RecvBytes = 0, Flags = 0;
	WSABUF ReceiveBuffer;
	ReceiveBuffer.buf = InBuffer;
	ReceiveBuffer.len = MAX_RECEIVE_BUFFER_LENGTH;

	if (WSARecv(GetSocket(), &ReceiveBuffer, 1, &RecvBytes, &Flags, nullptr, nullptr) == SOCKET_ERROR) {
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



UDPIP::CUDPIPSocket::CUDPIPSocket() : m_hWakeupThreadEvent(INVALID_HANDLE_VALUE), m_hSendCompleteEvent(INVALID_HANDLE_VALUE), m_hWaitForInitialize(INVALID_HANDLE_VALUE), m_hStop(INVALID_HANDLE_VALUE), m_bIsReliableSending(false) {
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

	if (!(m_hWakeupThreadEvent = CreateEvent(nullptr, false, false, nullptr))) {
		CLog::WriteLog(L"");
		return false;
	}

	if (!(m_hSendCompleteEvent = CreateEvent(nullptr, false, false, nullptr))) {
		CLog::WriteLog(L"");
		return false;
	}

	if (!(m_hWaitForInitialize = CreateEvent(nullptr, false, false, nullptr))) {
		CLog::WriteLog(L"");
		return false;
	}

	if (!(m_hStop = CreateEvent(nullptr, false, false, nullptr))) {
		CLog::WriteLog(L"");
		return false;
	}

	m_ReliableThread = std::thread(&UDPIP::CUDPIPSocket::ProcessReliable, this);

	WaitForSingleObject(m_hWaitForInitialize, INFINITE);
	return true;
}

bool PROTOCOL::UDPIP::CUDPIPSocket::Bind(const CSocketAddress& Address) {
	if (GetSocket() == INVALID_SOCKET) {
		CLog::WriteLog(L"Bind : Socket is Invalid!");
		return false;
	}

	if (bind(GetSocket(), reinterpret_cast<const sockaddr*>(&Address), Address.GetSize()) == SOCKET_ERROR) {
		CLog::WriteLog(L"Bind : Failed To Bind Socket!");
		return false;
	}
	return true;
}

bool UDPIP::CUDPIPSocket::Destroy() {
	SetEvent(m_hStop);
	
	if (m_ReliableThread.joinable()) {
		m_ReliableThread.join();
	}

	if (m_hStop) {
		CloseHandle(m_hStop);
		m_hStop = INVALID_HANDLE_VALUE;
	}

	if (m_hWaitForInitialize) {
		CloseHandle(m_hWaitForInitialize);
		m_hWaitForInitialize = INVALID_HANDLE_VALUE;
	}

	if (m_hSendCompleteEvent) {
		CloseHandle(m_hSendCompleteEvent);
		m_hSendCompleteEvent = INVALID_HANDLE_VALUE;
	}

	if (m_hWakeupThreadEvent) {
		CloseHandle(m_hWakeupThreadEvent);
		m_hWakeupThreadEvent = INVALID_HANDLE_VALUE;
	}
	return true;
}

bool UDPIP::CUDPIPSocket::InitializeReceiveFromForIOCP(OVERLAPPED_EX& ReceiveOverlapped) {
	DWORD RecvBytes = 0, Flags = 0;
	WSABUF ReceiveBuffer;
	ReceiveBuffer.buf = GetReceiveBuffer();
	ReceiveBuffer.len = MAX_RECEIVE_BUFFER_LENGTH;

	int AddressLen = CSocketAddress::GetSize();
	if (WSARecvFrom(GetSocket(), &ReceiveBuffer, 1, &RecvBytes, &Flags, reinterpret_cast<sockaddr*>(&m_LastRemoteAddress), &AddressLen, &ReceiveOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"WSA Recv From : Failed To Recv! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

bool UDPIP::CUDPIPSocket::ReceiveFromForEventSelect(char* InBuffer, uint16_t& DataLength) {
	DWORD RecvBytes = 0, Flags = 0;
	WSABUF ReceiveBuffer;
	ReceiveBuffer.buf = InBuffer;
	ReceiveBuffer.len = MAX_RECEIVE_BUFFER_LENGTH;

	CSocketAddress ReceiveAddress;
	int AddressLen = CSocketAddress::GetSize();
	if (WSARecvFrom(GetSocket(), &ReceiveBuffer, 1, &RecvBytes, &Flags, reinterpret_cast<sockaddr*>(&m_LastRemoteAddress), &AddressLen, nullptr, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			return false;
		}
	}
	DataLength = RecvBytes;

	CheckAck(DataLength);
	return true;
}

bool UDPIP::CUDPIPSocket::WriteTo(bool IsReliable, const CSocketAddress& SendAddress, const PACKET::PACKET_INFORMATION& PacketInfo, const char* OutBuffer, const uint16_t& DataLength, OVERLAPPED_EX& SendOverlapped) {
	char TempChar[MAX_RECEIVE_BUFFER_LENGTH] = { "\0" };
	int Ack = 0;
	DWORD SendBytes = 0;
	WSABUF SendBuffer;

	if (IsReliable) {
		Ack = 9999;
		SendBuffer.buf = reinterpret_cast<char*>(&Ack);
		SendBuffer.len = sizeof(int);
	}
	else {
		Ack = 0;

		CopyMemory(TempChar, reinterpret_cast<const char*>(&Ack), sizeof(int));
		CopyMemory(TempChar + sizeof(int), reinterpret_cast<const char*>(&PacketInfo), PACKET::PACKET_INFORMATION::GetSize());
		CopyMemory(TempChar + PACKET::PACKET_INFORMATION::GetSize() + sizeof(int), OutBuffer, DataLength);

		SendBuffer.buf = TempChar;
		SendBuffer.len = sizeof(int) + PACKET::PACKET_INFORMATION::GetSize() + DataLength;
	}
	
	if (WSASendTo(GetSocket(), &SendBuffer, 1, &SendBytes, 0, reinterpret_cast<const sockaddr*>(&SendAddress), CSocketAddress::GetSize(), &SendOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"WSA Send To : Failed To Send! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

bool UDPIP::CUDPIPSocket::WriteToQueue(const RELIABLE_DATA* const Data) {
	if (Data && m_ReliableQueue.Push(const_cast<RELIABLE_DATA * const&>(Data))) {
		if (!m_bIsReliableSending) {
			return SetEvent(m_hWakeupThreadEvent);
		}
		return true;
	}
	return false;
}

bool UDPIP::CUDPIPSocket::ProcessReliable() {
	HANDLE Events[2] = { m_hStop, m_hWakeupThreadEvent };
	DWORD EventType = 0;

	SetEvent(m_hWaitForInitialize);
	while (true) {
		EventType = WaitForMultipleObjects(2, Events, false, INFINITE);

		switch (EventType) {
		case WAIT_OBJECT_0:
			return false;
		case WAIT_OBJECT_0 + 1:
			RELIABLE_DATA * ReliableData = nullptr;
			while (!m_ReliableQueue.IsEmpty()) {
				if (m_ReliableQueue.Pop(ReliableData) && ReliableData) {
					CNetworkSession* Owner = reinterpret_cast<CNetworkSession*>(ReliableData->m_Owner);
					// for(){}
					if (!Owner || !WriteTo(false, ReliableData->m_RemoteAddress, ReliableData->m_PacketInformation, ReliableData->m_DataBuffer, ReliableData->m_DataSize, *Owner->GetOverlappedByIOType(EIOTYPE::EIOTYPE_WRITE))) {
						break;
					}

					DWORD Result = WaitForSingleObject(m_hSendCompleteEvent, 10);

					if (Result == WAIT_OBJECT_0) {
						break;
					}
					else {
						continue;
					}
				}
				else {
					break;
				}
			}
			m_bIsReliableSending = false;
			break;
		}
	}
	return true;
}
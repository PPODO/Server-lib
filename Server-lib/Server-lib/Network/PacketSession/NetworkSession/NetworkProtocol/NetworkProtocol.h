#pragma once
#include <thread>
#include "../../../../Functions/SocketUtil/SocketUtil.h"
#include "../../../../Functions/SocketAddress/SocketAddress.h"

struct OVERLAPPED_EX;

namespace PROTOCOL {
	enum class EPROTOCOLTYPE {
		EPT_NONE,
		EPT_TCP,
		EPT_UDP,
	};

	class CProtocol {
	private:
		char m_ReceiveBuffer[MAX_RECEIVE_BUFFER_LENGTH];

	private:
		SOCKET m_Socket;
		EPROTOCOLTYPE m_ProtocolType;

	protected:
		inline void SetSocket(const SOCKET& NewSocket) { m_Socket = NewSocket; }

	protected:
		inline char* GetReceiveBuffer() { return m_ReceiveBuffer; }

	public:
		CProtocol();

	public:
		virtual bool Initialize() = 0;
		virtual bool Destroy() = 0;

	public:
		bool CopyReceiveBufferForIOCP(char* InBuffer, const uint16_t& Length);

	public:
		inline void SetProtocolType(const EPROTOCOLTYPE& NewType) { m_ProtocolType = NewType; }

	public:
		inline SOCKET GetSocket() const { return m_Socket; }
		inline EPROTOCOLTYPE GetProtocolType() const { return m_ProtocolType; }

	};

	namespace TCPIP {
		class CTCPIPSocket : public CProtocol {
		public:
			CTCPIPSocket();

		public:
			virtual bool Initialize() override;
			bool Listen(const CSocketAddress& BindAddress, const uint16_t& BackLogCount);
			bool Connect(const CSocketAddress& ConnectionAddress);
			bool Accept(const SOCKET& ListenSocket, OVERLAPPED_EX& AcceptOverlapped);
			virtual bool Destroy() override;

		public:
			bool InitializeReceiveForIOCP(OVERLAPPED_EX& ReceiveOverlapped);

		public:
			bool ReceiveForEventSelect(char* InBuffer, uint16_t& DataLength, OVERLAPPED_EX& ReceiveOverlapped);

		public:
			bool Write(const char* OutBuffer, const uint16_t& DataLength, OVERLAPPED_EX& SendOverlapped);

		};
	}

	namespace UDPIP {
		class CUDPIPSocket : public CProtocol {
		private:
			std::thread m_ReliableThread;

		public:
			CUDPIPSocket();

		public:
			bool Initialize();
			bool Destroy();

		public:
			bool InitializeReceiveFromForIOCP(CSocketAddress& ReceiveAddress, OVERLAPPED_EX& ReceiveOverlapped);

		public:
			bool ReceiveFromForEventSelect(char* InBuffer, CSocketAddress& ReceiveAddress, uint16_t& DataLength, OVERLAPPED_EX& ReceiveOverlapped);

		public:
			bool WriteTo(const CSocketAddress& SendAddress, const char* OutBuffer, const uint16_t& DataLength, OVERLAPPED_EX& SendOverlapped);

		};
	}

	class CSocketSystem {
	public:
		// BOTH
		static bool InitializeSocket(PROTOCOL::CProtocol* const Socket) {
			if (Socket) {
				return Socket->Initialize();
			}
			return false;
		}

		static bool DestroySocket(PROTOCOL::CProtocol* const Socket) {
			if (Socket) {
				return Socket->Destroy();
			}
			return false;
		}

		static bool CopyReceiveBuffer(PROTOCOL::CProtocol* const Socket, char* const DataBuffer, const uint16_t& CopyLength) {
			if (Socket) {
				return Socket->CopyReceiveBufferForIOCP(DataBuffer, CopyLength);
			}
			return false;
		}

		static SOCKET GetSocketByClass(PROTOCOL::CProtocol* const Socket) {
			if (Socket) {
				return Socket->GetSocket();
			}
			return INVALID_SOCKET;
		}

		static EPROTOCOLTYPE GetProtocolTypeByClass(PROTOCOL::CProtocol* const Socket) {
			if (Socket) {
				return Socket->GetProtocolType();
			}
			return EPROTOCOLTYPE::EPT_NONE;
		}

		static void SetProtocolTypeByClass(PROTOCOL::CProtocol* const Socket, const EPROTOCOLTYPE& NewType) {
			if (Socket) {
				Socket->SetProtocolType(NewType);
			}
		}

		static void SetProtocolTypeByClass(PROTOCOL::CProtocol* const Socket, const PROTOCOL::CProtocol* const ProtocolSocket) {
			if (Socket) {
				Socket->SetProtocolType(ProtocolSocket->GetProtocolType());
			}
		}

	public:
		// TCP
		static bool Listen(PROTOCOL::TCPIP::CTCPIPSocket* const Socket, const CSocketAddress& BindAddress, const uint16_t& BackLogCount = SOMAXCONN) {
			if (Socket) {
				return Socket->Listen(BindAddress, BackLogCount);
			}
			return false;
		}

		static bool Connect(PROTOCOL::TCPIP::CTCPIPSocket* const Socket, const CSocketAddress& ConnectionAddress) {
			if (Socket) {
				return Socket->Connect(ConnectionAddress);
			}
			return false;
		}

		static bool Accept(PROTOCOL::TCPIP::CTCPIPSocket* const Socket, const SOCKET& ListenSocket, OVERLAPPED_EX* const Overlapped) {
			if (Socket && Overlapped) {
				return Socket->Accept(ListenSocket, *Overlapped);
			}
			return false;
		}

		static bool Receive(PROTOCOL::TCPIP::CTCPIPSocket* const Socket, OVERLAPPED_EX* const Overlapped) {
			if (Socket && Overlapped) {
				return Socket->InitializeReceiveForIOCP(*Overlapped);
			}
			return false;
		}

		static bool Write(PROTOCOL::TCPIP::CTCPIPSocket* const Socket, const char* const DataBuffer, const uint16_t& DataLength, OVERLAPPED_EX* const Overlapped) {
			if (Socket && DataBuffer && Overlapped) {
				return Socket->Write(DataBuffer, DataLength, *Overlapped);
			}
			return false;
		}

	public:
		// UDP

		static bool ReceiveFrom(PROTOCOL::UDPIP::CUDPIPSocket* const Socket, CSocketAddress& ReceiveAddress, OVERLAPPED_EX* const Overlapped) {
			if (Socket && Overlapped) {
				return Socket->InitializeReceiveFromForIOCP(ReceiveAddress, *Overlapped);
			}
			return false;
		}

		static bool WriteTo(PROTOCOL::UDPIP::CUDPIPSocket* const Socket, const CSocketAddress& SendAddress, const char* const DataBuffer, const uint16_t& DataLength, OVERLAPPED_EX* const Overlapped) {
			if (Socket && Overlapped) {
				return Socket->WriteTo(SendAddress, DataBuffer, DataLength, *Overlapped);
			}
			return false;
		}
	};
}
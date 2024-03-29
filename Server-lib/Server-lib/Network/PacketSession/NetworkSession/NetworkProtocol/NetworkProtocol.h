#pragma once
#include <thread>
#include <map>
#include "../../../../Functions/SocketUtil/SocketUtil.h"
#include "../../../../Functions/SocketAddress/SocketAddress.h"
#include "../../../../Functions/CircularQueue/CircularQueue.h"

struct OVERLAPPED_EX;
struct RELIABLE_DATA;
struct BUFFER_DATA;
namespace PACKET {
	struct PACKET_INFORMATION;
}

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
		bool CopyReceiveBuffer(char* InBuffer, uint16_t& Length);

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
			bool Listen(const CSocketAddress& BindAddress, const uint32_t& BackLogCount);
			bool Connect(const CSocketAddress& ConnectionAddress);
			bool Accept(const SOCKET& ListenSocket, OVERLAPPED_EX& AcceptOverlapped);
			virtual bool Destroy() override;

		public:
			bool InitializeReceiveForIOCP(OVERLAPPED_EX& ReceiveOverlapped);

		public:
			bool ReceiveForEventSelect(char* InBuffer, uint16_t& DataLength);

		public:
			bool Write(const PACKET::PACKET_INFORMATION& PacketInfo, const char* OutBuffer, const uint16_t& DataLength, OVERLAPPED_EX& SendOverlapped);

		};
	}

	namespace UDPIP {
		class CUDPIPSocket : public CProtocol {
			const size_t PACKET_DROP_MAX_COUNT = 50;
		private:
			std::thread m_ReliableThread;

		private:
			CSocketAddress m_LastRemoteAddress;

		private:
			HANDLE m_hWaitForInitialize;
			HANDLE m_hWakeupThreadEvent;
			HANDLE m_hSendCompleteEvent;
			HANDLE m_hStop;

		private:
			bool m_bIsReliableSending;

		private:
			CCircularQueue<RELIABLE_DATA*> m_ReliableQueue;

		private:
			CCriticalSection m_AckProcessLocking;

		private:
			bool ProcessReliable();
			
		private:
			inline bool ReliableSend(const RELIABLE_DATA* const ReliableData, OVERLAPPED_EX& SendOverlapped) {
				for (size_t i = 0; i < PACKET_DROP_MAX_COUNT; i++) {
					std::cout << "Try Send!\n";
					if (!WriteTo(false, ReliableData->m_RemoteAddress, ReliableData->m_PacketInformation, ReliableData->m_DataBuffer, ReliableData->m_DataSize, SendOverlapped)) {
						return false;
					}

					DWORD Result = WaitForSingleObject(m_hSendCompleteEvent, 10);

					if (Result == WAIT_OBJECT_0) {
						return true;
					}
					else {
						continue;
					}
				}
				return false;
			}

		public:
			CUDPIPSocket();

		public:
			bool Initialize();
			bool Bind(const CSocketAddress& Address);
			bool Destroy();
			bool CheckAck(uint16_t& Length);

		public:
			bool InitializeReceiveFromForIOCP(OVERLAPPED_EX& ReceiveOverlapped);

		public:
			bool ReceiveFromForEventSelect(char* InBuffer, uint16_t& DataLength);

		public:
			bool WriteToQueue(const RELIABLE_DATA* const Data);
			bool WriteTo(bool IsReliable, const CSocketAddress& SendAddress, const PACKET::PACKET_INFORMATION& PacketInfo, const char* OutBuffer, const uint16_t& DataLength, OVERLAPPED_EX& SendOverlapped);

		public:
			CSocketAddress GetLastRemoteAddress() const { return m_LastRemoteAddress; }

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

		static bool Bind(PROTOCOL::CProtocol* const Socket, const CSocketAddress& BindAddress, const uint32_t& BackLogCount = SOMAXCONN) {
			if (Socket) {
				switch (Socket->GetProtocolType()) {
				case EPROTOCOLTYPE::EPT_TCP:
					return reinterpret_cast<PROTOCOL::TCPIP::CTCPIPSocket*>(Socket)->Listen(BindAddress, BackLogCount);
				case EPROTOCOLTYPE::EPT_UDP:
					return reinterpret_cast<PROTOCOL::UDPIP::CUDPIPSocket*>(Socket)->Bind(BindAddress);
				}
			}
			return false;
		}

		static bool DestroySocket(PROTOCOL::CProtocol* const Socket) {
			if (Socket) {
				return Socket->Destroy();
			}
			return false;
		}

		static bool CopyReceiveBuffer(PROTOCOL::CProtocol* const Socket, char* const DataBuffer, uint16_t& CopyLength) {
			if (Socket) {
				return Socket->CopyReceiveBuffer(DataBuffer, CopyLength);
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
			if (Socket) {
				return Socket->InitializeReceiveForIOCP(*Overlapped);
			}
			return false;
		}

		static bool ReceiveEventSelect(PROTOCOL::TCPIP::CTCPIPSocket* const Socket, char* const Buffer, uint16_t& RecvBytes) {
			if (Socket) {
				return Socket->ReceiveForEventSelect(Buffer, RecvBytes);
			}
			return false;
		}

		static bool Write(PROTOCOL::TCPIP::CTCPIPSocket* const Socket, const PACKET::PACKET_INFORMATION& PacketInfo, const char* const DataBuffer, const uint16_t& DataLength, OVERLAPPED_EX* const Overlapped) {
			if (Socket && DataBuffer) {
				return Socket->Write(PacketInfo, DataBuffer, DataLength, *Overlapped);
			}
			return false;
		}

		static bool Write(PROTOCOL::TCPIP::CTCPIPSocket* const Socket, const BUFFER_DATA* const BufferData, OVERLAPPED_EX* const Overlapped) {
			if (Socket && BufferData) {
				return Socket->Write(BufferData->m_PacketInformation, BufferData->m_DataBuffer, BufferData->m_DataSize, *Overlapped);
			}
			return false;
		}

	public:
		// UDP
		static bool ReceiveFrom(PROTOCOL::UDPIP::CUDPIPSocket* const Socket, OVERLAPPED_EX* const Overlapped) {
			if (Socket) {
				return Socket->InitializeReceiveFromForIOCP(*Overlapped);
			}
			return false;
		}

		static bool ReceiveFromEventSelect(PROTOCOL::UDPIP::CUDPIPSocket* const Socket, char* const Buffer, uint16_t& RecvBytes) {
			if (Socket) {
				return Socket->ReceiveFromForEventSelect(Buffer, RecvBytes);
			}
			return false;
		}

		static bool WriteTo(PROTOCOL::UDPIP::CUDPIPSocket* const Socket, bool bUseAck, const CSocketAddress& SendAddress, const PACKET::PACKET_INFORMATION& PacketInfo, const char* const DataBuffer, const uint16_t& DataLength, OVERLAPPED_EX* const Overlapped) {
			if (Socket) {
				return Socket->WriteTo(bUseAck, SendAddress, PacketInfo, DataBuffer, DataLength, *Overlapped);
			}
			return false;
		}

		static bool WriteToQueue(PROTOCOL::UDPIP::CUDPIPSocket* const Socket, const RELIABLE_DATA* const Data) {
			if (Socket) {
				return Socket->WriteToQueue(Data);
			}
			return false;
		}
	};
}
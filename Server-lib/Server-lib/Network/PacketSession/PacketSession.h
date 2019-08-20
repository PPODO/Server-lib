#pragma once
#include <string>
#include "NetworkSession/NetworkSession.h"
#include "../../Functions/BasePacket/BasePacket.h"
#include "../../Functions/CircularQueue/CircularQueue.h"

class CPacketSession : public CNetworkSession {
private:
	CHAR m_PacketBuffer[MAX_RECEIVE_BUFFER_LENGTH];
	size_t m_CurrentReadBytes;

private:
	PACKET_INFORMATION m_PacketInformation;

private:
	CCircularQueue m_WriteQueue;

private:
	inline void MoveBufferMemory(const uint16_t& MoveLength) {
		MoveMemory(m_PacketBuffer, m_PacketBuffer + MoveLength, m_CurrentReadBytes);
	}

protected:
	virtual CBasePacket* GetPacket(const uint8_t& PacketType, const CHAR* Buffer, const USHORT& PacketSize) = 0;

public:
	CPacketSession();

public:
	virtual bool Initialize() override;
	virtual bool Destroy() override;

public:
	template<typename T>
	bool Write(const T& Packet) {
		CThreadSync Sync;

		if (std::is_arithmetic<T>() || std::is_enum<T>()) {
			CLog::WriteLog(L"Write : Only Types Support That Inherit BasePacket!");
			return;
		}

		std::string Buffer;
		Serialize(Packet, Buffer);

		PACKET_INFORMATION PacketInformation(Buffer.length(), Packet.m_PacketType);

		if (const CHAR* const TempBuffer = m_WriteQueue.Push(QUEUE_DATA(this, Buffer.c_str(), Buffer.length()))) {
			return CNetworkSession::Write(PacketInformation, TempBuffer, Buffer.length());
		}
		return false;
	}

public:
	inline bool CopyIOCPBuffer(const USHORT& DataLength) {
		CThreadSync Sync;

		if (!CNetworkSession::CopyIOCPBuffer(m_PacketBuffer + m_CurrentReadBytes, DataLength)) {
			return false;
		}
		m_CurrentReadBytes += DataLength;
		return true;
	}
	inline bool WriteComplete() {
		CThreadSync Sync;

		if (!m_WriteQueue.IsEmpty()) {
			return m_WriteQueue.Pop();
		}
		return false;
	}

public:
	CBasePacket* PacketAnalysis();

};
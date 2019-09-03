#pragma once
#include "../CriticalSection/CriticalSection.h"
#include "../MemoryPool/MemoryPool.h"
#include "../SocketUtil/SocketUtil.h"
#include "../BasePacket/BasePacket.h"

static const size_t MAX_QUEUE_LENGTH = 256;

template<typename T, size_t MAX_POOL_SIZE = 50>
struct BASE_QUEUE_DATA : public CMemoryPool<T, MAX_POOL_SIZE> {
public:
	void* m_Owner;
	void* m_Data;

public:
	BASE_QUEUE_DATA() : m_Owner(nullptr), m_Data(nullptr) {}
	BASE_QUEUE_DATA(const void* Owner, const void* Data) : m_Owner(const_cast<void*>(Owner)), m_Data(const_cast<void*>(Data)) {}

};

struct BUFFER_DATA : public BASE_QUEUE_DATA<BUFFER_DATA> {
public:
	PACKET::PACKET_INFORMATION m_PacketInformation;
	CHAR m_DataBuffer[MAX_RECEIVE_BUFFER_LENGTH];
	USHORT m_DataSize;

public:
	BUFFER_DATA() : m_DataSize(0) { ZeroMemory(&m_DataBuffer, MAX_RECEIVE_BUFFER_LENGTH); }
	BUFFER_DATA(const void* Owner, const PACKET::PACKET_INFORMATION& PacketInfo, const char* const DataBuffer, const USHORT& DataSize) : BASE_QUEUE_DATA(Owner, nullptr), m_PacketInformation(PacketInfo), m_DataSize(DataSize) {
		if (DataSize > 0) {
			ZeroMemory(&m_DataBuffer, MAX_RECEIVE_BUFFER_LENGTH);
			CopyMemory(m_DataBuffer, DataBuffer, DataSize);
		}
	}

};

struct PACKET_DATA : public BASE_QUEUE_DATA<PACKET_DATA> {
public:
	uint8_t m_PacketType;

public:
	PACKET_DATA() : m_PacketType(0) {};
	PACKET_DATA(const void* Owner, const void* Packet, const uint8_t& PacketType) : BASE_QUEUE_DATA(Owner, Packet), m_PacketType(PacketType){}

};

struct RELIABLE_DATA : public BASE_QUEUE_DATA<RELIABLE_DATA> {
public:
	CSocketAddress m_RemoteAddress;
	PACKET::PACKET_INFORMATION m_PacketInformation;
	CHAR m_DataBuffer[MAX_RECEIVE_BUFFER_LENGTH];
	USHORT m_DataSize;

public:
	RELIABLE_DATA() { ZeroMemory(&m_DataBuffer, MAX_RECEIVE_BUFFER_LENGTH); }
	RELIABLE_DATA(const void* Owner, CSocketAddress& RemoteAddress, const PACKET::PACKET_INFORMATION& Info, const char* const DataBuffer, const uint16_t& BufferLength) : BASE_QUEUE_DATA(Owner, nullptr), m_RemoteAddress(RemoteAddress), m_PacketInformation(Info), m_DataSize(BufferLength) {
		CopyMemory(m_DataBuffer, DataBuffer, BufferLength);
	}

};

template<typename QUEUEDATATYPE>
class CCircularQueue {
private:
	CCriticalSection m_ListLock;

private:
	QUEUEDATATYPE m_Queue[MAX_QUEUE_LENGTH];

private:
	size_t m_Head, m_Tail;

public:
	CCircularQueue() : m_Head(0), m_Tail(0) { ZeroMemory(&m_Queue, MAX_QUEUE_LENGTH); }

public:
	const QUEUEDATATYPE& Push(const QUEUEDATATYPE& InData) {
		CCriticalSectionGuard Lock(m_ListLock);

		size_t TempTail = (m_Tail + 1) % MAX_QUEUE_LENGTH;
		if (TempTail == m_Tail) {
			return nullptr;
		}
		m_Queue[TempTail] = InData;
		m_Tail = TempTail;

		return InData;
	}

	bool Pop() {
		CCriticalSectionGuard Lock(m_ListLock);

		size_t TempHead = (m_Head + 1) % MAX_QUEUE_LENGTH;
		if (TempHead == m_Head) {
			return false;
		}
		m_Queue[TempHead] = QUEUEDATATYPE();
		m_Head = TempHead;
		return true;
	}

	bool Pop(QUEUEDATATYPE& InData) {
		CCriticalSectionGuard Lock(m_ListLock);

		size_t TempHead = (m_Head + 1) % MAX_QUEUE_LENGTH;
		if (TempHead == m_Head) {
			return false;
		}
		InData = m_Queue[TempHead];
		m_Head = TempHead;
		return true;
	}

	bool IsEmpty() {
		CCriticalSectionGuard Lock(m_ListLock);

		if (m_Head == m_Tail) {
			return true;
		}
		return false;
	}

};
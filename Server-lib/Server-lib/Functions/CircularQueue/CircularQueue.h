#pragma once
#include "../ThreadSync/ThreadSync.h"
#include "../SocketUtil/SocketUtil.h"
#include "../BasePacket/BasePacket.h"

static const size_t MAX_QUEUE_LENGTH = 256;

struct QUEUE_DATA {
public:
	void* m_Owner;
	CHAR m_DataBuffer[MAX_RECEIVE_BUFFER_LENGTH];
	USHORT m_DataSize;

public:
	QUEUE_DATA() : m_Owner(nullptr), m_DataSize(0) { ZeroMemory(&m_DataBuffer, MAX_RECEIVE_BUFFER_LENGTH); }
	QUEUE_DATA(const void* Owner, const char* DataBuffer, const USHORT& DataSize) : m_Owner(const_cast<void*>(Owner)), m_DataSize(DataSize) {
		if (m_DataBuffer) {
			delete[] m_DataBuffer;
		}

		if (DataSize > 0) {
			CopyMemory(m_DataBuffer, DataBuffer, DataSize);
		}
	}

};


class CCircularQueue : public CMultiThreadSync<CCircularQueue> {
private:
	QUEUE_DATA m_Queue[MAX_QUEUE_LENGTH];

private:
	size_t m_Head, m_Tail;

public:
	CCircularQueue() : m_Head(0), m_Tail(0) {};

public:
	const CHAR* Push(const QUEUE_DATA& InData) {
		CThreadSync Sync;

		size_t TempTail = (m_Tail + 1) % MAX_QUEUE_LENGTH;
		if (TempTail == m_Tail) {
			return nullptr;
		}
		m_Queue[TempTail] = InData;
		m_Tail = TempTail;

		return InData.m_DataBuffer;
	}

	bool Pop() {
		CThreadSync Sync;

		size_t TempHead = (m_Head + 1) % MAX_QUEUE_LENGTH;
		if (TempHead == m_Head) {
			return false;
		}
		m_Head = TempHead;
		return true;
	}

	bool IsEmpty() {
		CThreadSync Sync;

		if (m_Head == m_Tail) {
			return true;
		}
		return false;
	}

};
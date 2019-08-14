#pragma once
#include "../ThreadSync/ThreadSync.h"
#include "../SocketUtil/SocketUtil.h"

struct QUEUE_DATA {
public:
	void* m_Owner;
	CHAR m_DataBuffer[MAX_RECEIVE_BUFFER_LENGTH];
	USHORT m_DataSize;

public:
	QUEUE_DATA() : m_Owner(nullptr), m_DataSize(0) { ZeroMemory(m_DataBuffer, MAX_RECEIVE_BUFFER_LENGTH); };
	QUEUE_DATA(const void* Owner, const char* DataBuffer, const USHORT& DataSize) : m_Owner(const_cast<void*>(Owner)), m_DataSize(DataSize) {
		if (m_DataBuffer) {
			delete[] m_DataBuffer;
		}

		if (DataSize > 0) {
			CopyMemory(m_DataBuffer, DataBuffer, DataSize);
		}
	}

};

static const size_t MAX_QUEUE_LENGTH = 256;

class CCircularQueue : public CMultiThreadSync<CCircularQueue> {
private:
	QUEUE_DATA m_Queue[MAX_QUEUE_LENGTH];

private:
	size_t m_Head, m_Tail;

public:
	CCircularQueue() : m_Head(0), m_Tail(0) {};
	
public:
	const CHAR* Push(const QUEUE_DATA& InData) {
		size_t TempTail = (m_Tail + 1) % MAX_QUEUE_LENGTH;
		if (TempTail == m_Tail) {
			return nullptr;
		}
		m_Queue[TempTail] = InData;
		m_Tail = TempTail;

		return InData.m_DataBuffer;
	}
	
	const CHAR* Push(const void* Owner, const CHAR* DataBuffer, const USHORT& DataSize) {
		size_t TempTail = (m_Tail + 1) % MAX_QUEUE_LENGTH;
		if (TempTail == m_Head) {
			return nullptr;
		}
		m_Queue[TempTail] = QUEUE_DATA(Owner, DataBuffer, DataSize);
		m_Tail = TempTail;

		return m_Queue[TempTail].m_DataBuffer;
	}

	bool Pop() {
		size_t TempHead = (m_Head + 1) % MAX_QUEUE_LENGTH;
		if (TempHead == m_Head) {
			return false;
		}
		m_Head = TempHead;
		return true;
	}

	bool IsEmpty() {
		if (m_Head == m_Tail) {
			return true;
		}
		return false;
	}

};
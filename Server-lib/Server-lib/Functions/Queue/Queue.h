#pragma once
#include "../ThreadSync/ThreadSync.h"
#include "../BasePacket/BasePacket.h"
#include "../Log/Log.h"

struct PACKET_QUEUE_DATA {
public:
	void* m_Owner;
	CBasePacket* m_Packet;
	
public:
	PACKET_QUEUE_DATA() : m_Owner(nullptr), m_Packet(nullptr) {}
	PACKET_QUEUE_DATA(const void* Owner, const CBasePacket* Packet) : m_Owner(const_cast<void*>(Owner)), m_Packet(const_cast<CBasePacket*>(Packet)) {}

};

const size_t MAX_QUEUE_SIZE = 256;

class CQueue : public CMultiThreadSync<CQueue> {
private:
	PACKET_QUEUE_DATA m_Queue[MAX_QUEUE_SIZE];

private:
	size_t m_CurrentIndex;

public:
	CQueue() : m_CurrentIndex(0) {}

public:
	bool Push(const PACKET_QUEUE_DATA& Data) {
		if (m_CurrentIndex == MAX_QUEUE_SIZE) {
			CLog::WriteLog(L"Queue : Max!");
			return false;
		}
		m_Queue[++m_CurrentIndex] = Data;
		return true;
	}

	bool Pop(PACKET_QUEUE_DATA& Data) {
		if (m_CurrentIndex == 0) {
			return false;
		}
		Data = m_Queue[m_CurrentIndex--];
		return true;
	}

	bool IsEmpty() {
		if (m_CurrentIndex == 0) {
			return true;
		}
		return false;
	}

};
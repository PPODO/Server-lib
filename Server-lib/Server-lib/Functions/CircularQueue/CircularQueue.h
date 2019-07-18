#pragma once
#include "../ThreadSync/ThreadSync.h"

static const size_t MAX_QUEUE_LENGTH = 256;

template<typename T>
class CCircularQueue : public CMultiThreadSync<T> {
private:
	T m_Queue[MAX_QUEUE_LENGTH];

private:
	size_t m_Head, m_Tail;

public:
	CCircularQueue() : m_Head(0), m_Tail(0) {};
	
public:
	bool Push(const T& InData) {
		size_t TempTail = (m_Tail + 1) % MAX_QUEUE_LENGTH;
		if (TempTail == m_Tail) {
			return false;
		}
		m_Queue[TempTail] = Data;
		m_Tail = TempTail;
		return true;
	}

	bool Pop(T& OutData) {
		size_t TempHead = (m_Head + 1) % MAX_QUEUE_LENGTH;
		if (TempHead == m_Head) {
			return false;
		}
		OutData = m_Queue[TempHead];
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
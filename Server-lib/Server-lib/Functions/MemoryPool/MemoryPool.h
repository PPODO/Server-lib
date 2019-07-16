#pragma once
#include <cstdlib>
#include <Windows.h>
#include "../ThreadSync/ThreadSync.h"

template<typename T, size_t ALLOC_BLOCK_SIZE = 50>
class CMemoryPool : public CMultiThreadSync<T> {
private:
	// 시작 주소
	static UCHAR* m_FreePointer;

private:
	// 메모리 하알다앙
	static void AllocBlock() {
		m_FreePointer = new UCHAR[sizeof(T) * ALLOC_BLOCK_SIZE];
		if (!m_FreePointer) {

		}

		UCHAR** Current = reinterpret_cast<UCHAR**>(m_FreePointer);
		UCHAR* Next = m_FreePointer;

		// 연결리스트와 비슷한 개념. 
		// ex) Current = 0x00, Next = 0x00, sizeof(T) = 4 일 경우
		// Next의 위치를 sizeof(T)만큼 이동시켜주고, Current에 Next의 메모리 위치를 기록함.
		// vector나 array를 사용해도 됨. 근데 딱히 그리 쓰고 싶지는 않음.
		for (size_t i = 0; i < ALLOC_BLOCK_SIZE; i++) {
			Next += sizeof(T);
			*Current = Next;
			Current = reinterpret_cast<UCHAR**>(Next);
		}
		// nullptr을 넣어줌으로써, 메모리가 부족한지 아닌지 판단
		*Current = nullptr;
	}

public:
	static void* operator new(std::size_t AllocLength) {
		CThreadSync Sync(this);

		if (!m_FreePointer) {
			AllocBlock();
		}
		UCHAR* ReturnPointer = m_FreePointer;
		if (!ReturnPointer) {

		}
		// *reinterpret_cast<UCHAR**>(ReturnPointer)를 하면, 기록된 다음 메모리의 위치를 가져올 수 있음.
		m_FreePointer = *reinterpret_cast<UCHAR**>(ReturnPointer);
		return ReturnPointer;
	}

	static void operator delete(void* DeletePointer) {
		CThreadSync Sync(this);

		// AllocBlock 함수의 *Current = Next와 같은 역할, 현재 delete된 메모리에 다음 메모리의 위치를 기록함으로써, 재활용이 가능하도록 함.
		*reinterpret_cast<UCHAR**>(DeletePointer) = m_FreePointer;
		m_FreePointer = static_cast<UCHAR*>(DeletePointer);
	}

};

template<typename T, size_t ALLOC_BLOCK_SIZE>
UCHAR* CMemoryPool<T, ALLOC_BLOCK_SIZE>::m_FreePointer;
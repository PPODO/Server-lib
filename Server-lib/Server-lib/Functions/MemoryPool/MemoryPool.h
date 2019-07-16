#pragma once
#include <cstdlib>
#include <Windows.h>
#include "../ThreadSync/ThreadSync.h"

template<typename T, size_t ALLOC_BLOCK_SIZE = 50>
class CMemoryPool : public CMultiThreadSync<T> {
private:
	// ���� �ּ�
	static UCHAR* m_FreePointer;

private:
	// �޸� �Ͼ˴پ�
	static void AllocBlock() {
		m_FreePointer = new UCHAR[sizeof(T) * ALLOC_BLOCK_SIZE];
		if (!m_FreePointer) {

		}

		UCHAR** Current = reinterpret_cast<UCHAR**>(m_FreePointer);
		UCHAR* Next = m_FreePointer;

		// ���Ḯ��Ʈ�� ����� ����. 
		// ex) Current = 0x00, Next = 0x00, sizeof(T) = 4 �� ���
		// Next�� ��ġ�� sizeof(T)��ŭ �̵������ְ�, Current�� Next�� �޸� ��ġ�� �����.
		// vector�� array�� ����ص� ��. �ٵ� ���� �׸� ���� ������ ����.
		for (size_t i = 0; i < ALLOC_BLOCK_SIZE; i++) {
			Next += sizeof(T);
			*Current = Next;
			Current = reinterpret_cast<UCHAR**>(Next);
		}
		// nullptr�� �־������ν�, �޸𸮰� �������� �ƴ��� �Ǵ�
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
		// *reinterpret_cast<UCHAR**>(ReturnPointer)�� �ϸ�, ��ϵ� ���� �޸��� ��ġ�� ������ �� ����.
		m_FreePointer = *reinterpret_cast<UCHAR**>(ReturnPointer);
		return ReturnPointer;
	}

	static void operator delete(void* DeletePointer) {
		CThreadSync Sync(this);

		// AllocBlock �Լ��� *Current = Next�� ���� ����, ���� delete�� �޸𸮿� ���� �޸��� ��ġ�� ��������ν�, ��Ȱ���� �����ϵ��� ��.
		*reinterpret_cast<UCHAR**>(DeletePointer) = m_FreePointer;
		m_FreePointer = static_cast<UCHAR*>(DeletePointer);
	}

};

template<typename T, size_t ALLOC_BLOCK_SIZE>
UCHAR* CMemoryPool<T, ALLOC_BLOCK_SIZE>::m_FreePointer;
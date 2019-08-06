#pragma once
#include "../CriticalSection/CriticalSection.h"
#include <iostream>

template<typename T>
class CMultiThreadSync {
	friend class CThreadSync;
private:
	// �ش� Ÿ���� ��� ������Ʈ�� Lock�� �Ǵ�.
	// ������ �غ���, ������� ������ �ϳ��� ����ǹǷ�, Lock�� ��� �ɴ� ����� ����.
	// ���� ���̴� �����⿡, �ش� ������Ʈ���� Lock�� �ɾ��ִ� �κ��� ����
	static CCriticalSection m_CriticalSection;

public:
	class CThreadSync {
	public:
		CThreadSync() {
			T::m_CriticalSection.Lock();
		}

		CThreadSync(CMultiThreadSync<T>::CThreadSync&) = delete;

		virtual ~CThreadSync() {
			T::m_CriticalSection.UnLock();
		}
	};
};

template<typename T>
CCriticalSection CMultiThreadSync<T>::m_CriticalSection;
#pragma once
#include "../CriticalSection/CriticalSection.h"

template<typename T>
class CMultiThreadSync {
	friend class CThreadSync;
private:
	// �ش� Ÿ���� ��� ������Ʈ�� Lock�� �Ǵ�.
	static CCriticalSection m_CriticalSectionForAllObject;
	// �ش� ������Ʈ���� Lock�� �Ǵ�.
	CCriticalSection m_CriticalSectionForOnlyThisObject;

public:
	class CThreadSync {
	private:
		CCriticalSection& m_CriticalSection;
	public:
		CThreadSync() {
			T::m_CriticalSectionForAllObject.Lock();
		}

		CThreadSync(CMultiThreadSync* const MultiThreadSyncPtr) : m_CriticalSection(MultiThreadSyncPtr->m_CriticalSectionForOnlyThisObject) {
			m_CriticalSection.Lock();
		}

		~CThreadSync() {
			if (m_CriticalSection.IsLocked()) {
				m_CriticalSection.UnLock();
			}
			else {
				T::m_CriticalSectionForAllObject.UnLock();
			}
		}
	};
};

template<typename T>
CCriticalSection CMultiThreadSync<T>::m_CriticalSectionForAllObject;
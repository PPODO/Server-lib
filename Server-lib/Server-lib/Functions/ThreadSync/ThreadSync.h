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
		CCriticalSection* m_CriticalSection;
	public:
		// �ش� Ÿ���� ��� ������Ʈ�� Lock�� �Ǵ�.
		CThreadSync() : m_CriticalSection(nullptr) {
			T::m_CriticalSectionForAllObject.Lock();
		}

		// �ش� ������Ʈ���� Lock�� �Ǵ�.
		CThreadSync(CMultiThreadSync* const MultiThreadSyncPtr) : m_CriticalSection(&MultiThreadSyncPtr->m_CriticalSectionForOnlyThisObject) {
			if (m_CriticalSection) {
				m_CriticalSection->Lock();
			}
		}

		CThreadSync(CMultiThreadSync<T>::CThreadSync&) = delete;

		virtual ~CThreadSync() {
			if (m_CriticalSection && m_CriticalSection->IsLocked()) {
				m_CriticalSection->UnLock();
			}
			else {
				T::m_CriticalSectionForAllObject.UnLock();
			}
		}
	};
};

template<typename T>
CCriticalSection CMultiThreadSync<T>::m_CriticalSectionForAllObject;
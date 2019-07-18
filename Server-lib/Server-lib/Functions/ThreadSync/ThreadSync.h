#pragma once
#include "../CriticalSection/CriticalSection.h"

template<typename T>
class CMultiThreadSync {
	friend class CThreadSync;
private:
	// 해당 타입의 모든 오브젝트에 Lock을 건다.
	static CCriticalSection m_CriticalSectionForAllObject;
	// 해당 오브젝트에만 Lock을 건다.
	CCriticalSection m_CriticalSectionForOnlyThisObject;

public:
	class CThreadSync {
	private:
		CCriticalSection* m_CriticalSection;
	public:
		// 해당 타입의 모든 오브젝트에 Lock을 건다.
		CThreadSync() : m_CriticalSection(nullptr) {
			T::m_CriticalSectionForAllObject.Lock();
		}

		// 해당 오브젝트에만 Lock을 건다.
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
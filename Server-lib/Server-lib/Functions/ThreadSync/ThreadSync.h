#pragma once
#include "../CriticalSection/CriticalSection.h"

template<typename T>
class CMultiThreadSync {
	friend class CThreadSync;
private:
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
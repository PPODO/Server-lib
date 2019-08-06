#pragma once
#include "../CriticalSection/CriticalSection.h"
#include <iostream>

template<typename T>
class CMultiThreadSync {
	friend class CThreadSync;
private:
	// 해당 타입의 모든 오브젝트에 Lock을 건다.
	// 생각을 해보니, 스레드는 무조건 하나만 실행되므로, Lock을 어떻게 걸던 상관이 없다.
	// 성능 차이는 없었기에, 해당 오브젝트에만 Lock을 걸어주는 부분은 삭제
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
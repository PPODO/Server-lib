#pragma once
#include <Windows.h>

class CCriticalSection {
private:
	CRITICAL_SECTION m_CriticalSection;

public:
	CCriticalSection() {
		InitializeCriticalSection(&m_CriticalSection);
	}

	~CCriticalSection() {
		DeleteCriticalSection(&m_CriticalSection);
	}

public:
	inline void Lock() {
		EnterCriticalSection(&m_CriticalSection);
	}

	inline void UnLock() {
		LeaveCriticalSection(&m_CriticalSection);
	}

};

class CCriticalSectionGuard {
private:
	CCriticalSection& m_CriticalSection;

public:
	CCriticalSectionGuard(CCriticalSection& _CriticalSection) : m_CriticalSection(_CriticalSection) {
		m_CriticalSection.Lock();
	}

	~CCriticalSectionGuard() {
		m_CriticalSection.UnLock();
	}
};
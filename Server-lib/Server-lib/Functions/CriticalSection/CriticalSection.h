#pragma once
#include <Windows.h>

class CriticalSection {
private:
	CRITICAL_SECTION m_CriticalSection;

public:
	CriticalSection() {
		InitializeCriticalSection(&m_CriticalSection);
	}

	~CriticalSection() {
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

class CriticalSectionGuard {
private:
	CriticalSection& m_CriticalSection;

public:
	CriticalSectionGuard(CriticalSection& _CriticalSection) : m_CriticalSection(_CriticalSection) {
		m_CriticalSection.Lock();
	}

	~CriticalSectionGuard() {
		m_CriticalSection.UnLock();
	}
};
#pragma once
#include <Windows.h>

class CCriticalSection {
private:
	CRITICAL_SECTION m_CriticalSection;

private:
	bool m_bIsLock;

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
		m_bIsLock = true;
	}

	inline void UnLock() {
		m_bIsLock = false;
		LeaveCriticalSection(&m_CriticalSection);
	}

	inline bool IsLocked() const { return m_bIsLock; }

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
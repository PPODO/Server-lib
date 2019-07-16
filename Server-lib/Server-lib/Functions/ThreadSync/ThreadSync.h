#pragma once
#include "../CriticalSection/CriticalSection.h"

class MultiThreadSync {
private:
	friend class ThreadSync;
public:
	class ThreadSync {
	private:


	public:
		ThreadSync() {
			// lock
		}

		~ThreadSync() {
			// unlock
		}
	};
};
#pragma once
#include <thread>

class CEventSelect {
private:
	std::thread m_SelectThread;

public:
	bool Initialize();
	
	bool Destroy();

};
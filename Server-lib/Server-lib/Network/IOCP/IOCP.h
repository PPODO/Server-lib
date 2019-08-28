#pragma once
#include "../PacketSession/PacketSession.h"
#include "../../Functions/CircularQueue/CircularQueue.h"
#include <vector>
#include <thread>
#include <array>

class CIOCP {
private:
	WSADATA m_WinSockData;

private:
	HANDLE m_hIOCP;
	HANDLE m_hWaitForInitialize;

private:
	CPacketSession* m_ListenSession;

private:
	std::vector<std::thread> m_WorkerThread;

private:
	CCircularQueue<PACKET_DATA*> m_PacketQueue;

private:
	bool CreateWorkerThread();
	bool ProcessWorkerThread();
	bool ProcessPacketThread();

protected:
	virtual bool OnIOConnect(void* const Object) = 0;
	virtual bool OnIODisconnect(void* const Object) = 0;
	virtual bool OnIORead(void* const Object, const uint16_t& RecvBytes) = 0;
	virtual bool OnIOWrite(void* const Object);
	virtual bool ProcessPacket(PACKET_DATA& PacketData) = 0;

protected:
	inline bool RegisterIOCompletionPort(const SOCKET& Socket, const ULONG_PTR& CompletionKey) {
		if (Socket == INVALID_SOCKET || m_hIOCP == NULL) {
			CLog::WriteLog(L"Register IO Completion Port : Invalid Socket or Invalid Handle Value!");
			return false;
		}

		if ((m_hIOCP = CreateIoCompletionPort(reinterpret_cast<HANDLE>(Socket), m_hIOCP, CompletionKey, 0)) == NULL) {
			CLog::WriteLog(L"Register IO Completion Port : Failed To Register Completion Port!");
			return false;
		}
		return true;
	}
	inline bool AddNewPacketAtQueue(PACKET_DATA* const NewPacketData) {
		if (NewPacketData) {
			m_PacketQueue.Push(NewPacketData);
		}
		return false;
	}

protected:
	template<typename T>
	inline void SetListenSesisonObject(T* const NewObject) { m_ListenSession = NewObject; }

protected:
	inline HANDLE GetIOCPHandle() const { return m_hIOCP; }

protected:
	template<typename SESSIONTYPE>
	inline SESSIONTYPE* const GetListenSession() const { return reinterpret_cast<SESSIONTYPE*>(m_ListenSession); };

public:
	CIOCP() : m_hIOCP(INVALID_HANDLE_VALUE), m_hWaitForInitialize(INVALID_HANDLE_VALUE), m_ListenSession(nullptr) {};

public:
	virtual bool Initialize();
	virtual bool Destroy();

};
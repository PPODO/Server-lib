#pragma once
#include "../PacketSession/PacketSession.h"
#include <thread>

class CEventSelect {
private:
	WSADATA m_WinSockData;

private:
	CPacketSession* m_Session;
	SOCKET m_SessionSocket;

private:
	std::thread m_EventSelectThread;

private:
	HANDLE m_hSelectHandle;
	HANDLE m_hWaitForInitialize;
	HANDLE m_hStopEvent;

private:
	bool ProcessEventSelect();

protected:
	//virtual bool OnIOAccept();
	//virtual bool OnIOConnect();
	//virtual bool OnIODisconnect();
	virtual bool OnIORead() = 0;
	//virtual bool OnIOWrite();

protected:
	template<typename SESSIONTYPE>
	inline void SetSessionSocketObject(SESSIONTYPE* const NewObject) { m_Session = NewObject; };

public:
	CEventSelect() : m_Session(nullptr), m_hSelectHandle(INVALID_HANDLE_VALUE), m_hStopEvent(INVALID_HANDLE_VALUE), m_hWaitForInitialize(INVALID_HANDLE_VALUE), m_SessionSocket(INVALID_SOCKET) {
		if (WSAStartup(WINSOCK_VERSION, &m_WinSockData) == SOCKET_ERROR) {
			CLog::WriteLog(L"Initialize Event Select : WSAStartup Failure!");
			throw "";
		}
	};

public:
	inline bool Write(const PACKET::PACKET_INFORMATION& Info, const char* const DataBuffer, const uint16_t& DataLength) {
		return CSocketSystem::Write(m_Session->GetTCPSocket(), Info, DataBuffer, DataLength, nullptr);
	}
	inline bool WriteTo(bool bIsReliable, const CSocketAddress& SendAddress, const PACKET::PACKET_INFORMATION& Info, const char* const DataBuffer, const uint16_t& DataLength) {
		if (bIsReliable) {
			RELIABLE_DATA* NewData = new RELIABLE_DATA(m_Session, const_cast<CSocketAddress&>(SendAddress), Info, DataBuffer, DataLength);
			return CSocketSystem::WriteToQueue(m_Session->GetUDPSocket(), NewData);
		}
		return CSocketSystem::WriteTo(m_Session->GetUDPSocket(), false, SendAddress, Info, DataBuffer, DataLength, m_Session->GetOverlappedByIOType(EIOTYPE::EIOTYPE_WRITE));
	}

public:
	virtual bool Initialize(const CSocketAddress& ConnectionAddress, const EPROTOCOLTYPE& ProtocolType);
	virtual bool Destroy();

public:
	template<typename SESSIONTYPE>
	inline SESSIONTYPE* GetSessionSocket() const { return m_Session; }

};
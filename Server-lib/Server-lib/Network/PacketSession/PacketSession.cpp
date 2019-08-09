#include "PacketSession.h"
#include "../../Functions/Log/Log.h"

CPacketSession::CPacketSession() : m_CurrentBufferBytes(0) {
	ZeroMemory(m_PacketBuffer, MAX_RECEIVE_BUFFER_LENGTH);
}

bool CPacketSession::CopyBufferDataForIOCP(const USHORT & ReceivedBytes) {
	CThreadSync Sync;

	if (!CNetworkSession::GetBufferData(m_PacketBuffer + m_CurrentBufferBytes, ReceivedBytes)) {
		CLog::WriteLog(L"Buffer Data Copy Failure!");
		return false;
	}
	m_CurrentBufferBytes += ReceivedBytes;
	return true;
}

bool CPacketSession::CopyBufferDataForEventSelect() {
	CThreadSync Sync;

	USHORT ReceivedBytes = 0;
	if (!CNetworkSession::ReceiveForEventSelect(m_PacketBuffer + m_CurrentBufferBytes, ReceivedBytes)) {
		CLog::WriteLog(L"Buffer Data Copy Failure!");
		return false;
	}
	m_CurrentBufferBytes += ReceivedBytes;
	return true;
}

#include "NetworkSession.h"

CNetworkSession::CNetworkSession() : m_TCPSocket(new CTCPIPSocket), m_UDPSocket(new CUDPIPSocket) {
	ZeroMemory(m_ReceiveBuffer, MAX_RECEIVE_BUFFER_LENGTH);
}

bool CNetworkSession::GetBufferData(CHAR * OutBuffer, const USHORT& Length) {
	CThreadSync Sync;
	if (!OutBuffer && Length <= 0) {
		return false;
	}
	CopyMemory(OutBuffer, m_ReceiveBuffer, Length);
	return true;
}
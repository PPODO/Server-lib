#include "NetworkBase.h"

CNetworkBase::CNetworkBase() {
}

CNetworkBase::~CNetworkBase() {
}

void CNetworkBase::Initialize() {
	CThreadSync Sync(this);

	ZeroMemory(&m_AcceptOveralapped.m_Overlapped, sizeof(WSAOVERLAPPED));
	ZeroMemory(&m_ReciveDataOverlapped.m_Overlapped, sizeof(WSAOVERLAPPED));
	ZeroMemory(&m_SendDataOverlapped.m_Overlapped, sizeof(WSAOVERLAPPED));

	m_AcceptOveralapped.m_IOType = EIOTYPE::EIOTYPE_ACCEPT;
	m_ReciveDataOverlapped.m_IOType = EIOTYPE::EIOTYPE_READ;
	m_SendDataOverlapped.m_IOType = EIOTYPE::EIOTYPE_WRITE;

	m_AcceptOveralapped.Owner = m_ReciveDataOverlapped.Owner = m_SendDataOverlapped.Owner = this;
}

void CNetworkBase::Clear() {
	CThreadSync Sync(this);

	m_TCPSocket.Shutdown();
	Initialize();
}

bool CNetworkBase::InitializeSocket(const IPPROTO& ProtocolType, const CSocketAddress& BindAddress) {
	CThreadSync Sync(this);

	switch (ProtocolType) {
	case IPPROTO::IPPROTO_TCP:
		return m_TCPSocket.BindTCP() && m_TCPSocket.Listen(BindAddress, SOMAXCONN);
	case IPPROTO::IPPROTO_UDP:
		return m_UDPSocket.BindUDP(BindAddress);
	default:
		return false;
	}
}

bool CNetworkBase::Accept(const SOCKET & ListenSocket) {
	return m_TCPSocket.Accept(ListenSocket, m_AcceptOveralapped);
}
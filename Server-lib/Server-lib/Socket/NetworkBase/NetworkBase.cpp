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

	m_AcceptOveralapped.m_Owner = m_ReciveDataOverlapped.m_Owner = m_SendDataOverlapped.m_Owner = this;
}

bool CNetworkBase::Initialize(const CNetworkBase* const ListenSocket) {
	CThreadSync Sync(this);

	if (!ListenSocket) {
		return false;
	}
	Initialize();

	return m_TCPSocket.Accept(ListenSocket->m_TCPSocket, m_AcceptOveralapped);
}

void CNetworkBase::Clear() {
	CThreadSync Sync(this);

	m_TCPSocket.Shutdown();
	m_UDPSocket.Shutdown();
}

bool CNetworkBase::OnIOConnect() {
	return m_TCPSocket.InitializeRecvBuffer_IOCP(m_ReciveDataOverlapped);
}

bool CNetworkBase::InitializeSocket(const bool& bIsClient, const IPPROTO & ProtocolType, const CSocketAddress & Address) {
	if (ProtocolType == IPPROTO_UDP) {
		return m_UDPSocket.BindUDP(Address);
	}
	else if(ProtocolType == IPPROTO_TCP) {
		return m_TCPSocket.BindTCP() && (bIsClient ? (m_TCPSocket.Connect(Address)) : m_TCPSocket.Listen(Address, SOMAXCONN));
	}
	return false;
}

bool CNetworkBase::ReadIOCP(CHAR * InData, const UINT16 & DataLength) {
	CThreadSync Sync(this);
	return m_TCPSocket.CopyRecvBuffer_IOCP(InData, DataLength);
}

bool CNetworkBase::ReadSelect(CHAR * InData, UINT16 & RecvLength) {
	CThreadSync Sync(this);
	return m_TCPSocket.ReadRecvBuffer_Select(InData, RecvLength, m_ReciveDataOverlapped);
}
#include "NetworkBase.h"

CNetworkBase::CNetworkBase() {
}

CNetworkBase::~CNetworkBase() {
}

void CNetworkBase::Initialize() {
	ZeroMemory(&m_AcceptOveralapped.m_Overlapped, sizeof(WSAOVERLAPPED));
	ZeroMemory(&m_ReciveDataOverlapped.m_Overlapped, sizeof(WSAOVERLAPPED));
	ZeroMemory(&m_SendDataOverlapped.m_Overlapped, sizeof(WSAOVERLAPPED));

	m_AcceptOveralapped.m_IOType = EIOTYPE::EIOTYPE_ACCEPT;
	m_ReciveDataOverlapped.m_IOType = EIOTYPE::EIOTYPE_READ;
	m_SendDataOverlapped.m_IOType = EIOTYPE::EIOTYPE_WRITE;

	m_AcceptOveralapped.Owner = m_ReciveDataOverlapped.Owner = m_SendDataOverlapped.Owner = this;
}

void CNetworkBase::Destroy() {

}
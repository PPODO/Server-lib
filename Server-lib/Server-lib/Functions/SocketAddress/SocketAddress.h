#pragma once
#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>

class CSocketAddress {
	friend class CTCPIPSocket;
private:
	sockaddr m_Address;

private:
	inline sockaddr_in* GetSockAddrIn() { return reinterpret_cast<sockaddr_in*>(&m_Address); }

public:
	CSocketAddress(const UINT16& Port) {
		GetSockAddrIn()->sin_family = AF_INET;
		GetSockAddrIn()->sin_port = htons(Port);
		ZeroMemory(&GetSockAddrIn()->sin_zero, sizeof(GetSockAddrIn()->sin_zero));
	};

	CSocketAddress(const std::string& Address, const UINT16& Port) : CSocketAddress(Port) {
		InetPtonA(AF_INET, Address.c_str(), &GetSockAddrIn()->sin_addr);
	}

	CSocketAddress(LPCTSTR& Address, const UINT16& Port) : CSocketAddress(Port) {
		InetPtonW(AF_INET, Address, &GetSockAddrIn()->sin_addr);
	}

	CSocketAddress(CSocketAddress& NewSocketAddress) {
		CopyMemory(&m_Address, &NewSocketAddress.m_Address, GetSize());
	}

public:
	static size_t GetSize() { return sizeof(m_Address); }

};
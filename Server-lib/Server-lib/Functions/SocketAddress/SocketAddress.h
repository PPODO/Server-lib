#pragma once
#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include "../Log/Log.h"

class CSocketAddress {
private:
	sockaddr m_Address;

private:
	inline sockaddr_in* GetSockAddrIn() { return reinterpret_cast<sockaddr_in*>(&m_Address); }

private:
	static inline in_addr* GetHostIPAddress() {
		char HostName[32] = { "\0" };
		if (gethostname(HostName, 32) == SOCKET_ERROR) {
			CLog::WriteLog(L"Get Host Name is Failure! - %d", WSAGetLastError());
			return nullptr;
		}
		if (hostent* HostInformation = gethostbyname(HostName)) {
			return reinterpret_cast<in_addr*>(HostInformation->h_addr_list[0]);
		}
		CLog::WriteLog(L"Get Host Information is Failure! - %d", WSAGetLastError());
		return nullptr;
	}

public:
	CSocketAddress() {
		ZeroMemory(&m_Address, GetSize());
	};

	CSocketAddress(const UINT16& Port) {
		in_addr* IPAddress = GetHostIPAddress();
		GetSockAddrIn()->sin_family = AF_INET;
		GetSockAddrIn()->sin_port = htons(Port);
		GetSockAddrIn()->sin_addr = IPAddress ? *IPAddress : IN_ADDR();
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
	static const char* const GetIPAddress() {
		return reinterpret_cast<const char* const>(GetHostIPAddress());
	}

};
#pragma once
#include "../../Network/PacketSession/NetworkSession/TCPIP/TCPIPSocket.h"
#include "../MemoryPool/MemoryPool.h"
#include "../ThreadSync/ThreadSync.h"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/string.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <string>

typedef struct PACKET_INFORMATION {
public:
	uint16_t m_PacketSize;
	uint8_t m_PacketType;

public:
	PACKET_INFORMATION(const uint16_t& PacketSize, const uint8_t& PacketType) : m_PacketSize(PacketSize), m_PacketType(PacketType) {};

public:
	static size_t GetSize() { return sizeof(PACKET_INFORMATION); }

};

class CBasePacket {
	friend boost::serialization::access;
	friend class CPacketSession;
private:
	const uint8_t m_PacketType;

protected:
	template<typename Archive>
	void serialize(Archive& ar, const unsigned int Version) {
		ar& m_MessageType;
	}

public:
	uint8_t m_MessageType;

public:
	CBasePacket(const uint8_t& PacketType, const uint8_t& MessageType) : m_PacketType(PacketType), m_MessageType(MessageType) {}

};

template<typename T>
void Serialize(const T& Packet, std::string& Buffer) {
	if (std::is_arithmetic<T>() || std::is_enum<T>()) {
		CLog::WriteLog(L"Serialize : Only Types Support That Inherit BasePacket!");
		return;
	}
	boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> OutStream(Buffer);
	boost::archive::binary_oarchive oa(OutStream, boost::archive::no_header);
	oa << Packet;
}

template<typename T>
void DeSerialize(const char* Buffer, const USHORT& BufferLength, T& Packet) {
	if (std::is_arithmetic<T>() || std::is_enum<T>()) {
		CLog::WriteLog(L"DeSerialize : Only Types Support That Inherit BasePacket!");
		return;
	}
	boost::iostreams::stream_buffer<boost::iostreams::basic_array_source<char>> InStream(Buffer, BufferLength);
	boost::archive::binary_iarchive ia(InStream, boost::archive::no_header);
	ia >> Packet;
}
#pragma once
#include "../ThreadSync/ThreadSync.h"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/string.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <string>

class CBasePacket {
	friend boost::serialization::access;
protected:
	template<typename Archive>
	void serialize(Archive& ar, const unsigned int Version) {
		ar& m_PacketType;
		ar& m_MessageType;
	}

public:
	uint8_t m_PacketType;
	uint8_t m_MessageType;

public:
	CBasePacket(const uint8_t PacketType = 0, const uint8_t MessageType = 0) : m_PacketType(PacketType), m_MessageType(MessageType) {}

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
		CLog::WriteLog(L"Serialize : Only Types Support That Inherit BasePacket!");
		return;
	}
	boost::iostreams::stream_buffer<boost::iostreams::basic_array_source<char>> InStream(Buffer, BufferLength);
	boost::archive::binary_iarchive ia(InStream, boost::archive::no_header);
	ia >> Packet;
}
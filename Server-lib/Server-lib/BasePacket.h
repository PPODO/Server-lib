#pragma once
#include <boost/serialization/serialization.hpp>

class BasePacket {
	friend boost::serialization::access;
public:
	uint16_t m_PacketSize;
	uint8_t m_PacketType;
	uint8_t m_MessageType;

public:
	BasePacket() : m_PacketSize(0), m_PacketType(0), m_MessageType(0) {}

};
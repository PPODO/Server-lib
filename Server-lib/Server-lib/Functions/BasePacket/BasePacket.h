#pragma once
#include <boost/serialization/serialization.hpp>

class CBasePacket {
	friend boost::serialization::access;
public:
	uint16_t m_PacketSize;
	uint8_t m_PacketType;
	uint8_t m_MessageType;

};
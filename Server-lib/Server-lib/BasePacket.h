#pragma once
#include <iostream>

class BasePacket {
public:
	uint16_t m_PacketLength;
	uint16_t m_PacketType;
	uint16_t m_UniqueKey;

};
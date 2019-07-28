#pragma once
#include <iostream>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

class BasePacket {
	friend boost::serialization::access;
public:
	uint16_t m_PacketLength;
	uint16_t m_PacketType;
	uint16_t m_UniqueKey;

protected:
	template<typename Archive>
	void serialize(Archive& ar, unsigned short Version) {
		ar & m_PacketLength;
		ar & m_PacketType;
		ar & m_UniqueKey;
	}

};
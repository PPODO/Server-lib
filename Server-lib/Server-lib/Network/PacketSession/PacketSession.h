#pragma once
#include "NetworkSession/NetworkSession.h"
#include "../../Functions/CircularQueue/CircularQueue.h"
#include <boost/serialization/string.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

class CPacketSession : public CNetworkSession {
private:
	char m_PacketBuffer[MAX_RECEIVE_BUFFER_LENGTH];
	USHORT m_CurrentBufferBytes;

private:
	USHORT m_NewPacketSize;

private:
	// 데이터를 보내는 도중에 버퍼가 메모리에서 삭-제 되면 안되기 때문에
	// 전송 완료가 되기 전 까지 보낸 버퍼를 스톡해주는 역할
	CCircularQueue m_WriteQueue;

public:
	CPacketSession();

private:
	template<typename T>
	inline void Serialize(const T& Data, std::string& DataBuffer);
	template<typename T>
	inline void DeSerialize(const char* DataBuffer, const USHORT& DataSize, T& OutData);

public:
	template<typename T>
	bool PacketAnalysis(T& Packet);
	template<typename T>
	bool WritePacket(const T& Packet);

public:
	bool CopyBufferDataForIOCP(const USHORT& ReceivedBytes);
	bool CopyBufferDataForEventSelect();

};

template<typename T>
inline void CPacketSession::Serialize(const T & Data, std::string& DataBuffer) {
	boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> OutStream(DataBuffer);
	boost::archive::binary_oarchive oBinaryStream(OutStream, boost::archive::no_header);
	oBinaryStream << Data;
}

template<typename T>
inline void CPacketSession::DeSerialize(const char * DataBuffer, const USHORT & DataSize, T& OutData) {
	boost::iostreams::stream_buffer<boost::iostreams::basic_array_source<char>> InStream(DataBuffer, DataSize);
	boost::archive::binary_iarchive iBinaryStream(InStream, boost::archive::no_header);
	iBinaryStream >> OutData;
}

template<typename T>
inline bool CPacketSession::PacketAnalysis(T& Packet) {
	if (m_NewPacketSize == 0) {
		uint16_t PacketSize = 0;
		DeSerialize(m_PacketBuffer, sizeof(uint16_t), PacketSize);
		m_NewPacketSize = PacketSize;
	}
	else if (m_NewPacketSize <= m_CurrentBufferBytes) {
		DeSerialize(m_PacketBuffer, m_NewPacketSize, Packet);
		m_NewPacketSize = 0;
		return true;
	}
	return false;
}

template<typename T>
inline bool CPacketSession::WritePacket(const T& Packet) {
	std::string TempBuffer;

	Serialize(Packet, TempBuffer);

	CHAR* WriteBuffer =  m_WriteQueue.Push(QUEUE_DATA(this, TempBuffer.c_str(), TempBuffer.length()));
	
	return CNetworkSession::Write(WriteBuffer, TempBuffer.length());
}
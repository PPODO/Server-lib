#pragma once
#include "../../Network/PacketSession/NetworkSession/NetworkProtocol/NetworkProtocol.h"
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
#include <sstream>

namespace PACKET {
	class CPacketSystem;

	namespace DETAIL {
		class CBasePacket {
			friend CPacketSystem;
		protected:
			friend boost::serialization::access;

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

	}

	typedef struct PACKET_INFORMATION {
	public:
		uint16_t m_PacketSize;
		uint8_t m_PacketType;

	public:
		PACKET_INFORMATION() : m_PacketSize(0), m_PacketType(0) {}
		PACKET_INFORMATION(const uint16_t& PacketSize, const uint8_t& PacketType) : m_PacketSize(PacketSize), m_PacketType(PacketType) {};
	
	public:
		static size_t GetSize() { return sizeof(PACKET_INFORMATION); }

	};

	template<typename T, size_t PoolSize>
	class CPacket : public DETAIL::CBasePacket, public CMemoryPool<T, PoolSize> {
		friend boost::serialization::access;
	public:
		CPacket(const uint8_t& PacketType, const uint8_t& MessageType) : DETAIL::CBasePacket(PacketType, MessageType) {}

	protected:
		template<typename Archive>
		void serialize(Archive& ar, const unsigned int Version) {
			DETAIL::CBasePacket::serialize(ar, Version);
		}

	};

	class CPacketSystem {
	public:
		template<typename T>
		static PACKET_INFORMATION Serialize(const T& Packet, std::string& Buffer) {
			// string 변수에 데이터가 입력이 되려면 binary_oarchive의 소멸자가 호출되야 함.
			{
				boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> OutStream(Buffer);
				boost::archive::binary_oarchive oa(OutStream, boost::archive::no_header);
				oa << Packet;
			}
			
			return PACKET_INFORMATION(Buffer.length(), reinterpret_cast<const DETAIL::CBasePacket*>(&Packet)->m_PacketType);
		}

		template<typename T>
		static void DeSerialize(const char* Buffer, const USHORT& BufferLength, T& Packet) {
			boost::iostreams::stream_buffer<boost::iostreams::basic_array_source<char>> InStream(Buffer, BufferLength);
			boost::archive::binary_iarchive ia(InStream, boost::archive::no_header);
			ia >> Packet;
		}

	};

}
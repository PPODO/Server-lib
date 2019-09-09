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
	struct Vector{
		friend boost::serialization::access;
	public:
		float X, Y, Z;

	protected:
		template<typename Archive>
		void serialize(Archive& ar, unsigned int Version) {
			ar& X;
			ar& Y;
			ar& Z;
		}

	public:
		Vector() : X(0.f), Y(0.f), Z(0.f) {}
		Vector(float x, float y, float z) : X(x), Y(y), Z(z) {};

	};

	class CPacketSystem;

	namespace DETAIL {
		#pragma pack(push, 8)
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
				ar& m_PacketNumber;
			}

		public:
			uint8_t m_MessageType;
			uint16_t m_PacketNumber;

		public:
			CBasePacket(const uint8_t& PacketType, const uint8_t& MessageType, const uint16_t& PacketNumber) : m_PacketType(PacketType), m_MessageType(MessageType), m_PacketNumber(PacketNumber) {}

		};

	}
	#pragma pack(pop)

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

	#pragma pack(push, 8)
	template<typename T, size_t PoolSize>
	class CPacket : public DETAIL::CBasePacket, public CMemoryPool<T, PoolSize> {
		friend boost::serialization::access;
	public:
		CPacket(const uint8_t& PacketType, const uint8_t& MessageType, const uint16_t& PacketNumber) : DETAIL::CBasePacket(PacketType, MessageType, PacketNumber) {}

	protected:
		template<typename Archive>
		void serialize(Archive& ar, const unsigned int Version) {
			DETAIL::CBasePacket::serialize(ar, Version);
		}

	};
	#pragma pack(pop)

	class CPacketSystem {
	public:
		template<typename T>
		static PACKET_INFORMATION Serialize(const T& Packet, std::string& Buffer) {
			if (!std::is_base_of<DETAIL::CBasePacket, T>()) {
				return PACKET_INFORMATION();
			}
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
			if (!std::is_base_of<DETAIL::CBasePacket, T>()) {
				return;
			}

			boost::iostreams::stream_buffer<boost::iostreams::basic_array_source<char>> InStream(Buffer, BufferLength);
			boost::archive::binary_iarchive ia(InStream, boost::archive::no_header);
			ia >> Packet;
		}

	};

}
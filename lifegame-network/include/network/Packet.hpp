#pragma once

#include <network/Sockets.hpp>
#include <network/Datagram.hpp>
#include <cstdint>
#include <array>

namespace network
{
	namespace udp
	{
		/*
		* @brief A packet send through the network.
		*/
		struct Packet
		{
			typedef uint16_t IdType;

			enum class Type : uint8_t
			{
				// A complete packet filling into a single datagram.
				Complete,
				// A partial packet divided into n datagrams. 
				PartialFirst,
				PartialInBetween,
				PartialLast,
			};

			struct Header
			{
				IdType id;
				uint16_t size{ 0 }; // Body size.
				Type type;
			};

			static constexpr uint16_t PacketMaxSize = Datagram::BodyMaxSize;
			static constexpr uint16_t HeaderSize = sizeof(Header);
			static constexpr uint16_t BodyMaxSize = PacketMaxSize - HeaderSize;
			static constexpr size_t MaxPacketsPerMessage = 32;
			static constexpr size_t MaxMessageSize = MaxPacketsPerMessage * BodyMaxSize;

			Header header;
			std::array<uint8_t, BodyMaxSize> body;

			/*inline*/ IdType id() const { return header.id; }
			/*inline*/ uint16_t bodySize() const { return header.size; }
			/*inline*/ Type type() const { return header.type; }
			/*inline*/ uint16_t size() const { return HeaderSize + header.size; }
			/*inline*/ uint8_t* bodyData() { return body.data(); }
			///*inline*/ const uint8_t* bodyData() const noexcept { return body.data(); }

			/*
			* @brief Serialize the packet as a 'PacketUnit' array.
			*
			* @return a 'PacketUnit' array.
			*/
			inline const PacketUnit* serialize() const { return reinterpret_cast<const PacketUnit*>(this); }
		};

		/*
		* @brief Unserialize a 'PacketUnit' array as a packet.
		*
		* @param the 'PacketUnit' array.
		*
		* @return a packet.
		*/
		inline const Packet* unserialize(const PacketUnit* packet) { return reinterpret_cast<const Packet*>(packet); }

	}
}
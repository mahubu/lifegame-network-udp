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
			using IdType = uint16_t;

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
				IdType id{ 0 };
				uint16_t size{ 0 }; // Body size.
				Type type{ Type::Complete };
			};

			static constexpr uint16_t PacketMaxSize = Datagram::BodyMaxSize;
			static constexpr uint16_t HeaderSize = sizeof(Header);
			static constexpr uint16_t BodyMaxSize = PacketMaxSize - HeaderSize;
			static constexpr size_t MaxPacketsPerMessage = 32; // Maximum allowed message size: ~40ko
			static constexpr size_t MaxMessageSize = MaxPacketsPerMessage * BodyMaxSize;

			Header header;
			std::array<PacketUnit, BodyMaxSize> body{ 0 };

			inline IdType id() const { return header.id; }
			inline uint16_t bodySize() const { return header.size; }
			inline Type type() const { return header.type; }
			inline uint16_t size() const { return HeaderSize + header.size; }
			inline PacketUnit* bodyData() { return body.data(); }

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
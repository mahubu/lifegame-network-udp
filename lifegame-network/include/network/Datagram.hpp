#pragma once

#include <network/Sockets.hpp>
#include <cstdint>
#include <array>

namespace network
{
	namespace udp
	{
		/*
		* @brief A datagram send through the network.
		*/
		struct Datagram
		{
			typedef uint16_t IdType;

			struct Header
			{
				IdType id;
				IdType newestAck; // Latest datagram packet id received on the remote side.
				uint64_t previousAcks; // Previous 64 known acks on the remote side, as a 64-bits mask : '1' is acked, '0' is not (yet?) acked.
			};

			// Modern network MTU is expected to have at least a value of 1400 bytes.
			static constexpr size_t DatagramMaxSize = 1400;
			static constexpr size_t HeaderSize = sizeof(Header);
			static constexpr size_t BodyMaxSize = DatagramMaxSize - HeaderSize;

			Header header{ 0 };
			std::array<PacketUnit, BodyMaxSize> body{ 0 };
			size_t bodySize{ 0 };
			size_t size() const { return HeaderSize + bodySize; }
		};
	}
}
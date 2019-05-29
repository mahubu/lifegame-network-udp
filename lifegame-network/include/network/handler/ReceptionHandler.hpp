#pragma once

#include <network/Packet.hpp>
#include <network/Sockets.hpp>
#include <network/event/Event.hpp>
#include <memory>
#include <vector>

namespace network
{
	namespace handler
	{
		/*
		* @brief An handler for receiving packets.
		*/
		class ReceptionHandler
		{

		public:
			ReceptionHandler() = default;
			~ReceptionHandler() = default;

			/*
			* @brief Actions to take when a packet is received.
			*
			* @param rawPacket the received packet.
			* @param rawPacketSize the received packet size.
			*/
			void onReceived(const PacketUnit* rawPacket, const size_t rawPacketSize);

			/*
			* @brief Unserialize the received packets.
			*
			* @return the unserialized packets.
			*/
			std::vector<std::vector<PacketUnit>> unserialize();

		private:
			void onPacketReceived(const udp::Packet* packet);

			std::vector<udp::Packet> queue_;
			udp::Packet::IdType lastProcessed_{ std::numeric_limits<udp::Packet::IdType>::max() };

		};

	}
}
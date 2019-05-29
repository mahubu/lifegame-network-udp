#pragma once

#include <network/Sockets.hpp>
#include <network/Packet.hpp>
#include <network/event/Event.hpp>
#include <list>
#include <vector>

namespace network
{
	namespace handler
	{
		/*
		* @brief An handler for sending packets.
		*/
		class SendingHandler
		{

		public:
			SendingHandler() = default;
			~SendingHandler() = default;

			/*
			* @brief Enqueue a packet to be send.
			*
			* @param rawPacket the packet to send.
			*/
			void queue(std::vector<PacketUnit>&& rawPacket);

			/*
			* @brief Serialize the enqueued packets to an unique packet.
			*
			* @param rawPacket the unique packet containing enqueued packets.
			* @param rawPacketSize the unique packet size.
			*
			* @return the unique packet size.
			*/
			size_t serialize(PacketUnit* rawPacket, const size_t rawPacketSize);

		private:
			std::vector<udp::Packet> queue_;
			udp::Packet::IdType newId_{ 0 };

		};
	}
}
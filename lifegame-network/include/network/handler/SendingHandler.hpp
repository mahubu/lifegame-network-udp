#pragma once

#include <network/Sockets.hpp>
#include <network/Packet.hpp>
#include <network/PacketHolder.hpp>
#include <network/event/Event.hpp>
#include <chrono>
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
			* @brief Enqueue a packet to be sent.
			*
			* @param rawPacket the packet to sent.
			*/
			void queue(std::vector<PacketUnit>&& rawPacket);

			/*
			* @brief Serialize the enqueued packets to an unique datagram.
			*
			* @param datagram the unique datagram id.
			* @param rawPacket the unique datagram containing enqueued packets.
			* @param rawPacketSize the unique datagram size.
			*
			* @return the unique packet size.
			*/
			size_t serialize(udp::Datagram::IdType datagram, PacketUnit* rawPacket, const size_t rawPacketSize);

			/*
			* @brief Action taken when a sended packet has been acked.
			*
			* @param datagram the datagram id of the acked packet.
			*/
			void onAcked(udp::Datagram::IdType datagram);

			/*
			* @brief Action taken when a sended packet has been lost.
			*
			* @param datagram the datagram id of the lost packet.
			*/
			void onLost(udp::Datagram::IdType datagram);

			/*
			* @return the queue size of sending messages.
			*/
			size_t size() const { return queue_.size(); };

			/*
			* @return whether (or not) message sending is idle & client may be disconnected.
			*/
			bool idle() const { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastSerializedTime_).count() >= 1000; };

			/*
			* @return the next packet identifier that will be sent.
			*/
			udp::Packet::IdType nextId() const { return nextId_; };

			/*
			* @return the packet identifier that is expected to be sent next.
			*/
			udp::Packet::IdType nextExpected() const { return nextExpectedId_; };

		private:
			std::vector<udp::PacketHolder> queue_;
			udp::Packet::IdType nextId_{ 0 };
			udp::Packet::IdType nextExpectedId_{ 0 };
			std::chrono::time_point<std::chrono::system_clock> lastSerializedTime_{ std::chrono::system_clock::now() };

		};
	}
}
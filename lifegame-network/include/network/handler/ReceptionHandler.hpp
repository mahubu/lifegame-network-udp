#pragma once

#include <network/Packet.hpp>
#include <network/Sockets.hpp>
#include <network/event/Event.hpp>
#include <chrono>
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
			* @param the unserialized packets.
			*/
			void unserialize(std::vector<std::vector<PacketUnit>>& packets);

			/*
			* @return whether (or not) message sending is idle & client may be disconnected.
			*/
			bool idle() const { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastProcessedTime_).count() >= 1000; };

			/*
			* @return the queue size of receiving messages.
			*/
			size_t size() const { return queue_.size(); };

			/*
			* @return the last processed packet identifier.
			*/
			udp::Packet::IdType lastProcessed() const { return lastProcessedId_; };

		private:
			void onPacketReceived(const udp::Packet* packet);

			std::vector<udp::Packet> queue_;
			udp::Packet::IdType lastProcessedId_{ std::numeric_limits<udp::Packet::IdType>::max() };
			std::chrono::time_point<std::chrono::system_clock> lastProcessedTime_{ std::chrono::system_clock::now() };

		};

	}
}
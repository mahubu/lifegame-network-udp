#pragma once

#include <network/Datagram.hpp>
#include <network/event/Event.hpp>
#include <network/handler/AckHandler.hpp>
#include <network/handler/ReceptionHandler.hpp>
#include <network/handler/SendingHandler.hpp>

#include <vector>

namespace network
{
	namespace udp
	{
		/*
		* @brief A peer client.
		*/
		class ClientHandler;
		class Client
		{
		public:
			Client(ClientHandler& handler, const sockaddr_storage& address);
			Client(const Client& client) = delete;
			Client& operator=(const Client& client) = delete;
			Client(Client&& client) = delete;
			Client& operator=(Client&& client) = delete;
			~Client() = default;

			/*
			* @brief Enqueue a packet to deliver to the client.
			*
			* @param packet the packet to send, as a vector of packet units.
			*/
			void queue(std::vector<PacketUnit>&& packet);

			/*
			* @brief Send enqueued messages to the client.
			*/
			void send();

			/*
			* @brief Actions to take when a datagram is received by the client.
			*
			* @param datagram the received datagram.
			*/
			void onReceived(Datagram&& datagram);

			/*
			* @return the client adress.
			*/
			const sockaddr_storage& address() const { return address_; }

		private:
			ClientHandler& clientHandler_;
			sockaddr_storage address_;
			Datagram::IdType nextDatagramIdToSend_{ 0 };
			handler::AckHandler receptionAck_;	// Detect missing received datagrams and duplicates.
			handler::AckHandler sendingAck_;		// Detect lose sended datagrams.
			handler::ReceptionHandler receptionHandler_;
			handler::SendingHandler sendingHandler_;

			bool fill(Datagram& datagram);
			void onPacketReceived(const uint8_t* packet, const size_t packetSize);

		};
	}
}
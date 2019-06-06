#pragma once

#include <network/Sockets.hpp>
#include <network/event/Event.hpp>
#include <map>
#include <memory>
#include <vector>

namespace network
{
	namespace udp
	{
		/*
		* @brief An handler for peer clients.
		*/
		class Client;
		class ClientHandler
		{
			// TODO avoid using friend if possible!
			friend class Client;

		public:
			ClientHandler() = default;
			ClientHandler(const ClientHandler& handler) = delete;
			ClientHandler& operator=(const ClientHandler& handler) = delete;
			ClientHandler(ClientHandler&& handler) = delete;
			ClientHandler& operator=(ClientHandler&& handler) = delete;
			~ClientHandler() = default;

			/*
			* @brief Start up the client handler on a port.
			*
			* @param port the port to start with.
			*
			* @return whether (nor not) the handler could be start up.
			*/
			bool startup(uint16_t port);

			/*
			* @brief Shutdown the client handler.
			*/
			void shutdown();

			/*
			* @brief Enqueue a packet to deliver to a client.
			*
			* @param target the client of the packet, as an address.
			* @param packet the packet to send, as a vector of packet units.
			*/
			void queue(const sockaddr_storage& target, std::vector<PacketUnit>&& packet);

			/*
			* @brief Disconnect a client.
			*
			* @param target the client to disconnect, as an address.
			*/
			void disconnect(const sockaddr_storage& address);

			/*
			* @brief Send enqueued messages to the clients.
			*/
			void send();

			/*
			* @brief Receive messages for the clients.
			*/
			void receive();

			/*
			* @brief Retrieve the received messages of the clients.
			*
			* @return the messages.
			*/
			std::vector<std::unique_ptr<event::Event>> events();

		private:
			SOCKET socket_{ INVALID_SOCKET };
			std::vector<std::unique_ptr<Client>> clients_;
			std::vector<std::unique_ptr<event::Event>> events_;

			Client& getOrCreate(const sockaddr_storage& address);
			void onReceived(std::unique_ptr<event::Event>&& event);

		};
	}
}

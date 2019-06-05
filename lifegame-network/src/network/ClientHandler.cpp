#include <network/Sockets.hpp>
#include <network/Client.hpp>
#include <network/ClientHandler.hpp>
#include <network/handler/ReceptionHandler.hpp>
#include <network/handler/SendingHandler.hpp>
#include <network/event/Disconnection.hpp>
#include <algorithm>
#include <assert.h>
#include <iostream>

namespace network
{
	namespace udp
	{

		bool ClientHandler::startup(uint16_t port)
		{
			shutdown();
			socket_ = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (socket_ == INVALID_SOCKET)
			{
				return false;
			}

			sockaddr_in address;
			address.sin_addr.s_addr = INADDR_ANY;
			address.sin_port = htons(port);
			address.sin_family = AF_INET;
			int bound = ::bind(socket_, reinterpret_cast<sockaddr*>(&address), sizeof(address));
			if (bound != 0)
			{
				return false;
			}

			if (!nonBlocking(socket_))
			{
				return false;
			}

			return true;
		}

		void ClientHandler::shutdown() {
			if (socket_ != INVALID_SOCKET)
			{
				close(socket_);
			}
			socket_ = INVALID_SOCKET;
		}

		void ClientHandler::queue(const sockaddr_storage& target, std::vector<PacketUnit>&& packet)
		{
			auto& client = getOrCreate(target);
			client.queue(std::move(packet));
		}

		void ClientHandler::send()
		{
			for (auto& client : clients_)
			{
				client->send();
			}
		}

		void ClientHandler::receive()
		{
			while (true)
			{
				Datagram datagram;
				sockaddr_in from{ 0 };
				socklen_t length = sizeof(from);
				int received = ::recvfrom(socket_, reinterpret_cast<char*>(&datagram), Datagram::DatagramMaxSize, 0, reinterpret_cast<sockaddr*>(&from), &length);
				if (received > 0)
				{
					if (received > Datagram::HeaderSize)
					{
						datagram.bodySize = received - Datagram::HeaderSize;
						// TODO sockaddr_storage -> IPv4 + IPv6 support
						auto& client = getOrCreate(reinterpret_cast<sockaddr_storage&>(from));
						client.onReceived(std::move(datagram));
					}
					else
					{
						std::cerr << "Unexpected datagram: " << received << std::endl;
					}
				}
				else
				{
					if (received < 0)
					{
						//!< Gestion des erreurs
						auto error = error::latest();
						if (error != error::WOULDBLOCK)
						{
							std::cerr << "Reception error: " << error << std::endl;
						}
					}
					return;
				}
			}
		}

		std::vector<std::unique_ptr<event::Event>> ClientHandler::events()
		{
			return std::move(events_);
		}

		Client& ClientHandler::getOrCreate(const sockaddr_storage& address)
		{

			auto itClient = std::find_if(
				clients_.begin(),
				clients_.end(),
				[&](const std::unique_ptr<Client>& client) { return equal(client->address(), address); }
				//[&](const std::unique_ptr<Client>& client) { return memcmp(&(client->address()), &address, sizeof(address)) == 0; }
			);
			if (itClient != clients_.end())
			{
				return *(itClient->get());
			}
			else
			{
				clients_.push_back(std::make_unique<Client>(*this, address));
				return *(clients_.back());
			}
		}

		void ClientHandler::onReceived(std::unique_ptr<event::Event>&& event)
		{
			events_.push_back(std::move(event));
		}

		bool ClientHandler::equal(const sockaddr_storage& left, const sockaddr_storage& right)
		{
			struct sockaddr_in* left_in = (struct sockaddr_in*) & left;
			struct sockaddr_in* right_in = (struct sockaddr_in*) & right;
			// TODO IPv6 only !!!!
			unsigned char* left_ip = (unsigned char*)& left_in->sin_addr.s_addr;
			unsigned char* right_ip = (unsigned char*)& right_in->sin_addr.s_addr;
			if (left_ip[0] != right_ip[0] || left_ip[1] != right_ip[1] || left_ip[2] != right_ip[2] || left_ip[3] != right_ip[3]) {
				return false;
			}

			unsigned short left_port = left_in->sin_port;
			unsigned short right_port = right_in->sin_port;
			if (left_port != right_port) {
				return false;
			}

			return true;
		}

	}


}

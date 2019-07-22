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

			sockaddr_in address{ 0 };
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

		void ClientHandler::disconnect(const sockaddr_storage& address)
		{
			auto itClient = std::find_if(
				clients_.cbegin(),
				clients_.cend(),
				[&address](const std::unique_ptr<Client>& client) { return memcmp(&(client->address()), &address, sizeof(address)) == 0; }
			);
			if (itClient != clients_.end())
			{
				clients_.erase(itClient);
			}
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
				sockaddr_storage address{ 0 };
				socklen_t length = sizeof(sockaddr_storage);
				int received = ::recvfrom(socket_, reinterpret_cast<char*>(&datagram), Datagram::DatagramMaxSize, 0, reinterpret_cast<sockaddr*>(&address), &length);
				if (received > 0)
				{
					if (received > Datagram::HeaderSize)
					{
						datagram.bodySize = received - Datagram::HeaderSize;
						auto& client = getOrCreate(address);
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
				clients_.cbegin(),
				clients_.cend(),
				[&address](const std::unique_ptr<Client>& client) { return memcmp(&(client->address()), &address, sizeof(address)) == 0;}
			);
			if (itClient != clients_.cend())
			{
				return *(itClient->get());
			}
			else
			{
				clients_.push_back(std::make_unique<Client>(*this, ++nextClient, address));
				return *(clients_.back());
			}
		}

		void ClientHandler::onReceived(std::unique_ptr<event::Event>&& event)
		{
			events_.push_back(std::move(event));
		}

	}


}

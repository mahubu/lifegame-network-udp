#include <network/Sockets.hpp>
#include <network/event/Connection.hpp>
#include <network/event/Disconnection.hpp>
#include <network/event/Event.hpp>
#include <network/event/Exchange.hpp>
#include <network/ClientHandler.hpp>
#include <network/Client.hpp>
#include <Cell.hpp>
#include <Dish.hpp>
#include <assert.h>
#include <iostream>
#include <vector>
#include <string>

int main()
{
	std::cout << "Server.\n";

	if (!network::startup())
	{
		std::cout << "Socket initialization error: " << network::error::latest();
		return EXIT_FAILURE;
	}

	network::udp::ClientHandler clientHandler;
	if (!clientHandler.startup(11000))
	{
		std::cerr << "Server initialization error: " << network::error::latest();
		return EXIT_FAILURE;
	}

	int i = 0;
	while (true)
	{
		clientHandler.receive();

		std::vector<std::unique_ptr<network::event::Event>> events = clientHandler.events();
		for (auto const& event : events)
		{
			if (event->is<network::event::Connection>())
			{
				std::cout << "Connection. " << std::endl;
			}
			else if (event->is<network::event::Disconnection>())
			{
				std::cout << "Disconnection." << std::endl;

			}
			else if (event->is<network::event::Exchange>())
			{
				auto exchange = event->as<network::event::Exchange>();
				std::string stoc("client->server: " + std::to_string(i++));
				std::vector<network::PacketUnit> msg(stoc.begin(), stoc.end());
				clientHandler.queue(event->client(), std::move(msg));
				std::cout << "Exchange: " << stoc << std::endl;
			}
			else
			{
				std::cerr << "Error: " << network::error::latest() << std::endl;
			}
		}

		clientHandler.send();
	}

	clientHandler.shutdown();

	network::shutdown();

	/*
	std::map<uint64_t, std::unique_ptr<lifegame::Dish>> dishes;

	std::map<uint64_t, std::unique_ptr<network::event::Event>> events;
	while (true)
	{
		events.clear();
		server.process(events);
		for (auto const& event : events)
		{
			uint64_t client = event.first;
			if (event.second->is<network::event::Connection>())
			{
				auto iterator = dishes.find(client);
				if (iterator == dishes.end())
				{
					std::cout << "Dish booked for: " << client << std::endl;
					dishes[client] = nullptr;
				}
				else
				{
					std::cerr << "Dish already booked for: " << client << std::endl;
				}
			}
			else if (event.second->is<network::event::Disconnection>())
			{
				auto iterator = dishes.find(client);
				if (iterator != dishes.end())
				{
					std::cout << "Dish dies for: " << client << std::endl;
					dishes.erase(client);
				}
				else
				{
					std::cerr << "No dish to die for: " << client << std::endl;
				}
			}
			else if (event.second->is<network::event::Exchange>())
			{
				auto iterator = dishes.find(client);
				if (iterator != dishes.end())
				{
					std::vector<uint8_t> cells;

					auto& dish = iterator->second;
					if (dish == nullptr) {
						std::cout << "Dish borns for: " << client << std::endl;
						auto exchange = event.second->as<network::event::Exchange>();
						std::vector<network::PacketUnit> packet = exchange->packet();
						assert(packet.size() == 3);
						auto dish = std::make_unique<lifegame::Dish>(packet.at(0), packet.at(1), packet.at(2));
						dish->cells(cells);
						dishes[client] = std::move(dish);
					}
					else
					{
						std::cout << "Dish lives for: " << client << std::endl;
						dish->live(cells);
					}

					if (cells.size() == 0)
					{
						std::cout << "Dish sleeps for: " << client << std::endl;
					}
					else
					{
						// Sending next step to client.
						if (!server.send(client, cells.data(), static_cast<unsigned int>(cells.size())))
						{
							std::cerr << "Server sending error: " << network::error::latest() << std::endl;
							break;
						}
					}
				}
				else
				{
					std::cerr << "No dish to live for: " << client << std::endl;
				}
			}
		}
	}
	*/

	return EXIT_SUCCESS;
}
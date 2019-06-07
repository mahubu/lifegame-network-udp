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
#include <sstream>
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

	std::map<uint16_t, std::unique_ptr<lifegame::Dish>> dishes;

	while (true)
	{
		clientHandler.receive();

		std::vector<std::unique_ptr<network::event::Event>> events = clientHandler.events();
		for (auto const& event : events)
		{
			auto& client = event->client();
			if (event->is<network::event::Connection>())
			{
				std::cout << "Connection. " << std::endl;
			}
			else if (event->is<network::event::Disconnection>())
			{
				auto iterator = dishes.find(client.uid);
				if (iterator != dishes.end())
				{
					std::cout << "Dish dies for: " << client.uid << std::endl;
					dishes.erase(client.uid);
					clientHandler.disconnect(client.address);
				}
				else
				{
					std::cerr << "No dish to die for: " << client.uid << std::endl;
				}
			}
			else if (event->is<network::event::Exchange>())
			{
				
				std::vector<uint8_t> cells;
				auto iterator = dishes.find(client.uid);
				if (iterator == dishes.end())
				{
					std::cout << "Dish borns for: " << client.uid << std::endl;
					auto exchange = event->as<network::event::Exchange>();
					std::vector<network::PacketUnit> packet = exchange->packet();
					assert(packet.size() == 3);
					auto dish = std::make_unique<lifegame::Dish>(packet.at(0), packet.at(1), packet.at(2));
					dish->cells(cells);
					dishes[client.uid] = std::move(dish);
				}
				else
				{
					std::cout << "Dish lives for: " << client.uid << std::endl;
					iterator->second->live(cells);
				}

				if (cells.size() == 0)
				{
					std::cout << "Dish sleeps for: " << client.uid << std::endl;
				}
				else
				{
					// Sending next step to client.
					clientHandler.queue(client.address, std::move(cells));
				}
			}
			
		}

		clientHandler.send();
	}

	clientHandler.shutdown();

	network::shutdown();

	return EXIT_SUCCESS;
}
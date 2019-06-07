#include <network/Client.hpp>
#include <network/Sockets.hpp>
#include <network/event/Connection.hpp>
#include <network/event/Disconnection.hpp>
#include <network/event/Event.hpp>
#include <network/event/Exchange.hpp>
#include <network/ClientHandler.hpp>
#include <network/Client.hpp>
#include <Cell.hpp>
#include <Dish.hpp>
#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char* argv[])
{
	std::cout << "Client.\n";

	if (argc <= 4)
	{
		std::cerr << "Not enough arguments: " << argc - 1 << std::endl;
		return EXIT_FAILURE;
	}

	int port = std::stoi(argv[1], nullptr);
	std::cout << "Port: " << port << std::endl;

	for (int i = 2; i < argc; i++)
	{
		int value = std::stoi(argv[i], nullptr);
		std::cout << "Argument " << i << ": " << value << std::endl;
		if (value <= (std::numeric_limits<network::PacketUnit>::min)() || value > (std::numeric_limits<network::PacketUnit>::max)())
		{
			std::cerr << "Argument not in ]0, 255] range: " << value << std::endl;
			return EXIT_FAILURE;
		}
	}

	if (!network::startup())
	{
		std::cout << "Socket initialization error: " << network::error::latest();
		return EXIT_FAILURE;
	}

	network::udp::ClientHandler clientHandler;
	if (!clientHandler.startup(port))
	{
		std::cerr << "Server initialization error: " << network::error::latest();
		return EXIT_FAILURE;
	}

	int rows = std::stoi(argv[2], nullptr);
	int columns = std::stoi(argv[3], nullptr);
	int ratio = std::stoi(argv[4], nullptr);
	lifegame::Dish dish(rows, columns, ratio);

	// Initializing exchange by sending parameters to server.
	std::vector<network::PacketUnit> parameters;
	parameters.push_back(rows);
	parameters.push_back(columns);
	parameters.push_back(ratio);

	sockaddr_in address{ 0 };
	inet_pton(AF_INET, "127.0.0.1", &address.sin_addr.s_addr);
	address.sin_port = htons(11000);
	address.sin_family = AF_INET;
	sockaddr_storage storage{ 0 };
	memcpy(&storage, &address, sizeof(address));
	clientHandler.queue(storage, std::move(parameters));

	int i = 0;
	while (true)
	{
		clientHandler.receive();

		std::vector<std::unique_ptr<network::event::Event>> events = clientHandler.events();
		for (auto const& event : events)
		{
			auto& client = event->client();
			if (event->is<network::event::Connection>())
			{
				std::cout << "Client connected. " << std::endl;
			}
			else if (event->is<network::event::Disconnection>())
			{
				std::cout << "Client disconnected." << std::endl;
				clientHandler.disconnect(client.address);
			}
			else if (event->is<network::event::Exchange>())
			{
				std::cout << "Client exchanging..." << std::endl;
				auto exchange = event->as<network::event::Exchange>();
				std::vector<network::PacketUnit> cells = exchange->packet();
				dish.modify(cells);

				// TODO display dish into a grid.
				/*
				for (auto cell : cells) {
					std::cout << (int)cell << "_";
				}
				std::cout << "/" << std::endl;
				*/

				// Asking for advancing into next step.
				std::vector<network::PacketUnit> step{ 0 };
				clientHandler.queue(client.address, std::move(step));
			}
		}

		clientHandler.send();
	}

	clientHandler.shutdown();

	network::shutdown();

	return EXIT_SUCCESS;
}
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
#include <string>

int main(int argc, char* argv[])
{
	std::cout << "Client.\n";


	if (!network::startup())
	{
		std::cout << "Socket initialization error: " << network::error::latest();
		return EXIT_FAILURE;
	}

	network::udp::ClientHandler clientHandler;
	if (!clientHandler.startup(10000))
	{
		std::cerr << "Server initialization error: " << network::error::latest();
		return EXIT_FAILURE;
	}

	// Initializing exchange by sending first datagram to server.
	std::string ctos("client->server");
	std::vector<network::PacketUnit> msg(ctos.begin(), ctos.end());
	sockaddr_in address;
	inet_pton(AF_INET, "127.0.0.1", &address.sin_addr.s_addr);
	address.sin_port = htons(11000);
	address.sin_family = AF_INET;
	clientHandler.queue(reinterpret_cast<sockaddr_storage&>(address), std::move(msg));

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
				std::cout << "Disconnection. " << std::endl;

			}
			else if (event->is<network::event::Exchange>())
			{
				auto exchange = event->as<network::event::Exchange>();
				std::string stoc("server->client: " + std::to_string(i++));
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
	if (argc <= 3)
	{
		std::cerr << "Not enough arguments: " << argc - 1 << std::endl;
		return EXIT_FAILURE;
	}

	for (int i = 1; i < argc; i++)
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
		std::cerr << "Socket initialization error: " << network::error::latest() << std::endl;
		return EXIT_FAILURE;
	}

	network::tcp::Client client;
	bool connected = client.connect("127.0.0.1", 11000);
	if (!connected)
	{
		std::cerr << "Client connection error: " << network::error::latest() << std::endl;
		return EXIT_FAILURE;
	}
	else
	{
		int rows = std::stoi(argv[1], nullptr);
		int columns = std::stoi(argv[2], nullptr);
		int ratio = std::stoi(argv[3], nullptr);
		lifegame::Dish dish(rows, columns, ratio);

		while (true)
		{
			// TODO time out when 'connecting' never become 'connected' ???
			while (auto const& event = client.process())
			{
				if (event->is<network::event::Connection>())
				{
					auto connection = event->as<network::event::Connection>();
					if (connection->state() == network::event::Connection::State::Successfull)
					{
						std::cout << "Client connected." << std::endl;
						std::vector<network::PacketUnit> parameters;
						parameters.push_back(rows);
						parameters.push_back(columns);
						parameters.push_back(ratio);
						if (!client.send(parameters.data(), static_cast<unsigned int>(parameters.size())))
						{
							std::cerr << "Client sending error: " << network::error::latest() << std::endl;
							break;
						}
					}
					else
					{
						std::cerr << "Client connection error: " << static_cast<int>(connection->state()) << std::endl;
						break;
					}
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
					*

					// Asking for advancing into next step.
					network::PacketUnit step(0);
					if (!client.send(&step, sizeof(step)))
					{
						std::cerr << "Client sending error: " << network::error::latest() << std::endl;
						break;
					}
				}
				else if (event->is<network::event::Disconnection>())
				{
					auto disconnection = event->as<network::event::Disconnection>();
					std::cout << "Client disconnected: " << static_cast<int>(disconnection->reason()) << std::endl;
					break;
				}
			}
		}
	}

	network::shutdown();
	*/

	return EXIT_SUCCESS;
}
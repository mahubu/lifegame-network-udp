#pragma once

#include <network/Sockets.hpp>
#include <network/event/Event.hpp>
#include <vector>

namespace network
{
	namespace event
	{
		/*
		* @brief An event bearing a network packet.
		*/
		class Exchange : public Event
		{
		public:
			static const Event::Type STATIC_TYPE = Event::Type::Exchange;
			Exchange(const std::vector<PacketUnit>& packet, const sockaddr_storage& client) : Event(Type::Exchange, client), packet_(packet)
			{
			}
			std::vector<PacketUnit> packet() const;

		private:
			std::vector<PacketUnit> packet_;

		};
	}

}
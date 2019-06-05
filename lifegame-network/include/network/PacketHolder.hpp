#pragma once

#include <network/Packet.hpp>
#include <set>

namespace network
{
	namespace udp
	{
		/*
		* @brief An holder for packets.
		*/
		class PacketHolder
		{
		public:
			PacketHolder(Packet& packet) : packet_(packet) {};

			/*
			* @return the held packet.
			*/
			Packet& packet() { return packet_; }

			/*
			* @return whether (or not) the packet has been sent.
			*/
			bool sent() const { return sent_; }

			/*
			* @brief Set whether the packet has been sent (or not).
			*
			* @param sent whether (or not) the packet has been sent.
			*/
			void sent(bool sent) { sent_ = sent; }

			/*
			* @brief Set the packet as sent & the datagram with which it has been sent.
			*
			* @param datagram the datagram identifier.
			*/
			void sentWith(Datagram::IdType datagram) { sent_ = true; datagrams_.insert(datagram); }

			/*
			* @param datagram a datagram identifier.
			*
			* @return whether (or not) the packet has been sent with this datagram.
			*/
			bool isSentWith(Datagram::IdType datagram) const { return datagrams_.find(datagram) != datagrams_.cend(); }

		private:
			Packet packet_;
			bool sent_{ false };
			std::set<Datagram::IdType> datagrams_;

		};

	}
}
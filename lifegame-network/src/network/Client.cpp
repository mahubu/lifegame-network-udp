#include <network/Sockets.hpp>
#include <network/Client.hpp>
#include <network/ClientHandler.hpp>
#include <network/event/Exchange.hpp>
#include <assert.h>
#include <memory>
#include <vector>

#include <iostream>
#include <string>

namespace network
{
	namespace udp
	{
		Client::Client(ClientHandler& handler, const sockaddr_storage& address) : clientHandler_(handler)
		{
			std::memcpy(&address_, &address, sizeof(address));
			//std::cout << "Client address: " << &address_ << " vs " << &address << std::endl;
		}

		bool Client::fill(Datagram& datagram)
		{
			datagram.bodySize = sendingHandler_.serialize(datagram.body.data(), Datagram::BodyMaxSize);
			if (datagram.bodySize > 0)
			{
				datagram.header.id = htons(nextDatagramIdToSend_++);
				datagram.header.newestAck = htons(receptionAck_.lastAck());
				datagram.header.previousAcks = receptionAck_.previousAcks();
				//++nextDatagramIdToSend_;
				return true;
			}
			return false;
		}

		void Client::queue(std::vector<PacketUnit>&& packet)
		{
			sendingHandler_.queue(std::move(packet));
		}

		void Client::send()
		{
			while (true)
			{
				Datagram datagram;
				if (!fill(datagram))
				{
					break;
				}
				::sendto(clientHandler_.socket_, reinterpret_cast<const char*>(&datagram), static_cast<int>(datagram.size()), 0, reinterpret_cast<const sockaddr*>(&address_), sizeof(address_));
			}
		}

		void Client::onReceived(Datagram&& datagram)
		{
			assert(datagram.bodySize > 0);

			// Update the received acks tracking.
			const auto receivedId = ntohs(datagram.header.id);
			receptionAck_.update(receivedId, 0, true);

			// Update the send acks tracking.
			const auto sendId = ntohs(datagram.header.newestAck);
			sendingAck_.update(sendId, datagram.header.previousAcks, true);

			// Ignore duplications.
			if (!receptionAck_.isNewlyAcked(receivedId))
			{
				std::cout << "Already acked: " << datagram.header.id << std::endl;
				return;
			}

			// Handle sended packets that have NOT been acked.
			const auto datagramsSentLost = sendingAck_.losses();
			for (const auto sendLoss : datagramsSentLost)
			{
				// TODO
			}

			// Handle sended packets that have been acked.
			const auto datagramsSentAcked = sendingAck_.newAcks();
			for (const auto sendAcked : datagramsSentAcked)
			{
				// TODO
			}

			// Handle received packets that have been loss.
			const auto lostDatagrams = receptionAck_.losses();
			for (const auto receivedLostDatagram : lostDatagrams)
			{
				// TODO
			}

			// Handle received packets that have NOT been loss.
			onPacketReceived(datagram.body.data(), datagram.bodySize);
		}

		void Client::onPacketReceived(const uint8_t* rawPacket, const size_t rawPacketSize)
		{
			receptionHandler_.onReceived(rawPacket, rawPacketSize);
			auto packets = receptionHandler_.unserialize();
			for (auto&& packet : packets)
			{
				//memcpy(&(event->from), &address_, sizeof(address_));
				clientHandler_.onReceived(std::make_unique<event::Exchange>(std::move(packet), address_));
			}

		}

	}
}
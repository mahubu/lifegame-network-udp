#include <network/Sockets.hpp>
#include <network/Client.hpp>
#include <network/ClientHandler.hpp>
#include <network/event/Disconnection.hpp>
#include <network/event/Event.hpp>
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
		Client::Client(ClientHandler& handler, const uint16_t uid, const sockaddr_storage& address) : clientHandler_(handler)
		{
			client_.uid = uid;
			std::memcpy(&client_.address, &address, sizeof(address));
		}

		bool Client::fill(Datagram& datagram)
		{
			datagram.bodySize = sendingHandler_.serialize(nextDatagramIdToSend_, datagram.body.data(), Datagram::BodyMaxSize);
			if (datagram.bodySize > 0)
			{
				datagram.header.id = htons(nextDatagramIdToSend_++);
				datagram.header.newestAck = htons(receptionAck_.lastAck());
				datagram.header.previousAcks = receptionAck_.previousAcks();
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
			if (sendingHandler_.idle()) {
				clientHandler_.onReceived(std::make_unique<event::Disconnection>(event::Disconnection::Reason::Lost, client_));
				return;
			}

			while (true)
			{
				Datagram datagram;
				if (!fill(datagram))
				{
					break;
				}
				::sendto(clientHandler_.socket_, reinterpret_cast<const char*>(&datagram), static_cast<int>(datagram.size()), 0, reinterpret_cast<const sockaddr*>(&client_.address), sizeof(client_.address));
			}
		}

		void Client::onReceived(Datagram&& datagram)
		{
			if (receptionHandler_.idle()) {
				clientHandler_.onReceived(std::make_unique<event::Disconnection>(event::Disconnection::Reason::Lost, client_));
				return;
			}

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
			const auto sentLosses = sendingAck_.losses();
			for (const auto loss : sentLosses)
			{
				sendingHandler_.onLost(loss);
			}

			// Handle sended packets that have been acked.
			const auto sentAcks = sendingAck_.newAcks();
			for (const auto ack : sentAcks)
			{
				sendingHandler_.onAcked(ack);
			}

			// Handle received packets that have NOT been loss.
			onPacketReceived(datagram.body.data(), datagram.bodySize);
		}

		void Client::onPacketReceived(const PacketUnit* rawPacket, const size_t rawPacketSize)
		{
			receptionHandler_.onReceived(rawPacket, rawPacketSize);

			std::vector<std::vector<PacketUnit>> packets;
			receptionHandler_.unserialize(packets);
			for (auto&& packet : packets)
			{
				clientHandler_.onReceived(std::make_unique<event::Exchange>(std::move(packet), client_));
			}
		}

	}
}
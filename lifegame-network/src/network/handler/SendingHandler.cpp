#include <network/handler/SendingHandler.hpp>
#include <network/helper/BitHelper.hpp>
#include <assert.h>
#include <numeric>

namespace network
{
	namespace handler
	{
		void SendingHandler::queue(std::vector<PacketUnit>&& rawPacket)
		{
			assert(rawPacket.size() <= udp::Packet::MaxMessageSize);
			if (rawPacket.size() > udp::Packet::BodyMaxSize)
			{
				size_t queuedSize = 0;
				while (queuedSize < rawPacket.size())
				{
					const auto partialSize = std::min(udp::Packet::BodyMaxSize, static_cast<uint16_t>(rawPacket.size() - queuedSize));
					udp::Packet packet;
					packet.header.id = nextId_++;
					packet.header.type = ((queuedSize == 0) ? udp::Packet::Type::PartialFirst : udp::Packet::Type::PartialInBetween);
					packet.header.size = partialSize;
					memcpy(packet.body.data(), rawPacket.data() + queuedSize, partialSize);
					udp::PacketHolder holder(packet);
					queue_.push_back(holder);
					queuedSize += partialSize;
				}
				// Get last packet from the queue to alter its type.
				queue_.back().packet().header.type = udp::Packet::Type::PartialLast;
				assert(queuedSize == rawPacket.size());
			}
			else
			{
				udp::Packet packet;
				packet.header.id = nextId_++;
				packet.header.type = udp::Packet::Type::Complete;
				packet.header.size = static_cast<uint16_t>(rawPacket.size());
				memcpy(packet.body.data(), rawPacket.data(), rawPacket.size());
				udp::PacketHolder holder(packet);
				queue_.push_back(holder);
			}
		}

		size_t SendingHandler::serialize(udp::Datagram::IdType datagram, PacketUnit* rawPacket, const size_t rawPacketSize)
		{
			size_t serializedSize = 0;
			for (auto& holder : queue_)
			{

				if (holder.sent())
				{
					continue;
				}

				const auto& packet = holder.packet();

				if (serializedSize + packet.size() > rawPacketSize)
				{
					// Raw packet is full.
					break;
				}

				// Allows packet to be sent according it's not too recent compared to the expected one.
				if (/*helper::isNewer(packet.id(), nextAllowed_) &&*/ helper::difference(packet.id(), nextExpected_) >= udp::Packet::MaxPacketQueueSize)
				{
					break;
				}

				// Copy packet struct into the raw packet array. 
				memcpy(rawPacket, &packet, sizeof(packet));
				//memcpy(rawPacket, packet.serialize(), packet.size());
				rawPacket += packet.size();
				serializedSize += packet.size();

				holder.sentWith(datagram);
			}
			return serializedSize;
		}

		void SendingHandler::onAcked(udp::Datagram::IdType datagram)
		{
			if (queue_.empty())
			{
				return;
			}

			// Remove acked packets.
			queue_.erase(
				std::remove_if(
					queue_.begin(),
					queue_.end(),
					[&](const udp::PacketHolder& holder) { return holder.isSentWith(datagram); }
				),
				queue_.cend()
			);

			// Next packet expected to be sent is the latest one.
			if (queue_.empty())
			{
				nextExpected_ = nextId_;
			}
			// Next packet expected to be sent is the one at start of the queue. 
			else if (helper::isNewer(queue_.front().packet().id(), nextExpected_))
			{
				nextExpected_ = queue_.front().packet().id();
			}
		}

		void SendingHandler::onLost(udp::Datagram::IdType datagram)
		{
			for (auto& holder : queue_)
			{
				if (holder.isSentWith(datagram))
				{
					holder.sent(false);
				}
			}
		}

	}
}
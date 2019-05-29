#include <network/handler/SendingHandler.hpp>
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
					packet.header.id = newId_++;
					packet.header.type = ((queuedSize == 0) ? udp::Packet::Type::PartialFirst : udp::Packet::Type::PartialInBetween);
					packet.header.size = partialSize;
					memcpy(packet.body.data(), rawPacket.data() + queuedSize, partialSize);
					queue_.push_back(packet);
					queuedSize += partialSize;
				}
				// Get last packet from the queue to alter its type.
				queue_.back().header.type = udp::Packet::Type::PartialLast;
				assert(queuedSize == rawPacket.size());
			}
			else
			{
				udp::Packet packet;
				packet.header.id = newId_++;
				packet.header.type = udp::Packet::Type::Complete;
				packet.header.size = static_cast<uint16_t>(rawPacket.size());
				memcpy(packet.body.data(), rawPacket.data(), rawPacket.size());
				queue_.push_back(packet);
			}
		}

		size_t SendingHandler::serialize(PacketUnit* rawPacket, const size_t rawPacketSize)
		{
			size_t serializedSize = 0;
			for (auto packetIt = queue_.begin(); packetIt != queue_.end();)
			{
				const auto& packet = *packetIt;
				if (serializedSize + packet.size() > rawPacketSize)
				{
					// Raw packet is full.
					break;
				}

				// Copy packet struct into the raw packet array. 
				memcpy(rawPacket, packet.serialize(), packet.size());
				rawPacket += packet.size();
				serializedSize += packet.size();

				// Wipe processed packet from queue.
				packetIt = queue_.erase(packetIt);
			}
			return serializedSize;
		}

	}
}
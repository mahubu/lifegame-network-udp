#include <network/handler/ReceptionHandler.hpp>
#include <network/helper/BitHelper.hpp>
#include <network/event/Disconnection.hpp>
#include <network/event/Exchange.hpp>
#include <assert.h>

namespace network
{
	namespace handler
	{
		void ReceptionHandler::onReceived(const PacketUnit* rawPacket, const size_t rawPacketSize)
		{
			// Extract individual packets from the raw packet.
			size_t processedSize = 0;
			while (processedSize < rawPacketSize)
			{
				// Get raw packet as a packet struct.
				udp::Packet packet;
				memcpy(&packet, rawPacket, rawPacketSize);
				//const udp::Packet* packet = udp::unserialize(rawPacket);
				if (processedSize + packet.size() > rawPacketSize || packet.bodySize() > udp::Packet::BodyMaxSize)
				{
					// Malformed packet.
					return;
				}
				onPacketReceived(&packet);
				processedSize += packet.size();
				rawPacket += packet.size();
			}
		}

		void ReceptionHandler::onPacketReceived(const udp::Packet* packet)
		{
			if (!helper::isNewer(packet->id(), lastProcessedId_))
			{
				// Packet too old.
				return;
			}

			// Find index for the packet, the queue must remain ordered.
			if (queue_.empty() || helper::isNewer(packet->id(), queue_.back().id()))
			{
				queue_.push_back(*packet);
			}
			else
			{
				// Find the first iterator with an id equals to or newer than our packet, the packet must be inserted before that one.
				auto index = std::find_if(queue_.cbegin(), queue_.cend(), [&packet](const udp::Packet& p) { return p.id() == packet->id() || helper::isNewer(p.id(), packet->id()); });
				// Make sure the packet is not inserted multiple times.
				if (index->id() != packet->id())
				{
					queue_.insert(index, *packet);
				}
			}
		}

		void ReceptionHandler::unserialize(std::vector<std::vector<PacketUnit>>& packets)
		{
			auto packetIt = queue_.begin();
			auto endIt = queue_.end();
			std::vector<udp::Packet>::iterator latestPacket;
			while (packetIt != endIt)
			{
				// Complete packet.
				if (packetIt->type() == udp::Packet::Type::Complete)
				{
					std::vector<PacketUnit> packet(packetIt->body.data(), packetIt->body.data() + packetIt->bodySize());
					packets.push_back(std::move(packet));
					latestPacket = packetIt;
					++packetIt;
				}
				// Partial packet.
				else if (packetIt->type() == udp::Packet::Type::PartialFirst)
				{
					// TODO don't use lambda expression ?
					std::vector<PacketUnit> packet = [&]()
					{
						// Initializing packet with first part.
						std::vector<PacketUnit> packet(packetIt->body.data(), packetIt->body.data() + packetIt->bodySize());
						auto expectedId = packetIt->id();

						// Iterating over all part of the packet.
						++packetIt;
						++expectedId;
						while (packetIt != endIt && packetIt->id() == expectedId)
						{
							// Intermediate packet part.
							if (packetIt->type() == udp::Packet::Type::PartialInBetween)
							{
								packet.insert(packet.end(), packetIt->body.data(), packetIt->body.data() + packetIt->bodySize());
								++packetIt;
								++expectedId;
							}
							// Last packet part.
							else if (packetIt->type() == udp::Packet::Type::PartialLast)
							{
								packet.insert(packet.end(), packetIt->body.data(), packetIt->body.data() + packetIt->bodySize());
								return packet;
							}
							// Unknown / Malformed packet.
							else
							{
								packet.clear();
								return packet;
							}
						}
						// Incomplete packet.
						packet.clear();
						return packet;
					}();

					if (!packet.empty())
					{
						// Inserting formed packet.
						packets.push_back(std::move(packet));
						latestPacket = packetIt;
						++packetIt;
					}
					else 
					{
						// Unknown / Malformed / incomplete packet
						break;
					}
				}
				// Unknown / malformed / incomplete packet
				else
				{
					break;
				}
			}

			if (!packets.empty())
			{
				lastProcessedId_ = latestPacket->id();
				lastProcessedTime_ = std::chrono::system_clock::now();
				// Erase all processed packets.
				queue_.erase(queue_.begin(), std::next(latestPacket));
			}
		}

	}
}

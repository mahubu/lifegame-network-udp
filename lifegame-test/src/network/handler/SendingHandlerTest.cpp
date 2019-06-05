#include <network/handler/SendingHandlerTest.hpp>
#include <network/handler/SendingHandler.hpp>
#include <chrono>
#include <random>

namespace network
{
	namespace handler
	{
		void SendingHandlerTest::test()
		{
			constructTest();
			queueCompletePacketTest();
			serializeCompletePacketTest();
			onAckedCompletePacketTest();
			onLostCompletePacketTest();
			onResendCompletePacketTest();
			queuePartialPacketTest();
			serializePartialPacketTest();
			onAckedPartialPacketTest();
			onLostPartialPacketTest();
			onResendPartialPacketTest();
		}


		void SendingHandlerTest::constructTest()
		{
			SendingHandler handler;
			ASSERT_TRUE(handler.size() == 0);
			ASSERT_TRUE(handler.nextId() == 0);
			ASSERT_TRUE(handler.nextExpected() == 0);
		}

		void SendingHandlerTest::queueCompletePacketTest()
		{
			SendingHandler handler;

			const std::string str("Hello!");
			std::vector<uint8_t> msg(str.cbegin(), str.cend());
			handler.queue(std::move(msg));
			ASSERT_TRUE(handler.size() == 1);
			ASSERT_TRUE(handler.nextId() == 1);
			ASSERT_TRUE(handler.nextExpected() == 0);
		}

		void SendingHandlerTest::serializeCompletePacketTest()
		{
			SendingHandler handler;
			udp::Datagram::IdType datagramId = 0;
			const std::string str("Hello!");
			std::vector<uint8_t> msg(str.cbegin(), str.cend());
			handler.queue(std::move(msg));

			std::array<uint8_t, udp::Packet::PacketMaxSize> buffer;
			const size_t serialized = handler.serialize(datagramId++, buffer.data(), buffer.size());
			udp::Packet packet;
			memcpy(&packet, &buffer, sizeof(buffer));
			ASSERT_TRUE(packet.bodySize() == str.size());
			ASSERT_TRUE(memcmp(packet.bodyData(), str.data(), packet.bodySize()) == 0);
			ASSERT_TRUE(handler.size() == 1);
			ASSERT_TRUE(handler.nextId() == 1);
			ASSERT_TRUE(handler.nextExpected() == 0);
		}

		void SendingHandlerTest::onAckedCompletePacketTest()
		{
			SendingHandler handler;
			udp::Datagram::IdType datagramId = 0;
			const std::string str("Hello!");
			std::vector<uint8_t> msg(str.cbegin(), str.cend());
			handler.queue(std::move(msg));
			std::array<uint8_t, udp::Packet::PacketMaxSize> buffer;
			const size_t serialized = handler.serialize(datagramId++, buffer.data(), buffer.size());

			handler.onAcked(0);
			ASSERT_TRUE(handler.size() == 0);
			ASSERT_TRUE(handler.nextId() == 1);
			ASSERT_TRUE(handler.nextExpected() == 1);
		}

		void SendingHandlerTest::onLostCompletePacketTest()
		{
			SendingHandler handler;
			udp::Datagram::IdType datagramId = 0;
			const std::string str("Hello?");
			std::vector<uint8_t> msg(str.cbegin(), str.cend());
			handler.queue(std::move(msg));
			std::array<uint8_t, udp::Packet::PacketMaxSize> buffer;
			const size_t serialized = handler.serialize(datagramId++, buffer.data(), buffer.size());

			handler.onLost(0);
			ASSERT_TRUE(handler.size() == 1);
			ASSERT_TRUE(handler.nextId() == 1);
			ASSERT_TRUE(handler.nextExpected() == 0);
		}

		void SendingHandlerTest::onResendCompletePacketTest()
		{
			SendingHandler handler;
			udp::Datagram::IdType datagramId = 0;
			const std::string str("Hello?");
			std::vector<uint8_t> msg(str.cbegin(), str.cend());
			handler.queue(std::move(msg));
			std::array<uint8_t, udp::Packet::PacketMaxSize> buffer;
			const size_t serialized = handler.serialize(datagramId++, buffer.data(), buffer.size());
			handler.onLost(0);

			const size_t reserialized = handler.serialize(datagramId++, buffer.data(), buffer.size());
			handler.onAcked(0);
			ASSERT_TRUE(handler.size() == 0);
			ASSERT_TRUE(handler.nextId() == 1);
			ASSERT_TRUE(handler.nextExpected() == 1);
		}

		void SendingHandlerTest::queuePartialPacketTest()
		{
			SendingHandler handler;
			std::vector<uint8_t> sentMsg(udp::Packet::BodyMaxSize * 3, 0);
			const unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
			std::default_random_engine generator(seed);
			std::uniform_int_distribution<unsigned int> distribution(0, 100);
			for (uint8_t& character : sentMsg)
			{
				character = distribution(generator);
			}
			const auto originalMsg = sentMsg;

			handler.queue(std::move(sentMsg));
			ASSERT_TRUE(handler.size() == 3);
			ASSERT_TRUE(handler.nextId() == 3);
			ASSERT_TRUE(handler.nextExpected() == 0);
		}


		void SendingHandlerTest::serializePartialPacketTest()
		{
			SendingHandler handler;
			std::vector<uint8_t> sentMsg(udp::Packet::BodyMaxSize * 3, 0);
			const unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
			std::default_random_engine generator(seed);
			std::uniform_int_distribution<unsigned int> distribution(0, 100);
			for (uint8_t& character : sentMsg)
			{
				character = distribution(generator);
			}
			const auto originalMsg = sentMsg;
			handler.queue(std::move(sentMsg));

			size_t totalSize = 0;
			udp::Datagram::IdType datagramId = 0;
			for (;;)
			{
				std::array<uint8_t, udp::Packet::PacketMaxSize> buffer;
				const size_t serialized = handler.serialize(datagramId++, buffer.data(), buffer.size());
				if (serialized == 0)
				{
					break;
				}
				udp::Packet packet;
				memcpy(&packet, &buffer, sizeof(buffer));
				ASSERT_TRUE(memcmp(packet.bodyData(), originalMsg.data() + totalSize, packet.bodySize()) == 0);
				totalSize += serialized - udp::Packet::HeaderSize;
			}
			ASSERT_TRUE(totalSize == originalMsg.size());
			ASSERT_TRUE(handler.size() == 3);
			ASSERT_TRUE(handler.nextId() == 3);
			ASSERT_TRUE(handler.nextExpected() == 0);
		}

		void SendingHandlerTest::onAckedPartialPacketTest()
		{
			SendingHandler handler;
			std::vector<uint8_t> sentMsg(udp::Packet::BodyMaxSize * 3, 0);
			const unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
			std::default_random_engine generator(seed);
			std::uniform_int_distribution<unsigned int> distribution(0, 100);
			for (uint8_t& character : sentMsg)
			{
				character = distribution(generator);
			}
			const auto originalMsg = sentMsg;
			handler.queue(std::move(sentMsg));

			udp::Datagram::IdType datagramId = 0;
			while (true)
			{
				std::array<uint8_t, udp::Packet::PacketMaxSize> buffer;
				const size_t serialized = handler.serialize(datagramId++, buffer.data(), buffer.size());
				if (serialized == 0)
				{
					break;
				}
			}

			const size_t packets = handler.size();
			size_t acked = 0;
			for (unsigned int i = 0; i < packets; ++i)
			{
				handler.onAcked(i);
				++acked;
				ASSERT_TRUE(handler.size() == 3 - acked);
			}
			ASSERT_TRUE(handler.size() == 0);
			ASSERT_TRUE(handler.nextId() == 3);
			ASSERT_TRUE(handler.nextExpected() == 3);
		}

		void SendingHandlerTest::onLostPartialPacketTest()
		{
			SendingHandler handler;
			std::vector<uint8_t> sentMsg(udp::Packet::BodyMaxSize * 3, 0);
			const unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
			std::default_random_engine generator(seed);
			std::uniform_int_distribution<unsigned int> distribution(0, 100);
			for (uint8_t& character : sentMsg)
			{
				character = distribution(generator);
			}
			const auto originalMsg = sentMsg;
			handler.queue(std::move(sentMsg));

			udp::Datagram::IdType datagramId = 0;
			while (true)
			{
				std::array<uint8_t, udp::Packet::PacketMaxSize> buffer;
				const size_t serialized = handler.serialize(datagramId++, buffer.data(), buffer.size());
				if (serialized == 0)
				{
					break;
				}
			}

			handler.onAcked(0);
			handler.onLost(1);
			handler.onAcked(2);
			ASSERT_TRUE(handler.size() == 1);
			ASSERT_TRUE(handler.nextId() == 3);
			ASSERT_TRUE(handler.nextExpected() == 1
			);
		}

		void SendingHandlerTest::onResendPartialPacketTest()
		{
			SendingHandler handler;
			std::vector<uint8_t> sentMsg(udp::Packet::BodyMaxSize * 3, 0);
			const unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
			std::default_random_engine generator(seed);
			std::uniform_int_distribution<unsigned int> distribution(0, 100);
			for (uint8_t& character : sentMsg)
			{
				character = distribution(generator);
			}
			const auto originalMsg = sentMsg;
			handler.queue(std::move(sentMsg));

			udp::Datagram::IdType datagramId = 0;
			while (true)
			{
				std::array<uint8_t, udp::Packet::PacketMaxSize> buffer;
				const size_t serialized = handler.serialize(datagramId++, buffer.data(), buffer.size());
				if (serialized == 0)
				{
					break;
				}
			}

			handler.onAcked(0);
			handler.onLost(1);
			handler.onAcked(2);

			std::array<uint8_t, udp::Packet::PacketMaxSize> buffer;
			const size_t reserialized = handler.serialize(datagramId++, buffer.data(), buffer.size());
			handler.onAcked(1);
			ASSERT_TRUE(handler.size() == 0);
			ASSERT_TRUE(handler.nextId() == 3);
			ASSERT_TRUE(handler.nextExpected() == 3);
		}

	}
}
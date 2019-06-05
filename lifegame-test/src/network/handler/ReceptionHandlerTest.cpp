#include <network/handler/ReceptionHandlerTest.hpp>
#include <network/handler/ReceptionHandler.hpp>
#include <network/handler/SendingHandler.hpp>
#include <chrono>
#include <random>

namespace network
{
	namespace handler
	{
		void ReceptionHandlerTest::test()
		{		
			constructTest();
			receiveCompletePacketTest();
			receivePartialPacketTest();
			receiveIncompletePartialPacketTest();
			receiveLostPartialPacketTest();
			receiveResendPartialPacketTest();
		}

		void ReceptionHandlerTest::constructTest()
		{
			ReceptionHandler reception;
			ASSERT_TRUE(reception.size() == 0);
			ASSERT_TRUE(reception.lastProcessed() == std::numeric_limits<udp::Packet::IdType>::max());
		}

		void ReceptionHandlerTest::receiveCompletePacketTest()
		{
			SendingHandler sending;
			ReceptionHandler reception;
			udp::Datagram::IdType datagramId = 0;

			//!< Receive packet 0 & 1
			const std::string str0("Hello!");
			std::vector<PacketUnit> msg0(str0.cbegin(), str0.cend());
			sending.queue(std::move(msg0));
			std::array<PacketUnit, udp::Packet::PacketMaxSize> buffer0;
			sending.serialize(datagramId++, buffer0.data(), buffer0.size());
			reception.onReceived(buffer0.data(), buffer0.size());

			const std::string str1("Hi!");
			std::vector<PacketUnit> msg1(str1.cbegin(), str1.cend());
			sending.queue(std::move(msg1));
			std::array<PacketUnit, udp::Packet::PacketMaxSize> buffer1;
			sending.serialize(datagramId++, buffer1.data(), buffer1.size());
			reception.onReceived(buffer1.data(), buffer1.size());

			const std::vector<std::vector<PacketUnit>> packets = reception.unserialize();
			ASSERT_TRUE(reception.size() == 0);
			ASSERT_TRUE(reception.lastProcessed() == 1);
			ASSERT_TRUE(packets.size() == 2);
			ASSERT_TRUE(packets[0].size() == msg0.size());
			ASSERT_TRUE(packets[0] == msg0);
			ASSERT_TRUE(packets[1].size() == msg1.size());
			ASSERT_TRUE(packets[1] == msg1);
		}

		void ReceptionHandlerTest::receivePartialPacketTest()
		{
			SendingHandler sending;
			ReceptionHandler reception;
			udp::Datagram::IdType datagramId = 0;

			std::vector<uint8_t> sentMsg(udp::Packet::BodyMaxSize * 3, 0);
			const unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
			std::default_random_engine generator(seed);
			std::uniform_int_distribution<unsigned int> distribution(0, 100);
			for (uint8_t& character : sentMsg)
			{
				character = distribution(generator);
			}
			const auto originalMsg = sentMsg;

			sending.queue(std::move(sentMsg));

			// First packet
			std::array<PacketUnit, udp::Packet::PacketMaxSize> buffer0;
			sending.serialize(datagramId++, buffer0.data(), buffer0.size());
			reception.onReceived(buffer0.data(), buffer0.size());
			sending.onAcked(0);

			// In-between packet
			std::array<PacketUnit, udp::Packet::PacketMaxSize> buffer1;
			sending.serialize(datagramId++, buffer1.data(), buffer1.size());
			reception.onReceived(buffer1.data(), buffer1.size());
			sending.onAcked(1);

			// Last packet
			std::array<PacketUnit, udp::Packet::PacketMaxSize> buffer2;
			sending.serialize(datagramId++, buffer2.data(), buffer2.size());
			reception.onReceived(buffer2.data(), buffer2.size());
			sending.onAcked(2);

			// Empty packet
			std::array<PacketUnit, udp::Packet::PacketMaxSize> buffer3;
			const size_t serialized3 = sending.serialize(datagramId++, buffer3.data(), buffer3.size());
			ASSERT_TRUE(serialized3 == 0);

			const std::vector<std::vector<PacketUnit>> packets = reception.unserialize();
			ASSERT_TRUE(reception.size() == 0);
			ASSERT_TRUE(reception.lastProcessed() == 2);
			ASSERT_TRUE(packets.size() == 1);
			ASSERT_TRUE(packets[0].size() == originalMsg.size());
			ASSERT_TRUE(packets[0] == originalMsg);
		}

		void ReceptionHandlerTest::receiveIncompletePartialPacketTest()
		{
			SendingHandler sending;
			ReceptionHandler reception;
			udp::Datagram::IdType datagramId = 0;

			std::vector<uint8_t> sentMsg(udp::Packet::BodyMaxSize * 3, 0);
			const unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
			std::default_random_engine generator(seed);
			std::uniform_int_distribution<unsigned int> distribution(0, 100);
			for (uint8_t& character : sentMsg)
			{
				character = distribution(generator);
			}
			const auto originalMsg = sentMsg;

			sending.queue(std::move(sentMsg));

			// Incomplete sending with first packet
			std::array<PacketUnit, udp::Packet::PacketMaxSize> buffer0;
			sending.serialize(datagramId++, buffer0.data(), buffer0.size());
			reception.onReceived(buffer0.data(), buffer0.size());
			sending.onAcked(0);

			const std::vector<std::vector<PacketUnit>> completePackets = reception.unserialize();
			ASSERT_TRUE(reception.size() == 1);
			ASSERT_TRUE(reception.lastProcessed() == std::numeric_limits<udp::Packet::IdType>::max());
			ASSERT_TRUE(completePackets.empty());
		}

		void ReceptionHandlerTest::receiveLostPartialPacketTest()
		{
			SendingHandler sending;
			ReceptionHandler reception;
			udp::Datagram::IdType datagramId = 0;

			std::vector<uint8_t> sentMsg(udp::Packet::BodyMaxSize * 3, 0);
			const unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
			std::default_random_engine generator(seed);
			std::uniform_int_distribution<unsigned int> distribution(0, 100);
			for (uint8_t& character : sentMsg)
			{
				character = distribution(generator);
			}
			const auto originalMsg = sentMsg;

			sending.queue(std::move(sentMsg));

			// First packet
			std::array<PacketUnit, udp::Packet::PacketMaxSize> buffer0;
			sending.serialize(datagramId++, buffer0.data(), buffer0.size());
			reception.onReceived(buffer0.data(), buffer0.size());
			sending.onAcked(0);

			// Lost in-between packet
			std::array<PacketUnit, udp::Packet::PacketMaxSize> buffer1;
			sending.serialize(datagramId++, buffer1.data(), buffer1.size());

			// Last packet
			std::array<PacketUnit, udp::Packet::PacketMaxSize> buffer2;
			sending.serialize(datagramId++, buffer2.data(), buffer2.size());
			reception.onReceived(buffer2.data(), buffer2.size());
			sending.onAcked(2);

			// Empty packet
			std::array<PacketUnit, udp::Packet::PacketMaxSize> buffer3;
			const size_t serialized3 = sending.serialize(datagramId++, buffer3.data(), buffer3.size());
			ASSERT_TRUE(serialized3 == 0);

			const std::vector<std::vector<PacketUnit>> completePackets = reception.unserialize();
			ASSERT_TRUE(reception.size() == 2);
			ASSERT_TRUE(reception.lastProcessed() == std::numeric_limits<udp::Packet::IdType>::max());
			ASSERT_TRUE(completePackets.empty());
		}

		void ReceptionHandlerTest::receiveResendPartialPacketTest()
		{
			SendingHandler sending;
			ReceptionHandler reception;
			udp::Datagram::IdType datagramId = 0;

			std::vector<uint8_t> sentMsg(udp::Packet::BodyMaxSize * 3, 0);
			const unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
			std::default_random_engine generator(seed);
			std::uniform_int_distribution<unsigned int> distribution(0, 100);
			for (uint8_t& character : sentMsg)
			{
				character = distribution(generator);
			}
			const auto originalMsg = sentMsg;

			sending.queue(std::move(sentMsg));

			// First packet
			std::array<PacketUnit, udp::Packet::PacketMaxSize> buffer0;
			sending.serialize(datagramId++, buffer0.data(), buffer0.size());
			reception.onReceived(buffer0.data(), buffer0.size());
			sending.onAcked(0);

			// Lost in-between packet
			std::array<PacketUnit, udp::Packet::PacketMaxSize> buffer1;
			sending.serialize(datagramId++, buffer1.data(), buffer1.size());

			// Last packet
			std::array<PacketUnit, udp::Packet::PacketMaxSize> buffer2;
			sending.serialize(datagramId++, buffer2.data(), buffer2.size());
			reception.onReceived(buffer2.data(), buffer2.size());
			sending.onAcked(2);

			// Empty packet
			std::array<PacketUnit, udp::Packet::PacketMaxSize> buffer3;
			const size_t serialized3 = sending.serialize(datagramId++, buffer3.data(), buffer3.size());
			ASSERT_TRUE(serialized3 == 0);

			// Re-send packet
			sending.onLost(1);
			std::array<PacketUnit, udp::Packet::PacketMaxSize> buffer4;
			sending.serialize(datagramId++, buffer4.data(), buffer4.size());
			reception.onReceived(buffer4.data(), buffer4.size());
			sending.onAcked(1);

			const std::vector<std::vector<PacketUnit>> completePackets = reception.unserialize();
			ASSERT_TRUE(reception.size() == 0);
			ASSERT_TRUE(reception.lastProcessed() == 2);
			ASSERT_TRUE(completePackets.size() == 1);
			ASSERT_TRUE(completePackets[0].size() == originalMsg.size());
			ASSERT_TRUE(completePackets[0] == originalMsg);
		}

	}
}
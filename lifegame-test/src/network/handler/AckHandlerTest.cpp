#include <network/handler/AckHandlerTest.hpp>
#include <network/handler/AckHandler.hpp>
#include <network/helper/BitHelper.hpp>

#include <bitset>
#include <iostream>

namespace network
{
	namespace handler
	{
		static constexpr uint64_t MASK_COMPLETE = std::numeric_limits<uint64_t>::max();
		static const uint64_t MASK_FIRST_ACKED = network::helper::UNSIGNED_LONG_BIG_ENDIAN_UNITY;
		static const uint64_t MASK_FIRST_MISSING = ~MASK_FIRST_ACKED;;
		static const uint64_t MASK_LAST_ACKED = (MASK_FIRST_ACKED << 63);

		void AckHandlerTest::test()
		{
			
			constructTest();
			updateFirstPacketTest();
			updateMissingPacketTest();
			updateLatePacketTest();
			updateEmptyMaskPacketTest();
			updateLostPacketTest();
			updateOutdatedPacketTest();
			updateLostPacketAgainTest();
			updateOutdatedPacketAgainTest();
			
			updateLateMaskPacketTest();
		}

		void AckHandlerTest::constructTest()
		{
			AckHandler handler;
			ASSERT_TRUE(handler.lastAck() == std::numeric_limits<uint16_t>::max());
			ASSERT_TRUE(handler.previousAcks() == MASK_COMPLETE);
			ASSERT_FALSE(handler.isAcked(0));
			ASSERT_FALSE(handler.isNewlyAcked(0));
			ASSERT_TRUE(handler.losses().empty());
		}

		void AckHandlerTest::updateFirstPacketTest()
		{
			AckHandler handler;
			handler.update(0, MASK_COMPLETE, true);
			ASSERT_TRUE(handler.lastAck() == 0);
			ASSERT_TRUE(handler.previousAcks() == MASK_COMPLETE);
			ASSERT_TRUE(handler.isAcked(0));
			ASSERT_TRUE(handler.isNewlyAcked(0));
			ASSERT_TRUE(handler.newAcks().size() == 1);
			ASSERT_TRUE(handler.newAcks()[0] == 0);
			ASSERT_TRUE(handler.losses().empty());
		}

		void AckHandlerTest::updateMissingPacketTest()
		{
			AckHandler handler;
			handler.update(0, MASK_COMPLETE, true);

			handler.update(2, MASK_FIRST_MISSING, true);
			ASSERT_TRUE(handler.lastAck() == 2);
			ASSERT_TRUE(handler.previousAcks() == MASK_FIRST_MISSING);
			ASSERT_TRUE(handler.isAcked(2));
			ASSERT_TRUE(handler.isAcked(0));
			ASSERT_TRUE(handler.isNewlyAcked(2));
			ASSERT_FALSE(handler.isNewlyAcked(0));
			ASSERT_TRUE(handler.losses().empty());
		}

		void AckHandlerTest::updateLatePacketTest()
		{
			AckHandler handler;
			handler.update(0, MASK_COMPLETE, true);
			handler.update(2, MASK_FIRST_MISSING, true);

			handler.update(1, MASK_COMPLETE, true);
			ASSERT_TRUE(handler.lastAck() == 2);
			ASSERT_TRUE(handler.previousAcks() == MASK_COMPLETE);
			ASSERT_TRUE(handler.isAcked(1));
			ASSERT_TRUE(handler.isAcked(2));
			ASSERT_TRUE(handler.isAcked(0));
			ASSERT_TRUE(handler.isNewlyAcked(1));
			ASSERT_FALSE(handler.isNewlyAcked(2));
			ASSERT_FALSE(handler.isNewlyAcked(0));
			ASSERT_TRUE(handler.losses().empty());
		}

		void AckHandlerTest::updateEmptyMaskPacketTest()
		{
			AckHandler handler;
			handler.update(0, MASK_COMPLETE, true);
			handler.update(2, MASK_FIRST_MISSING, true);
			handler.update(1, MASK_COMPLETE, true);

			handler.update(66, 0, true);
			ASSERT_TRUE(handler.lastAck() == 66);
			ASSERT_TRUE(handler.isNewlyAcked(66));
			ASSERT_TRUE(handler.previousAcks() == MASK_LAST_ACKED);
			ASSERT_TRUE(handler.losses().empty());

			handler.update(67, 0, true);
			ASSERT_TRUE(handler.lastAck() == 67);
			ASSERT_TRUE(handler.isNewlyAcked(67));
			ASSERT_FALSE(handler.isNewlyAcked(66));
			ASSERT_TRUE(handler.previousAcks() == MASK_FIRST_ACKED);
		}

		void AckHandlerTest::updateLostPacketTest()
		{
			AckHandler handler;
			handler.update(0, MASK_COMPLETE, true);
			handler.update(2, MASK_FIRST_MISSING, true);
			handler.update(1, MASK_COMPLETE, true);
			handler.update(66, 0, true);
			handler.update(67, 0, true);

			handler.update(68, MASK_COMPLETE, true);
			ASSERT_TRUE(handler.lastAck() == 68);
			ASSERT_TRUE(handler.isNewlyAcked(68));
			ASSERT_TRUE(handler.previousAcks() == MASK_COMPLETE);
			auto losses = handler.losses();
			ASSERT_TRUE(losses.size() == 1);
			ASSERT_TRUE(losses[0] == 3);
			for (uint16_t i = 4; i < 66; ++i)
			{
				ASSERT_TRUE(handler.isNewlyAcked(i));
			}
		}

		void AckHandlerTest::updateOutdatedPacketTest()
		{
			AckHandler handler;
			handler.update(0, MASK_COMPLETE, true);
			handler.update(2, MASK_FIRST_MISSING, true);
			handler.update(1, MASK_COMPLETE, true);
			handler.update(66, 0, true);
			handler.update(67, 0, true);
			handler.update(68, MASK_COMPLETE, true);

			handler.update(0, 0, true);
			ASSERT_TRUE(handler.lastAck() == 68);
			ASSERT_FALSE(handler.isNewlyAcked(68));
			ASSERT_TRUE(handler.previousAcks() == MASK_COMPLETE);
		}

		void AckHandlerTest::updateLostPacketAgainTest()
		{
			AckHandler handler;
			handler.update(0, MASK_COMPLETE, true);
			handler.update(2, MASK_FIRST_MISSING, true);
			handler.update(1, MASK_COMPLETE, true);
			handler.update(66, 0, true);
			handler.update(67, 0, true);
			handler.update(68, MASK_COMPLETE, true);
			handler.update(0, 0, true);
			// TODO clear losses ?
			handler.losses();

			handler.update(133, 0, true);
			ASSERT_TRUE(handler.lastAck() == 133);
			ASSERT_TRUE(handler.previousAcks() == 0);
			auto losses133 = handler.losses();
			ASSERT_TRUE(losses133.size() == 0);
			
			handler.update(132, MASK_COMPLETE, true);
			ASSERT_TRUE(handler.lastAck() == 133);
			ASSERT_TRUE(handler.previousAcks() == MASK_COMPLETE);
			ASSERT_TRUE(handler.losses().empty());

			handler.update(234, 0, true);
			ASSERT_TRUE(handler.lastAck() == 234);
			ASSERT_TRUE(handler.previousAcks() == 0);
			auto losses234 = handler.losses();
			const auto firstLost = 134;
			const auto lastLost = 169;
			const auto totalLost = lastLost - firstLost + 1;
			ASSERT_TRUE(losses234.size() == totalLost);
			for (auto i = 0; i < totalLost; ++i)
			{
				ASSERT_TRUE(losses234[i] == firstLost + i);
			}
		}

		void AckHandlerTest::updateOutdatedPacketAgainTest()
		{
			AckHandler handler;
			handler.update(0, MASK_COMPLETE, true);
			handler.update(2, MASK_FIRST_MISSING, true);
			handler.update(1, MASK_COMPLETE, true);
			handler.update(66, 0, true);
			handler.update(67, 0, true);
			handler.update(68, MASK_COMPLETE, true);
			handler.update(0, 0, true);
			handler.update(133, 0, true);
			handler.update(132, MASK_COMPLETE, true);
			handler.update(234, 0, true);
			// TODO clear losses
			handler.losses();

			handler.update(234, MASK_COMPLETE, true);
			handler.update(236, MASK_COMPLETE, true);
			handler.update(301, 0, true);
			ASSERT_TRUE(handler.lastAck() == 301);
			ASSERT_TRUE(handler.previousAcks() == 0);
			ASSERT_TRUE(handler.losses().empty());
			ASSERT_FALSE(handler.isAcked(237));

			handler.update(237, MASK_COMPLETE, true);
			ASSERT_TRUE(handler.lastAck() == 301);
			ASSERT_TRUE(handler.previousAcks() == MASK_LAST_ACKED);
			ASSERT_TRUE(handler.losses().empty());
			ASSERT_TRUE(handler.isAcked(237));
			ASSERT_TRUE(handler.isNewlyAcked(237));
		}

		void AckHandlerTest::updateLateMaskPacketTest()
		{
			AckHandler handler;
			handler.update(0, MASK_COMPLETE, true);
			handler.update(2, MASK_FIRST_MISSING, true);
			handler.update(1, MASK_COMPLETE, true);
			handler.update(66, 0, true);
			handler.update(67, 0, true);
			handler.update(68, MASK_COMPLETE, true);
			handler.update(0, 0, true);
			handler.update(133, 0, true);
			handler.update(132, MASK_COMPLETE, true);
			handler.update(234, 0, true);
			handler.update(234, MASK_COMPLETE, true);
			handler.update(236, MASK_COMPLETE, true);
			handler.update(301, 0, true);
			handler.update(237, MASK_COMPLETE, true);
			// TODO clear losses ?
			handler.losses();

			handler.update(301, MASK_COMPLETE, true);
			ASSERT_TRUE(handler.lastAck() == 301);
			ASSERT_TRUE(handler.previousAcks() == MASK_COMPLETE);
			ASSERT_TRUE(handler.losses().empty());

			handler.update(303, MASK_COMPLETE, true);
			auto newAcks = handler.newAcks();
			ASSERT_TRUE(newAcks.size() == 2);
			ASSERT_TRUE(newAcks[0] == 302);
			ASSERT_TRUE(newAcks[1] == 303);
		}

	}
}
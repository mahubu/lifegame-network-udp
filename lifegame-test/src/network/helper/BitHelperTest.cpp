#include <network/helper/BitHelperTest.hpp>
#include <network/helper/BitHelper.hpp>

namespace network
{
	namespace helper
	{

		void BitHelperTest::test()
		{
			setBitTest();
			unsetBitTest();
			hasBitTest();
			isNewerTest();
			differenceTest();
		}

		void BitHelperTest::setBitTest()
		{
			uint64_t bitfield = 0;
			ASSERT_TRUE(bitfield == 0);

			setBit(bitfield, 0);
			ASSERT_TRUE(hasBit(bitfield, 0));

			setBit(bitfield, 5);
			ASSERT_TRUE(hasBit(bitfield, 5));
		}

		void BitHelperTest::unsetBitTest()
		{
			uint64_t bitfield = 0;
			ASSERT_TRUE(bitfield == 0);

			setBit(bitfield, 0);
			ASSERT_FALSE(bitfield == 0);
			unsetBit(bitfield, 0);
			ASSERT_TRUE(bitfield == 0);

			setBit(bitfield, 5);
			ASSERT_FALSE(bitfield == 0);
			unsetBit(bitfield, 5);
			ASSERT_TRUE(bitfield == 0);
		}

		void BitHelperTest::hasBitTest()
		{
			// TODO
		}
		
		void BitHelperTest::isNewerTest()
		{
			ASSERT_TRUE(isNewer(1, 0));
			ASSERT_FALSE(isNewer(0, 1));
			ASSERT_FALSE(isNewer(0, 0));
			ASSERT_TRUE(isNewer(0, std::numeric_limits<uint16_t>::max()));
		}
		
		void BitHelperTest::differenceTest()
		{
			ASSERT_TRUE(difference(0, 0) == 0);
			ASSERT_TRUE(difference(1, 0) == 1);
			ASSERT_TRUE(difference(0, std::numeric_limits<uint16_t>::max()) == 1);
		}

	}
}
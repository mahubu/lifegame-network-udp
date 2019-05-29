#pragma once

#include "TestHelper.hpp"

namespace network
{
	namespace handler
	{
		class AckHandlerTest
		{
		public:
			static void test();
		private:
			static void constructTest();
			static void updateFirstPacketTest();
			static void updateMissingPacketTest();
			static void updateLatePacketTest();
			static void updateEmptyMaskPacketTest();
			static void updateLostPacketTest();
			static void updateOutdatedPacketTest();
			static void updateLostPacketAgainTest();
			static void updateOutdatedPacketAgainTest();
			static void updateLateMaskPacketTest();
			
		};
	}
}
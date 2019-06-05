#pragma once

#include "TestHelper.hpp"

namespace network
{
	namespace handler
	{
		class SendingHandlerTest
		{
		public:
			static void test();
		private:
			static void constructTest();

			static void queueCompletePacketTest();
			static void serializeCompletePacketTest();
			static void onAckedCompletePacketTest();
			static void onLostCompletePacketTest();
			static void onResendCompletePacketTest();

			static void queuePartialPacketTest();
			static void serializePartialPacketTest();
			static void onAckedPartialPacketTest();
			static void onLostPartialPacketTest();
			static void onResendPartialPacketTest();

		};
	}
}
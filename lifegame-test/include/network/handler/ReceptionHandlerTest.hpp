#pragma once

#include "TestHelper.hpp"

namespace network
{
	namespace handler
	{
		class ReceptionHandlerTest
		{
		public:
			static void test();
		private:
			static void constructTest();
			static void receiveCompletePacketTest();
			static void receivePartialPacketTest();
			static void receiveIncompletePartialPacketTest();
			static void receiveLostPartialPacketTest();
			static void receiveResendPartialPacketTest();
		};
	}
}
#pragma once

#include "TestHelper.hpp"

namespace network
{
	namespace helper
	{
		class BitHelperTest
		{
		public:
			static void test();
		private:
			static void setBitTest();
			static void unsetBitTest();
			static void hasBitTest();
			static void isNewerTest();
			static void differenceTest();
		};
	}
}
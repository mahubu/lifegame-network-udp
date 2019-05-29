#include <network/handler/AckHandlerTest.hpp>
#include <network/helper/BitHelperTest.hpp>
#include <iostream>

int main()
{
    std::cout << "Test!\n"; 

	std::cout << "BitHelper test:\n";
	network::helper::BitHelperTest::test();

	std::cout << "AckHandler test:\n";
	network::handler::AckHandlerTest::test();
}



#include <network/handler/AckHandlerTest.hpp>
#include <network/handler/ReceptionHandlerTest.hpp>
#include <network/handler/SendingHandlerTest.hpp>
#include <network/helper/BitHelperTest.hpp>
#include <iostream>

int main()
{
	std::cout << "Test!\n";

	std::cout << "BitHelper test:\n";
	network::helper::BitHelperTest::test();

	std::cout << "AckHandler test:\n";
	network::handler::AckHandlerTest::test();

	std::cout << "ReceptionHandler test:\n";
	network::handler::ReceptionHandlerTest::test();

	std::cout << "SendingHandler test:\n";
	network::handler::SendingHandlerTest::test();
}



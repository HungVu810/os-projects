#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <functional>
#include <exception>

//#ifdef _NDEBUG
//	constexpr auto printExceptionMsg = true;
//#else
//	constexpr auto printExceptionMsg = false;
//#endif

#include "Predefined.h"
#include "PCB.h"
#include "RCB.h"
#include "System.h"
#include "Shell.h"

// when call destroy process i, i must be the running process or its child

int main() // Accept arguments
{
	auto system = System::getInstance();
	auto shell = Shell::getInstance();

	while (true)
	{
		try
		{
			shell.run(system);
		}
		catch (const std::runtime_error& error)
		{
			const auto prompt = std::string_view{"* error"};
			//const auto prompt = std::string_view{error.what()};
			std::cout << prompt << '\n';
		}
	}
}



#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <functional>
#include <exception>

#include "Predefined.h"
#include "RCB.h"
#include "PCB.h"
#include "System.h"
#include "Shell.h"

namespace
{
	[[nodiscard]] std::unordered_map<std::string, CommandFunction> getCommandMap()
	{
		return {
			{"cr", std::mem_fn(&System::create)}
			, {"de", std::mem_fn(&System::destroy)}
			, {"rq", std::mem_fn(&System::request)}
			, {"rl", std::mem_fn(&System::release)}
			, {"to", std::mem_fn(&System::timeout)}
			, {"in", std::mem_fn(&System::init)}
		};
	}
}

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
			std::cout << "* error\n";
		}
	}
}



#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <functional>
#include <exception>

#include "PCB.h"
#include "RCB.h"
#include "System.h"
#include "Shell.h"

namespace
{
	[[nodiscard]] std::unordered_map<std::string, CommandFunction> getCommandMap() noexcept
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
	std::cout << "hello";
	//while (true)
	//{
	//	try
	//	{
	//		auto system = System::getInstance();
	//		auto shell = Shell::getInstance();
	//		shell.run(system);
	//	}
	//	catch (const std::runtime_error& error)
	//	{
	//		std::cout << "* error\n";
	//	}
	//}
}



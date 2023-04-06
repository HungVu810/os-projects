#define CATCH_CONFIG_MAIN
#include "catch.hpp"

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

TEST_CASE("PCB instantiation")
{
	const auto pcb = PCB{};
	REQUIRE(pcb.state == PCB::State::Ready);
	REQUIRE(pcb.parent == -1);
	REQUIRE(pcb.childs.empty());
	REQUIRE(pcb.resources.empty());
	REQUIRE(pcb.priority >= 0); REQUIRE(pcb.priority <= 2);
}

TEST_CASE("RCB instantiation")
{
	const auto rcb = RCB{};
	REQUIRE(rcb.state == RCB::State::Free);
	REQUIRE(rcb.waitList.empty());
}

TEST_CASE("System instantiation")
{
	const auto system = System::getInstance();
	REQUIRE(System::isInstantiated);
}

TEST_CASE("Shell instantiation")
{
	const auto shell = Shell::getInstance();
	REQUIRE(Shell::isInstantiated);
}











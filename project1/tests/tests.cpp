#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <iostream>

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

namespace singleton
{
	auto system = System::getInstance();
	auto shell = Shell::getInstance();
}

// ============ SYSTEM ============
TEST_CASE("System instantiation")
{
	REQUIRE(System::isInstantiated);
}
TEST_CASE("create() input")
{
	REQUIRE_NOTHROW(singleton::system.create({}));

	REQUIRE_THROWS(singleton::system.create({"x"}));
	REQUIRE_THROWS(singleton::system.create({"$"}));
	REQUIRE_THROWS(singleton::system.create({"10"}));
	REQUIRE_THROWS(singleton::system.create({"x $ 10"}));
}
TEST_CASE("destroy() input")
{
	REQUIRE_NOTHROW(singleton::system.destroy({"0"}));
	REQUIRE_NOTHROW(singleton::system.destroy({std::to_string(NUM_PROCESS - 1)}));

	REQUIRE_THROWS(singleton::system.destroy({}));
	REQUIRE_THROWS(singleton::system.destroy({"-1"}));
	REQUIRE_THROWS(singleton::system.destroy({std::to_string(NUM_PROCESS)}));
	REQUIRE_THROWS(singleton::system.destroy({"x"}));
	REQUIRE_THROWS(singleton::system.destroy({"$"}));
	REQUIRE_THROWS(singleton::system.destroy({"x $"}));
}
TEST_CASE("request() input")
{
	REQUIRE_NOTHROW(singleton::system.request({"0"}));
	REQUIRE_NOTHROW(singleton::system.request({std::to_string(NUM_RESOURCE - 1)}));

	REQUIRE_THROWS(singleton::system.request({}));
	REQUIRE_THROWS(singleton::system.request({"-1"}));
	REQUIRE_THROWS(singleton::system.request({std::to_string(NUM_RESOURCE)}));
	REQUIRE_THROWS(singleton::system.request({"x"}));
	REQUIRE_THROWS(singleton::system.request({"$"}));
	REQUIRE_THROWS(singleton::system.request({"x $"}));
}
// ============ SYSTEM ============

// ============ SHELL ============
TEST_CASE("Shell instantiation")
{
	REQUIRE(Shell::isInstantiated);
}
// ============ SHELL ============











#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <iostream>

#include "Predefined.h"
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
	REQUIRE(pcb.state == PCB::State::Free);
	REQUIRE(pcb.parent == std::nullopt);
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
	for (int i = 0; i < ProcessID::MAX_EXCLUSIVE - 1; i++) // Only N - 1 processes can be create because process 0 is already running
	{
		REQUIRE_NOTHROW(singleton::system.create({}));
	}
	REQUIRE_THROWS(singleton::system.create({})); // Over limit
	const auto processes = singleton::system.getProcesses();
	const auto isReady = [](const PCB& process)
	{
		return process.state == PCB::State::Ready;
	};
	REQUIRE(std::ranges::all_of(processes, isReady));

	REQUIRE_THROWS(singleton::system.create({"x"}));
	REQUIRE_THROWS(singleton::system.create({"$"}));
	REQUIRE_THROWS(singleton::system.create({"10"}));
	REQUIRE_THROWS(singleton::system.create({"x $ 10"}));
}
TEST_CASE("destroy() input")
{
	REQUIRE_NOTHROW(singleton::system.destroy({"0"}));
	REQUIRE_NOTHROW(singleton::system.destroy({std::to_string(ProcessID::MAX_EXCLUSIVE - 1)}));

	REQUIRE_THROWS(singleton::system.destroy({}));
	REQUIRE_THROWS(singleton::system.destroy({"-1"}));
	REQUIRE_THROWS(singleton::system.destroy({std::to_string(ProcessID::MAX_EXCLUSIVE)}));
	REQUIRE_THROWS(singleton::system.destroy({"x"}));
	REQUIRE_THROWS(singleton::system.destroy({"$"}));
	REQUIRE_THROWS(singleton::system.destroy({"x $"}));
}
TEST_CASE("request() input")
{
	// requesting the same resource
	// releashing the same resource
	// destroy new process

	for (int i = 0; i < ResourceID::MAX_EXCLUSIVE; i++) // Create max number of resources without destroy
	{
		REQUIRE_NOTHROW(singleton::system.request({std::to_string(i)}));
	}
	REQUIRE_THROWS(singleton::system.request({std::to_string(ResourceID::MAX_EXCLUSIVE)})); // Over limit

	REQUIRE_THROWS(singleton::system.request({}));
	REQUIRE_THROWS(singleton::system.request({"-1"}));
	REQUIRE_THROWS(singleton::system.request({std::to_string(ResourceID::MAX_EXCLUSIVE)}));
	REQUIRE_THROWS(singleton::system.request({"x"}));
	REQUIRE_THROWS(singleton::system.request({"$"}));
	REQUIRE_THROWS(singleton::system.request({"x $"}));
}
TEST_CASE("release() input")
{
	REQUIRE_NOTHROW(singleton::system.release({"0"}));
	REQUIRE_NOTHROW(singleton::system.release({std::to_string(ResourceID::MAX_EXCLUSIVE - 1)}));

	REQUIRE_THROWS(singleton::system.release({}));
	REQUIRE_THROWS(singleton::system.release({"-1"}));
	REQUIRE_THROWS(singleton::system.release({std::to_string(ResourceID::MAX_EXCLUSIVE)}));
	REQUIRE_THROWS(singleton::system.release({"x"}));
	REQUIRE_THROWS(singleton::system.release({"$"}));
	REQUIRE_THROWS(singleton::system.release({"x $"}));
}
TEST_CASE("timeout() input")
{
	REQUIRE_NOTHROW(singleton::system.timeout({}));

	REQUIRE_THROWS(singleton::system.timeout({"x"}));
	REQUIRE_THROWS(singleton::system.timeout({"$"}));
	REQUIRE_THROWS(singleton::system.timeout({"10"}));
	REQUIRE_THROWS(singleton::system.timeout({"x $ 10"}));
}
TEST_CASE("init() input")
{
	REQUIRE_NOTHROW(singleton::system.init({}));

	REQUIRE_THROWS(singleton::system.init({"x"}));
	REQUIRE_THROWS(singleton::system.init({"$"}));
	REQUIRE_THROWS(singleton::system.init({"10"}));
	REQUIRE_THROWS(singleton::system.init({"x $ 10"}));
}
// ============ SYSTEM ============

// ============ SHELL ============
TEST_CASE("Shell instantiation")
{
	REQUIRE(Shell::isInstantiated);
}
// ============ SHELL ============











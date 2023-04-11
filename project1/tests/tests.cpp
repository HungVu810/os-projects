#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <iostream>
#include <sstream>
#include <array>

#include "Predefined.h"
#include "PCB.h"
#include "RCB.h"
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

// BASIC ============
TEST_CASE("init() input")
{
	// Invalid inputs
	REQUIRE_THROWS(singleton::system.init({"x"}));
	REQUIRE_THROWS(singleton::system.init({"$"}));
	REQUIRE_THROWS(singleton::system.init({"10"}));
	REQUIRE_THROWS(singleton::system.init({"x $ 10"}));

	// Valid input
	REQUIRE_NOTHROW(singleton::system.init({}));
}

TEST_CASE("create() input")
{
	singleton::system.init({});

	// Invalid inputs
	REQUIRE_THROWS(singleton::system.create({"x"}));
	REQUIRE_THROWS(singleton::system.create({"$"}));
	REQUIRE_THROWS(singleton::system.create({"10"}));
	REQUIRE_THROWS(singleton::system.create({"x $ 10"}));

	// Valid input
	REQUIRE_NOTHROW(singleton::system.create({}));
}

TEST_CASE("destroy() input")
{
	singleton::system.init({});

	// Invalid inputs
	REQUIRE_THROWS(singleton::system.destroy({}));
	REQUIRE_THROWS(singleton::system.destroy({"-1"}));
	REQUIRE_THROWS(singleton::system.destroy({std::to_string(ProcessID::MAX_EXCLUSIVE)}));
	REQUIRE_THROWS(singleton::system.destroy({"x"}));
	REQUIRE_THROWS(singleton::system.destroy({"$"}));
	REQUIRE_THROWS(singleton::system.destroy({"x $"}));

	// Valid input
	REQUIRE_NOTHROW(singleton::system.destroy({"0"}));
}

TEST_CASE("timeout() input")
{
	singleton::system.init({});

	// Invalid inputs
	REQUIRE_THROWS(singleton::system.timeout({"x"}));
	REQUIRE_THROWS(singleton::system.timeout({"$"}));
	REQUIRE_THROWS(singleton::system.timeout({"10"}));
	REQUIRE_THROWS(singleton::system.timeout({"x $ 10"}));

	// Valid input
	REQUIRE_NOTHROW(singleton::system.timeout({}));
}

TEST_CASE("request() input")
{
	singleton::system.init({});

	// Invalid inputs
	REQUIRE_THROWS(singleton::system.request({}));
	REQUIRE_THROWS(singleton::system.request({"-1"}));
	REQUIRE_THROWS(singleton::system.request({std::to_string(ResourceID::MAX_EXCLUSIVE)}));
	REQUIRE_THROWS(singleton::system.request({"x"}));
	REQUIRE_THROWS(singleton::system.request({"$"}));
	REQUIRE_THROWS(singleton::system.request({"x $"}));

	// Valid input
	REQUIRE_NOTHROW(singleton::system.request({"0"}));
}

TEST_CASE("release() input")
{
	singleton::system.init({});

	// Invalid inputs
	REQUIRE_THROWS(singleton::system.release({}));
	REQUIRE_THROWS(singleton::system.release({"-1"}));
	REQUIRE_THROWS(singleton::system.release({std::to_string(ResourceID::MAX_EXCLUSIVE)}));
	REQUIRE_THROWS(singleton::system.release({"x"}));
	REQUIRE_THROWS(singleton::system.release({"$"}));
	REQUIRE_THROWS(singleton::system.release({"x $"}));

	// Valid input
	singleton::system.request({"0"});
	REQUIRE_NOTHROW(singleton::system.release({"0"}));
}
// BASIC ============

// ADVANCE ============
TEST_CASE("init()")
{
	singleton::system.init({});
	const auto& processes = singleton::system.getProcesses();
	const auto& resources = singleton::system.getResources();
	const auto& readyList = singleton::system.getReadyList();

	const auto isGood = [](const auto& pair)
	{
		const auto& [process, index] = pair;
		return process.state == (index == 0 ? PCB::State::Ready : PCB::State::Free)
			&& process.parent == std::nullopt
			&& process.childs.size() == 0
			&& process.resources.size() == 0;
	};
	const auto index = std::views::iota(0U, processes.size());
	REQUIRE(std::ranges::all_of(std::views::zip(processes, index), isGood));
}

TEST_CASE("create() without timeout")
{
	singleton::system.init({});
	const auto& processes = singleton::system.getProcesses();
	const auto& resources = singleton::system.getResources();
	const auto& readyList = singleton::system.getReadyList();
	auto outputCapture = OutputCapture{};
	outputCapture.capture();

	// Create check
	for (int i = 1; i < ProcessID::MAX_EXCLUSIVE; i++) // Only N - 1 processes can be create because process 0 is already running
	{
		singleton::system.create({});
		REQUIRE(processes[i].state == PCB::State::Ready);
		REQUIRE(readyList.size() == i + 1);
		REQUIRE(processes[i].parent == 0);
		REQUIRE(processes[i].childs.size() == 0);
		REQUIRE(processes[i].resources.size() == 0);
		REQUIRE(readyList.back() == i);
		REQUIRE(outputCapture.getOutput() == "process " + std::to_string(i) + " created");
	}

	// Overlimit check
	REQUIRE_THROWS(singleton::system.create({}));
}

TEST_CASE("timeout()")
{
	singleton::system.init({});
	const auto& processes = singleton::system.getProcesses();
	const auto& resources = singleton::system.getResources();
	const auto& readyList = singleton::system.getReadyList();
	auto outputCapture = OutputCapture{};
	outputCapture.capture();

	// Create process 1 and 2
	singleton::system.create({});
	singleton::system.create({});

	// Context switch from process 0 to 1, RL: 1 2 0
	singleton::system.timeout({});
	REQUIRE(readyList.front() == 1);
	REQUIRE(readyList.back() == 0);
	REQUIRE(outputCapture.getOutput() == "process 1 running");

	// Context switch from process 1 to 2, RL: 2 0 1
	singleton::system.timeout({});
	REQUIRE(readyList.front() == 2);
	REQUIRE(readyList.back() == 1);
	REQUIRE(outputCapture.getOutput() == "process 2 running");
}

TEST_CASE("create()/destroy() with timeout")
{
	singleton::system.init({});
	const auto& processes = singleton::system.getProcesses();
	const auto& resources = singleton::system.getResources();
	const auto& readyList = singleton::system.getReadyList();
	auto outputCapture = OutputCapture{};
	outputCapture.capture();

	// Create process 1 and 2, childs of 0, RL: 0, 1, 2
	singleton::system.create({});
	singleton::system.create({});

	// Switch to 1, create childs 3 and 4, RL: 1, 2, 0, 3, 4
	singleton::system.timeout({});
	singleton::system.create({});
	REQUIRE(outputCapture.getOutput() == "process 3 created");
	singleton::system.create({});
	REQUIRE(outputCapture.getOutput() == "process 4 created");
	REQUIRE(readyList == std::list<ProcessID>{1, 2, 0, 3, 4});
	REQUIRE(processes[3].state == PCB::State::Ready);
	REQUIRE(processes[4].state == PCB::State::Ready);
	REQUIRE(processes[3].parent == 1);
	REQUIRE(processes[4].parent == 1);
	REQUIRE(processes[1].childs == std::vector<ProcessID>{3, 4});

	// Switch to 2, create childs 5 and 6, RL: 2, 0, 3, 4, 1, 5, 6
	singleton::system.timeout({});
	singleton::system.create({});
	singleton::system.create({});
	REQUIRE(readyList == std::list<ProcessID>{2, 0, 3, 4, 1, 5, 6});
	REQUIRE(processes[5].state == PCB::State::Ready);
	REQUIRE(processes[6].state == PCB::State::Ready);
	REQUIRE(processes[5].parent == 2);
	REQUIRE(processes[6].parent == 2);
	REQUIRE(processes[2].childs == std::vector<ProcessID>{5, 6});

	// Destroy process 1
	singleton::system.destroy({"1"});
	REQUIRE(outputCapture.getOutput() == "3 processes destroyed");
	REQUIRE(readyList == std::list<ProcessID>{2, 0, 5, 6});
	REQUIRE(processes[1].childs.size() == 0);
	REQUIRE(processes[1].parent == std::nullopt);
	REQUIRE(processes[1].state == PCB::State::Free);

	// Destroy process 0
	singleton::system.destroy({"0"});
	REQUIRE(outputCapture.getOutput() == "4 processes destroyed");
	REQUIRE(readyList.empty());
	REQUIRE(processes[0].childs.size() == 0);
	REQUIRE(processes[0].state == PCB::State::Free);

	// Destroy free process check
	REQUIRE_THROWS(singleton::system.destroy({"0"}));
	REQUIRE_THROWS(singleton::system.destroy({std::to_string(ProcessID::MAX_EXCLUSIVE - 1)}));
}

TEST_CASE("request()/destroy() with timeout")
{
	const auto& processes = singleton::system.getProcesses();
	const auto& resources = singleton::system.getResources();
	const auto& readyList = singleton::system.getReadyList();
	auto outputCapture = OutputCapture{};
	outputCapture.capture();

	// Request resource when there isn't any running process
	singleton::system.init({});
	singleton::system.destroy({"0"});
	REQUIRE_THROWS(singleton::system.request({"0"}));
	REQUIRE_THROWS(singleton::system.request({std::to_string(ResourceID::MAX_EXCLUSIVE - 1)}));

	singleton::system.init({});

	// Create childs of 0 to match the number of resource
	for (int i = 0; i < ResourceID::MAX_EXCLUSIVE - 1; i++) // Subtract 1 since we already have process 0
	{
		singleton::system.create({});
	}
	REQUIRE(readyList == std::list<ProcessID>{0, 1, 2, 3}); // Sanity check

	// Request resources for each process
	for (int i = 0; i < ResourceID::MAX_EXCLUSIVE; i++) // Create max number of resources for process 0 without destroy
	{
		singleton::system.request({std::to_string(i)});
		REQUIRE_THROWS(singleton::system.request({std::to_string(i)})); // Requesting the same owned resource for the running process
		REQUIRE(processes[i].state == PCB::State::Ready); // Check the running process isn't blocked if requesting the same resource
		REQUIRE(processes[i].resources.size() == 1);
		REQUIRE(processes[i].resources.front() == i);
		REQUIRE(resources[i].state == RCB::State::Allocated);
		REQUIRE(outputCapture.getOutput() == "resource " + std::to_string(i) + " allocated");
		singleton::system.timeout({}); // Context switch
	}

	// Create childs of 0 to match the number of resource
	for (int i = 0; i < ResourceID::MAX_EXCLUSIVE; i++)
	{
		singleton::system.create({});
	}
	for (int i = 0; i < ResourceID::MAX_EXCLUSIVE; i++) singleton::system.timeout({});
	REQUIRE(readyList == std::list<ProcessID>{4, 5, 6, 7, 0, 1, 2, 3}); // Sanity check

	// Blocking process 4, 5, 6, 7 check
	const int min = 4;
	const int max = 8;
	for (int i = min; i < max; i++)
	{
		const auto resourceID = i - min;
		singleton::system.request({std::to_string(resourceID)});
		REQUIRE(processes[i].state == PCB::State::Blocked);
		REQUIRE(std::ranges::find(readyList, ProcessID{(uint32_t)i}) == readyList.end());
		REQUIRE(resources[resourceID].waitList.front() == i);
		REQUIRE(outputCapture.getOutput(1) == "process " + std::to_string(i) + " blocked");
		REQUIRE(outputCapture.getOutput() == "process " + std::to_string((i + 1) % max) + " running");
	}

	REQUIRE(readyList == std::list<ProcessID>{0, 1, 2, 3}); // Sanity check

	// Unblock 4, 5, 6, 7 and owns 0, 1, 2, 3
	for (int i = 0; i < ResourceID::MAX_EXCLUSIVE; i++)
	{
		singleton::system.release({std::to_string(i)});
		REQUIRE(processes[i].resources.empty());
		REQUIRE(resources[i].waitList.empty());
		REQUIRE(resources[i].state == RCB::State::Allocated);
		REQUIRE(readyList.back() == i + min);
		REQUIRE(processes[i + min].state == PCB::State::Ready);
		REQUIRE(processes[i + min].resources[0] == i);
		REQUIRE(outputCapture.getOutput() == "resource " + std::to_string(i) + " released");
		singleton::system.timeout({}); // Context switch
	}

	REQUIRE(readyList == std::list<ProcessID>{4, 0, 5, 1, 6, 2, 7, 3}); // Sanity check

	// Free resource 0, 1, 2, 3 from 4, 5, 6, 7
	for (int i = min; i < max; i++)
	{
		const auto resourceID = i - min;
		singleton::system.release({std::to_string(resourceID)});
		REQUIRE(resources[resourceID].state == RCB::State::Free);
		REQUIRE_THROWS(singleton::system.release({std::to_string(resourceID)})); // Release the same resource check
		singleton::system.timeout({});
		singleton::system.timeout({});
	}
}
// ADVANCE ============

// RANDOM ============
TEST_CASE("random one")
{
	singleton::system.init({});
	const auto& processes = singleton::system.getProcesses();
	const auto& resources = singleton::system.getResources();
	const auto& readyList = singleton::system.getReadyList();
	auto outputCapture = OutputCapture{};
	outputCapture.capture();

	singleton::system.create({});
	singleton::system.create({});
	singleton::system.create({});

	singleton::system.destroy({"0"});
	REQUIRE(outputCapture.getOutput() == "4 processes destroyed"); // 0, 1, 2, 3
}

TEST_CASE("random two")
{
	singleton::system.init({});
	const auto& processes = singleton::system.getProcesses();
	const auto& resources = singleton::system.getResources();
	const auto& readyList = singleton::system.getReadyList();
	auto outputCapture = OutputCapture{};
	outputCapture.capture();

	singleton::system.create({}); // child 1 for process 0
	singleton::system.request({"0"}); // resource 0 for process 0

	singleton::system.timeout({}); // switch to process 1
	singleton::system.request({"0"}); // process 1 blocked, process 0 running

	singleton::system.release({"0"}); // unblock process 1, now owns resource 0, process 0 still running
	singleton::system.timeout({}); // switch to process 1
	REQUIRE_THROWS(singleton::system.request({"0"})); // request owning resource
}
// RANDOM ============
// ============ SYSTEM ============

// ============ SHELL ============
TEST_CASE("Shell instantiation")
{
	REQUIRE(Shell::isInstantiated);
}
// ============ SHELL ============











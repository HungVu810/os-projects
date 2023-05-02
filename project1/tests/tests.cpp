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

TEST_CASE("PCB instantiation")
{
	const auto pcb = PCB{};
	REQUIRE(pcb.state == PCB::State::Free);
	REQUIRE(pcb.parent == std::nullopt);
	REQUIRE(pcb.childs.empty());
	REQUIRE(pcb.resources.empty());
	REQUIRE(pcb.priority == 0);
	REQUIRE(pcb.id == 0);
}

TEST_CASE("RCB instantiation")
{
	const auto rcb = RCB{};
	REQUIRE(rcb.state == RCB::State::Free);
	REQUIRE(rcb.waitList.empty());
	REQUIRE(rcb.id == 0);
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

	const auto& processes = singleton::system.getProcesses();
	const auto& resources = singleton::system.getResources();
	const auto& readyList = singleton::system.getReadyList();

	const auto& zipProcess = std::views::zip(processes, std::views::iota(0U, ProcessID::MAX_EXCLUSIVE));
	for (const auto& [process, id] : zipProcess)
	{
		REQUIRE(process.state == (id == 0 ? PCB::State::Ready : PCB::State::Free));
		REQUIRE(process.parent == std::nullopt);
		REQUIRE(process.childs.empty());
		REQUIRE(process.resources.empty());
		REQUIRE(process.priority == 0);
		REQUIRE(process.id == id);
	}

	const auto& zipResource = std::views::zip(resources, std::views::iota(0U, ResourceID::MAX_EXCLUSIVE));
	for (const auto& [resource, id] : zipResource)
	{
		REQUIRE(resource.state == RCB::State::Free);
		REQUIRE(resource.id == id);
		REQUIRE(resource.remain == unitMap[id]);
		REQUIRE(resource.waitList.empty());
	}

	const auto& zipList = std::views::zip(readyList, std::views::iota(0U, PriorityID::MAX_EXCLUSIVE));
	for (const auto& [list, index] : zipList)
	{
		if (index == 0)
		{
			REQUIRE(list.size() == 1); // Process 0 is running
		}
		else
		{
			 REQUIRE(list.empty());
		}
	}
	REQUIRE(readyList.size() == PriorityID::MAX_EXCLUSIVE);
}

// BASIC ============
TEST_CASE("init() input")
{
	// Invalid inputs
	REQUIRE_THROWS(singleton::system.init({"x"}));
	REQUIRE_THROWS(singleton::system.init({"1"}));
	REQUIRE_THROWS(singleton::system.init({"0.5"}));
	REQUIRE_THROWS(singleton::system.init({"-1"}));

	// Valid input
	REQUIRE_NOTHROW(singleton::system.init({}));
}

TEST_CASE("create() input")
{
	singleton::system.init({});

	// Invalid inputs
	REQUIRE_THROWS(singleton::system.create({}));
	REQUIRE_THROWS(singleton::system.create({"-1"})); // Bound break
	REQUIRE_THROWS(singleton::system.create({std::to_string(PriorityID::MAX_EXCLUSIVE)})); // Bound break
	REQUIRE_THROWS(singleton::system.create({"0.5"})); // Fraction
	REQUIRE_THROWS(singleton::system.create({"x"}));

	// Valid input
	REQUIRE_NOTHROW(singleton::system.create({"0"}));
	REQUIRE_NOTHROW(singleton::system.create({std::to_string(PriorityID::MAX_EXCLUSIVE - 1)}));
}

TEST_CASE("destroy() input")
{
	singleton::system.init({});

	// Invalid inputs
	REQUIRE_THROWS(singleton::system.destroy({}));
	REQUIRE_THROWS(singleton::system.destroy({"-1"})); // Bound break
	REQUIRE_THROWS(singleton::system.destroy({std::to_string(PriorityID::MAX_EXCLUSIVE)})); // Bound break
	REQUIRE_THROWS(singleton::system.destroy({"0.5"})); // Fraction
	REQUIRE_THROWS(singleton::system.destroy({"x"}));

	// Valid input
	REQUIRE_NOTHROW(singleton::system.destroy({"0"}));
}

TEST_CASE("timeout() input")
{
	singleton::system.init({});

	// Invalid inputs
	REQUIRE_THROWS(singleton::system.timeout({"x"}));
	REQUIRE_THROWS(singleton::system.timeout({"10"}));

	// Valid input
	REQUIRE_NOTHROW(singleton::system.timeout({}));
}

TEST_CASE("request() input")
{
	singleton::system.init({});

	// Invalid inputs
	REQUIRE_THROWS(singleton::system.request({}));
	REQUIRE_THROWS(singleton::system.request({"x"}));
	REQUIRE_THROWS(singleton::system.request({"0"}));
	REQUIRE_THROWS(singleton::system.request({"-1", "1"}));
	REQUIRE_THROWS(singleton::system.request({std::to_string(ResourceID::MAX_EXCLUSIVE), "1"}));
	REQUIRE_THROWS(singleton::system.request({"0.5", "1"}));
	singleton::system.create({"0"}); // Process 0 can't request
	singleton::system.timeout({});
	REQUIRE_THROWS(singleton::system.request({"0", "0"})); // Request 0 units

	// Valid input
	REQUIRE_NOTHROW(singleton::system.request({"0", "1"}));
}

TEST_CASE("release() input")
{
	singleton::system.init({});

	// Invalid inputs
	REQUIRE_THROWS(singleton::system.release({}));
	REQUIRE_THROWS(singleton::system.release({"x"}));
	REQUIRE_THROWS(singleton::system.release({"0"}));
	REQUIRE_THROWS(singleton::system.release({"-1", "1"}));
	REQUIRE_THROWS(singleton::system.release({std::to_string(ResourceID::MAX_EXCLUSIVE), "1"}));
	REQUIRE_THROWS(singleton::system.release({"0.5", "1"}));
	REQUIRE_THROWS(singleton::system.release({"0", "0"})); // Release 0 units

	// Valid input
	singleton::system.create({"0"});
	singleton::system.timeout({});
	REQUIRE_NOTHROW(singleton::system.request({"0", "1"}));
	REQUIRE_NOTHROW(singleton::system.release({"0", "1"}));
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
			&& process.resources.size() == 0
			&& process.priority == 0
			&& process.id == index;
	};
	const auto index = std::views::iota(0U, processes.size());
	REQUIRE(std::ranges::all_of(std::views::zip(processes, index), isGood));

	const auto isResourceGood = [](const auto& pair)
	{
		const auto& [resource, index] = pair;
		return resource.state == RCB::State::Free
			&& resource.waitList.empty()
			&& resource.remain == unitMap[index]
			&& resource.id == index;
	};
	const auto resourceIndex = std::views::iota(0U, processes.size());
	REQUIRE(std::ranges::all_of(std::views::zip(resources, resourceIndex), isResourceGood));

	// Priority level
	REQUIRE(readyList[0].size() == 1); // Process 0
	REQUIRE(readyList[1].empty());
	REQUIRE(readyList[2].empty());
}

TEST_CASE("create() without timeout and priority scheduling")
{
	singleton::system.init({});
	const auto& processes = singleton::system.getProcesses();
	const auto& resources = singleton::system.getResources();
	const auto& readyList = singleton::system.getReadyList();
	auto outputCapture = OutputCapture{};
	outputCapture.capture();

	// Create with 5 processes at each level, assuming 14 processes [1, 14] exclude 0 and 3 levels
	// Level 0: [0, 1, 2, 3, 4] 1,2,3,4,5 childs of 0
	// Level 1: [5, 6, 7, 8, 9] 6,7,8,9,10 childs of 5. 11, 12, 13, 14 childs of 10
	// Level 2: [10, 11, 12, 13, 14], Exclude 15 because we only have 3 levels
	for (int i = 1, level = 0; i < ProcessID::MAX_EXCLUSIVE - 1; i++) // Only N - 1 processes can be create because process 0 is already running
	{
		level = i / 5;
		singleton::system.create({std::to_string(level)}); // Process at higher level will be switched with the current running lower level one
		REQUIRE(processes[i].state == PCB::State::Ready);
		if (i % 5 == 0) REQUIRE(processes[i].parent == readyList[level - 1].front()); // 5 or 10, scheduler will switch the running process at the start of level 1 and 2
		else REQUIRE(processes[i].parent == readyList[level].front());
		REQUIRE(processes[i].childs.size() == 0);
		REQUIRE(processes[i].resources.size() == 0);
		REQUIRE(processes[i].priority == level);
		REQUIRE(readyList[level].size() == (i % 5) + 1);
		REQUIRE(readyList[level].back() == i);
		REQUIRE(outputCapture.getOutput(1) == "process " + std::to_string(i) + " created");
		REQUIRE(outputCapture.getOutput() == "process " + std::to_string(level * 5) + " running");
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

	// Create process 1 and 2 at level 0
	singleton::system.create({"0"});
	singleton::system.create({"0"});
	REQUIRE(readyList[0] == std::list<ProcessID>{0, 1, 2});

	// Context switch from process 0 to 1, Level 0: 1 2 0
	singleton::system.timeout({});
	REQUIRE(readyList[0] == std::list<ProcessID>{1, 2, 0});
	REQUIRE(outputCapture.getOutput() == "process 1 running");

	// Context switch from process 1 to 2, Level 0: 2 0 1
	singleton::system.timeout({});
	REQUIRE(readyList[0] == std::list<ProcessID>{2, 0, 1});
	REQUIRE(outputCapture.getOutput() == "process 2 running");

	// Create process 3, 4, 5 at level 1
	singleton::system.create({"1"});
	singleton::system.create({"1"});
	singleton::system.create({"1"});
	REQUIRE(readyList[1] == std::list<ProcessID>{3, 4, 5});

	singleton::system.timeout({});
	REQUIRE(readyList[1] == std::list<ProcessID>{4, 5, 3});
	REQUIRE(outputCapture.getOutput() == "process 4 running");

	singleton::system.timeout({});
	REQUIRE(readyList[1] == std::list<ProcessID>{5, 3, 4});
	REQUIRE(outputCapture.getOutput() == "process 5 running");
}

TEST_CASE("create()/destroy() with timeout")
{
	singleton::system.init({});
	const auto& processes = singleton::system.getProcesses();
	const auto& resources = singleton::system.getResources();
	const auto& readyList = singleton::system.getReadyList();
	auto outputCapture = OutputCapture{};
	outputCapture.capture();

	// Create process 1 and 2, childs of 0, L0: 0, 1, 2
	singleton::system.create({"0"});
	singleton::system.create({"0"});

	// Create process 3, 4, 5, L1: 3, 4, 5.
	// 3 is child of 0. 4, 5 are childs of 3
	singleton::system.create({"1"});
	singleton::system.create({"1"});
	singleton::system.create({"1"});
	// Current running process is 3

	// Switch to 4, create childs 6 and 7, L1: 4, 5, 3, 6, 7
	singleton::system.timeout({});
	singleton::system.create({"1"});
	REQUIRE(outputCapture.getOutput(1) == "process 6 created");
	singleton::system.create({"1"});
	REQUIRE(outputCapture.getOutput(1) == "process 7 created");
	REQUIRE(readyList[1] == std::list<ProcessID>{4, 5, 3, 6, 7});
	REQUIRE(processes[6].state == PCB::State::Ready);
	REQUIRE(processes[7].state == PCB::State::Ready);
	REQUIRE(processes[6].parent == 4);
	REQUIRE(processes[7].parent == 4);
	REQUIRE(processes[4].childs == std::vector<ProcessID>{6, 7});

	// Switch to 5, create child 8 at L2, L2: 8. L1: 5, 3, 6, 7, 4
	singleton::system.timeout({});
	singleton::system.request({"3", "3"}); // Request so we can block process 8 and goes back to process 5
	singleton::system.create({"2"});
	REQUIRE(readyList[2] == std::list<ProcessID>{8});
	REQUIRE(processes[8].state == PCB::State::Ready);
	REQUIRE(processes[8].parent == 5);
	REQUIRE(processes[8].childs.empty());
	singleton::system.request({"3", "3"}); // Block process 8 and goes back to process 5

	// Destroy only works for current process and its child, otherwise must throw
	for (int i = 0; i < 9; i++)
	{
		if (i == 5 || i == 8) continue;
		REQUIRE_THROWS(singleton::system.destroy({std::to_string(i)}));
	}

	// Destroy process 5. L1: 3, 6, 7, 4. Process 4 owns 6 and 7
	singleton::system.destroy({"5"});
	REQUIRE(outputCapture.getOutput(1) == "2 processes destroyed");
	REQUIRE(readyList[1] == std::list<ProcessID>{3, 6, 7, 4});
	REQUIRE(processes[8].parent == std::nullopt);
	REQUIRE(processes[8].state == PCB::State::Free);
	REQUIRE(processes[5].childs.empty());
	REQUIRE(processes[4].childs == std::vector<ProcessID>{6, 7});

	// Destroy process 0, L0: 0, 1, 2. L1: 3, 6, 7, 4
	// Block all L1 to get to process 0
	singleton::system.request({"0", "1"});
	singleton::system.timeout({});
	singleton::system.request({"1", "1"});
	singleton::system.request({"0", "1"});
	singleton::system.request({"0", "1"});
	singleton::system.request({"0", "1"});
	singleton::system.request({"1", "1"}); // Deadlock between 4 and 3 (owning and blocked) -> get to process 0

	singleton::system.destroy({"0"});
	REQUIRE(outputCapture.getOutput() == "7 processes destroyed");
	REQUIRE(processes[0].childs.size() == 0);
	for (int i = 0; i < ProcessID::MAX_EXCLUSIVE; i++)
	{
		REQUIRE(processes[i].state == PCB::State::Free);
		REQUIRE(processes[i].childs.empty());
		REQUIRE(processes[i].parent == std::nullopt);
		REQUIRE(processes[i].resources.empty());
		REQUIRE(processes[i].priority == 0);
	}
	for (int i = 0; i < ResourceID::MAX_EXCLUSIVE; i++)
	{
		REQUIRE(resources[i].remain == unitMap[i]);
		REQUIRE(resources[i].state == RCB::State::Free);
		REQUIRE(resources[i].waitList.empty());
	}
	for (const auto& list : readyList) REQUIRE(list.empty());

	// Destroy free process check
	REQUIRE_THROWS(singleton::system.destroy({"0"}));
	REQUIRE_THROWS(singleton::system.destroy({std::to_string(ProcessID::MAX_EXCLUSIVE - 1)}));
}

TEST_CASE("request()/release() with timeout")
{
	const auto& processes = singleton::system.getProcesses();
	const auto& resources = singleton::system.getResources();
	const auto& readyList = singleton::system.getReadyList();
	auto outputCapture = OutputCapture{};
	outputCapture.capture();

	// Request resource when there isn't any running process
	singleton::system.init({});
	singleton::system.destroy({"0"});
	REQUIRE_THROWS(singleton::system.request({"0", "1"}));
	REQUIRE_THROWS(singleton::system.request({std::to_string(ResourceID::MAX_EXCLUSIVE - 1)}));

	singleton::system.init({});
	REQUIRE_THROWS(singleton::system.request({"0", "1"})); // Process 0 can't request

	// Create processes to match the number of resources' unit (assuming 7 including process 0)
	// Resource 3: 3 units
	// Resource 2: 2 units
	// Resource 1: 1 units
	// Resource 0: 1 units
	const auto resourceMap = std::array<std::pair<ResourceID, Units>, 8>{
		std::pair{-1, -1}
			, {0, 1}
			, {1, 1}
			, {2, 1}
			, {2, 1}
			, {3, 1}
			, {3, 1}
			, {3, 1}
	};
	for (int i = 1; i < 9; i++)
	{
		singleton::system.create({std::to_string(i / 3)});
		if (i == 8) continue; // Skip the last process for testing with the resource and blocking
		for (int j = 0; j < (i % 3); j++) singleton::system.timeout({}); // Context switch within level i / 3 to reach process i
		const auto& [resource, units] = resourceMap[i];
		REQUIRE_NOTHROW(singleton::system.request({std::to_string(resource), std::to_string(units)}));
		REQUIRE_THROWS(singleton::system.request({std::to_string(resource), std::to_string(units)})); // Requesting the same owned resource for the running process
		REQUIRE(processes[i].state == PCB::State::Ready); // Check the running process isn't blocked if requesting the same resource
		REQUIRE(processes[i].resources.size() == 1);
		REQUIRE(processes[i].resources.front() == resourceMap[i]);
		REQUIRE(resources[resource].state == RCB::State::Allocated);
		REQUIRE(outputCapture.getOutput() == std::to_string(units) + " units of resource " + std::to_string(resource) + " allocated");
	}

	// Parent->Child: 0 -> 1 -> 2 -> 3 -> 4 -> 5 -> 6 -> 7 -> 8
	REQUIRE(readyList[2] == std::list<ProcessID>{7, 6, 8});
	REQUIRE(readyList[1] == std::list<ProcessID>{5, 4, 3});
	REQUIRE(readyList[0] == std::list<ProcessID>{2, 1, 0});
	// Sanity check
	for (int i = 1; i < 8; i++) assert(processes[i].childs == std::vector<ProcessID>{i + 1}); // Process 8 is a leaf process for testing

	// Switch to and block process 8
	singleton::system.timeout({});
	singleton::system.timeout({});
	singleton::system.request({"3", "1"}); // Block 8
	REQUIRE(processes[8].state == PCB::State::Blocked);
	REQUIRE(resources[3].waitList == std::list{std::pair<ProcessID, Units>{8, 1}});
	REQUIRE(outputCapture.getOutput(1) == "process 8 blocked");
	REQUIRE(outputCapture.getOutput() == "process 7 running");

	// Process 7 releases resource 3 with 1 unit, process 8 unblocked and owns resource 3 with 1 unit
	singleton::system.release({"3", "1"});
	REQUIRE_THROWS(singleton::system.release({"3", "1"})); // Release the same resource check
	REQUIRE(processes[8].state == PCB::State::Ready);
	REQUIRE(resources[3].waitList.empty());
	REQUIRE(readyList[2] == std::list<ProcessID>{7, 6, 8});
}
// ADVANCE ============

//// RANDOM ============
TEST_CASE("random one")
{
	singleton::system.init({});
	const auto& processes = singleton::system.getProcesses();
	const auto& resources = singleton::system.getResources();
	const auto& readyList = singleton::system.getReadyList();
	auto outputCapture = OutputCapture{};
	outputCapture.capture();

	singleton::system.create({"0"});
	singleton::system.create({"0"});
	singleton::system.create({"0"});

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

	singleton::system.create({"0"}); // child 1 for process 0
	singleton::system.create({"0"}); // child 2 for process 0
	singleton::system.timeout({});
	singleton::system.request({"0", "1"}); // resource 0 for process 1

	singleton::system.timeout({}); // switch to process 2
	singleton::system.request({"0", "1"}); // process 2 blocked, process 0 running
	singleton::system.timeout({}); // switch to process 1

	singleton::system.release({"0", "1"}); // unblock process 2, now owns resource 0 and 1 unit, process 1 still running
	singleton::system.timeout({});
	singleton::system.timeout({}); // switch to process 2
	REQUIRE_THROWS(singleton::system.request({"0", "1"})); // request owning resource
}

TEST_CASE("random three")
{
	singleton::system.init({});
	const auto& processes = singleton::system.getProcesses();
	const auto& resources = singleton::system.getResources();
	const auto& readyList = singleton::system.getReadyList();
	auto outputCapture = OutputCapture{};
	outputCapture.capture();

	singleton::system.create({"0"});
	singleton::system.timeout({});
	singleton::system.request({"3", "3"});
	singleton::system.create({"1"});
	singleton::system.create({"1"});
	singleton::system.create({"1"});
	singleton::system.request({"3", "1"}); // blocked
	singleton::system.request({"3", "1"}); // blocked
	singleton::system.request({"3", "1"}); // blocked
	// back to process 1
	REQUIRE_NOTHROW(singleton::system.release({"3", "3"}));
	REQUIRE(readyList[1] == std::list<ProcessID>{2, 3, 4});
}

TEST_CASE("random four")
{
	singleton::system.init({});
	const auto& processes = singleton::system.getProcesses();
	const auto& resources = singleton::system.getResources();
	const auto& readyList = singleton::system.getReadyList();
	auto outputCapture = OutputCapture{};
	outputCapture.capture();

	singleton::system.create({"0"});
	singleton::system.timeout({});
	singleton::system.request({"3", "1"});
	REQUIRE_THROWS(singleton::system.release({"3", "2"}));
	REQUIRE_THROWS(singleton::system.request({"3", "4"}));
	REQUIRE_THROWS(singleton::system.request({"3", "1.5"}));
	REQUIRE_THROWS(singleton::system.release({"3", "1.5"}));
	singleton::system.release({"3", "1"});
	singleton::system.request({"3", "3"});
	REQUIRE_NOTHROW(singleton::system.release({"3", "2"}));
	REQUIRE_NOTHROW(singleton::system.release({"3", "1"}));
	REQUIRE(processes[1].resources.empty());
}

// RANDOM ============
// ============ SYSTEM ============

// ============ SHELL ============
TEST_CASE("Shell instantiation")
{
	REQUIRE(Shell::isInstantiated);
}
// ============ SHELL ============


// ############## TODO ##################
// Accumulated request from the same process -> accumulate into one pair {resource, }

//process 0 request {3, 1} then do the same thing again -> eror? accumulate into one release or multiple release?
//	
//• number of units requested + number already held <= initial inventory ==> error
//• number of units released <= number of units currently held ==> error
//	
//TODO: Delete process must release any resource that it held
//TODO: timeout the only running process? timeout the only highest level running process and there are a lots of lower level processes?
//TODO: test fraction within boundary (2.5) for create, release and request priority/id
//	
//Functions must implement checks to detect illegal/unexpected operations
//• Examples:
//• Creating more than n processes
//• Destroying a process that is not a child of the current process
//• Requesting a nonexistent resource
//• Requesting a resource the process is already holding
//• Releasing a resource the process is not holding
//• Process 0 should be prevented from requesting any resource to avoid 
//deadlock where no process is on the RL
//• In each case, the corresponding function should display “error” (e.g. -1
//
//
//Test case
//	cr 0
//	to
//	rq 3 3
//	cr 1
//	rq 3 2
//	rl 3 1
//	rl 3 1 --> error buts it not
//
//Test case
//	> cr 0
//	process 1 created
//	process 0 running
//	> to
//	process 1 running
//	> rq 3 3
//	3 units of resource 3 allocated
//	> cr 1
//	process 2 created
//	process 2 running
//	> cr 1
//	process 3 created
//	process 2 running
//	> cr 1
//	process 4 created
//	process 2 running
//	> rq 3 1
//	process 2 blocked
//	process 3 running
//	> rq 3 1
//	process 3 blocked
//	process 4 running
//	> rq 3 1
//	process 4 blocked
//	process 1 running
//	> rl 3 3
//	3 units of resource 3 released
//	process 2 running
//	> to
//	process 3 running
//	> to
//	process 4 running
//	> to
//	process 2 running
//	>
//
//Repete the above test case with delete process 1
//
//TESt case destroy blocked process
//	rq 3 3
//	cr 1
//	to
//	rq 3 1
//	de 1
//
//test case destroy a process with blocked childs
//	rq 3 3
//	cr 1
//	cr 1
//	cr 1
//	cr 1
//	to
//	rq 3 1
//	rq 3 1
//	rq 3 1
//	de 1
//
//
//// TODO: release a unit of the resource 3 only, so its state shouln't be free when the waitlist is empty?
//// TODO: or force a complete release of the resource and the number of units must match the number of owned units?
//
//https://slideplayer.com/slide/3334835/









#pragma once

#include <vector>
#include <cassert>
#include <optional>
#include <array>

using Units = uint32_t;
constexpr auto unitMap = std::array<Units, ResourceID::MAX_EXCLUSIVE>{1, 1, 2, 3}; // Map Resource ID to Units, stores inventory for each resource ID. ie: ResourceID 2 == Index 2 has at most 2 units

struct PCB // Process
{
	enum class State : uint8_t {Free, Ready, Blocked};

	PCB() :
	state{State::Free}
	, parent{std::nullopt}
	, childs{}
	, resources{}
	, priority{0}
	, id{0}
	{}

	~PCB(){} 

	State state;
	std::optional<ProcessID> parent;
	std::vector<ProcessID> childs;
	std::vector<std::pair<ResourceID, Units>> resources;
	PriorityID priority;
	ProcessID id;
};



#pragma once

#include <vector>
#include <cassert>
#include <optional>

struct PCB // Process
{
	enum class State : uint8_t {Free, Ready, Blocked};

	PCB() :
	state{State::Free}
	, parent{std::nullopt}
	, childs{}
	, resources{}
	, priority{0}
	, id{}
	{}

	~PCB(){} 

	State state;
	std::optional<ProcessID> parent;
	std::vector<ProcessID> childs;
	std::vector<ResourceID> resources;
	Priority priority;
	ProcessID id;
};



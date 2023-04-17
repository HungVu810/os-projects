#pragma once

#include <list>
#include <cassert>
#include <array>

struct RCB // Resource
{
	enum class State : uint8_t {Free, Allocated};

	RCB() :
	state{State::Free}
	, id{0}
	, remain{}
	, waitList{}
	{}

	~RCB(){};

	State state;
	ResourceID id;
	Units remain;
	std::list<std::pair<ProcessID, Units>> waitList; // Blocked processes waiting for this resource
};

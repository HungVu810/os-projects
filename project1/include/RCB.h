#pragma once

#include <list>
#include <cassert>

struct RCB // Resource
{
	enum class State : uint8_t {Free, Allocated};

	RCB() :
	state{State::Free}
	, waitList{}
	, id{}
	{}

	~RCB(){};

	State state;
	std::list<ProcessID> waitList; // Blocked processes waiting for this resource
	ResourceID id;
};

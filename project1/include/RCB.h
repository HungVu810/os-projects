#pragma once

#include <queue>

struct RCB // Resource
{
	enum class State : uint8_t {Free, Allocated};

	RCB() : state{State::Free}, waitList{}{};

	~RCB(){};

	State state;
	std::queue<ProcessIndex> waitList; // Blocked processes waiting for this resource
};

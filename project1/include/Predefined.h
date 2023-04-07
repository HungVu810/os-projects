#pragma once
#include <cassert>

// using ProcessID = uint32_t;
struct ProcessID
{
	uint32_t id;
	static const uint32_t MAX_EXCLUSIVE = 16; // [0, MAX_EXCLUSIVE]
	operator uint32_t() const noexcept {return id;}
	ProcessID& operator=(uint32_t inID)
	{
		assert(inID >= 0 && inID < MAX_EXCLUSIVE);
		id = inID;
		return *this;
	}
};

// using ResourceID = uint32_t;
struct ResourceID
{
	uint32_t id;
	static const uint32_t MAX_EXCLUSIVE = 4;
	operator uint32_t() const noexcept {return id;}
	ResourceID& operator=(uint32_t inID)
	{
		assert(inID >= 0 && inID < MAX_EXCLUSIVE);
		id = inID;
		return *this;
	}
};

using Priority = uint8_t;
constexpr uint32_t NUM_PRIORITY = 3;



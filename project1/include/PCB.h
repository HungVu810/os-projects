#pragma once

#include <vector>

// These are used later on by RCB, System and Shell headers
using ProcessIndex = uint32_t;
using ResourceIndex = uint32_t;
class System; // Forwarded declaration 
using CommandFunction = std::function<void(System&, std::vector<std::string>)>;

struct RCB; // Forwarded declaration

struct PCB // Process
{
	enum class State : uint8_t {New, Ready, Running, Blocked};

	PCB() :
	state{State::New}
	, parent{std::numeric_limits<ProcessIndex>::max()}
	, childs{}
	, resources{}
	, priority{0}
	{}

	~PCB(){} 

	State state;
	ProcessIndex parent;
	std::vector<ProcessIndex> childs;
	std::vector<ResourceIndex> resources;
	uint8_t priority; // TODO: Priority is [0, 2]
};
#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <functional>
#include <exception>
#include <sstream>
#include <tuple>
#include <optional>
#include <cassert>
#include <variant>
#include <cerrno>
#include "process.h"

using ProcessIndex = int;
using ResourceIndex = int;
class System;
using CommandFunction = std::function<void(System&, std::vector<std::string>)>;

namespace
{
	std::unordered_map<std::string, CommandFunction> getCommandMap() noexcept;
}

struct RCB; // Forward declare
struct PCB // Process
{
	enum class State : uint8_t {Ready, Running, Blocked};

	PCB() :
	state{State::Ready}
	, parent{-1}
	, childs{}
	, resources{}
	, priority{0}
	{}

	~PCB()
	{

	}

	State state;
	ProcessIndex parent;
	std::vector<ProcessIndex> childs;
	std::vector<ResourceIndex> resources;
	uint8_t priority; // TODO: 0, 1, 2
};

struct RCB // Resource
{
	enum class State : uint8_t {Free, Allocated};

	RCB() : state{State::Free}, waitList{}{};

	~RCB(){};

	State state;
	std::queue<ProcessIndex> waitList; // Blocked processes waiting for this resource
};

class System // Singleton
{
	public:
		~System(){};

		void create(const std::vector<std::string>& arguments)
		{
			if (arguments.size() > 0) throw std::runtime_error{"Invalid number of arguments."};
			//readyList.push_back(PCB{});
			//const auto numProcess = readyLists.size() + waitList.size() - 1;
			//std::cout << "process " << numProcess << " created\n";
		}

		void destroy(const std::vector<std::string>& arguments)
		{
			if (arguments.size() > 1) throw std::runtime_error{"Invalid number of arguments."};
			uint32_t process = strtoul(arguments.front().data(), nullptr, 0);
			if (errno == ERANGE) throw std::runtime_error{"Failed to convert argument to uint32_t."};
			// auto processIter = 
			// destroy process 0?
		}

		void request(const std::vector<std::string>& arguments)
		{
			if (arguments.size() > 1) throw std::runtime_error{"Invalid number of arguments."};
			uint32_t process = strtoul(arguments.front().data(), nullptr, 0);
			if (errno == ERANGE) throw std::runtime_error{"Failed to convert argument to uint32_t."};

		}

		void release(const std::vector<std::string>& arguments)
		{
			if (arguments.size() > 1) throw std::runtime_error{"Invalid number of arguments."};
			uint32_t process = strtoul(arguments.front().data(), nullptr, 0);
			if (errno == ERANGE) throw std::runtime_error{"Failed to convert argument to uint32_t."};

		}

		void timeout(const std::vector<std::string>& arguments)
		{
			if (arguments.size() > 0) throw std::runtime_error{"Invalid number of arguments."};

		}

		void init(const std::vector<std::string>& arguments)
		{
			if (arguments.size() > 0) throw std::runtime_error{"Invalid number of arguments."};
			//readyList.clear();
			//readyList.push_back(PCB{});
			//waitList.clear();
			//std::cout << "process 0 running\n";
		}

		static auto getInstance()
		{
			if (!isInstantiated)
			{
				isInstantiated = true;
				return System{};
			}
			else assert(false); // Programmer's error
		};

		static bool isInstantiated;

	private:
		System() : readyList{}, resources{}{};

		std::vector<PCB> processes;
		std::vector<RCB> resources;
		std::queue<ProcessIndex> readyList; // Current running process is at the head of the readyList
};
bool System::isInstantiated = false;

class Shell // Singleton
{
	public:
		~Shell(){};

		void run(System& system)
		{
			// preProcessing();
			preRead();
			auto command = readCommand();
			auto tokens = parseCommand(command);
			runCommand(tokens, system);
			// postProcessing();
		}

		static auto getInstance()
		{
			if (!isInstantiated)
			{
				isInstantiated = true;
				return Shell{};
			}
			else assert(false); // Programmer's error
		};

		static bool isInstantiated;

		static const uint32_t maxArgs;

	private:
		Shell() : commandMap{getCommandMap()}{};

		void preRead()
		{
			std::cout << "> ";
		}

		std::string readCommand() const
		{
			auto command = std::string{};
			std::cin >> command;
			return command;
		}

		std::vector<std::string> parseCommand(std::string command)
		{
			auto commandStream = std::istringstream{command};
			auto tokens = std::vector<std::string>{};
			auto token = std::string{};
			while (commandStream >> token) // commandStream's delim is space, use getline() if \n is delim
			{
				if (tokens.size() == maxArgs) throw std::runtime_error{"Invalid number of arguments."};
				tokens.push_back(token);
			}
			return tokens; // tokens == {command, [argument1, argument2, ...]}
		}

		void runCommand(const std::vector<std::string>& tokens, System& system) const
		{
			const auto iterPair = commandMap.find(tokens.front());
			if (iterPair == commandMap.end()) throw std::runtime_error{"Invalid command."};
			const auto& [command, function] = *iterPair;
			auto iterArg = tokens.begin();
			std::advance(iterArg, 1); // argument1
			const auto args = std::vector<std::string>(iterArg, tokens.end());
			function(system, args);
		}

		std::unordered_map<std::string, CommandFunction> commandMap;
};
bool Shell::isInstantiated = false;
const uint32_t Shell::maxArgs = 2; // Maximum number of arguments including the command

namespace
{
	std::unordered_map<std::string, CommandFunction> getCommandMap() noexcept
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


int main() // Accept arguments
{
	while (true)
	{
		try
		{
			auto system = System::getInstance();
			auto shell = Shell::getInstance();
			shell.run(system);
		}
		catch (const std::runtime_error& error)
		{
			std::cout << "* error\n";
		}
	}
}



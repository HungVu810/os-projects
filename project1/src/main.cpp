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
#include "process.h"

namespace
{
	inline auto requestID() noexcept
	{
		static uint32_t id = 0; // Maximum 2^32 id
		return id++;
	}
	void create(const std::vector<std::string>& arguments)
	{

	}
	void destroy(const std::vector<std::string>& arguments)
	{

	}
	void request(const std::vector<std::string>& arguments)
	{

	}
	void release(const std::vector<std::string>& arguments)
	{

	}
	void timeout(const std::vector<std::string>& arguments)
	{

	}
	void init(const std::vector<std::string>& arguments)
	{

	}
	inline auto getCommandMap()
	{
		return std::unordered_map<std::string, std::function<void(std::vector<std::string>)>>{
			{"cr", create}
			, {"de", destroy}
			, {"rq", request}
			, {"rl", release}
			, {"to", timeout}
			, {"in", init}
		};
	}
}

struct RCB; // Forward declare

struct PCB // Process
{
	enum class State : uint8_t {Ready, Running, Blocked};

	PCB() :
	id{requestID()}
	, state{State::Ready}
	, parent{nullptr}
	, childs{}
	, resources{}
	, priority{0}
	{}

	~PCB()
	{

	}

	void create() noexcept // Create a new child
	{

	}

	//void destroy(PCB& process)
	//{
	//	if (process == *this)
	//	{

	//	}
	//}


	uint32_t id;
	State state;
	std::unique_ptr<PCB> parent;
	std::vector<PCB> childs;
	std::vector<RCB> resources;
	uint32_t priority;
};

struct RCB // Resource
{
	enum class State : uint8_t {Free, Allocated};

	RCB() : state{State::Free}, waitList{}{};

	~RCB(){};

	State state;
	std::queue<PCB> waitList; // Blocked processes waiting for this resource
};

class System // Singleton
{
	public:
		~System(){};

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

		std::queue<PCB> readyList;
		std::vector<RCB> resources;
		//std::vector<PCB> runList; // Current running process
};
bool System::isInstantiated = false;

class Shell // Singleton
{
	public:
		~Shell(){};

		void run()
		{
			// preProcessing();
			preRead();
			auto command = readCommand();
			auto tokens = parseCommand(command);
			runCommand(tokens);
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
			return tokens;
		}

		void runCommand(const std::vector<std::string>& tokens) const
		{
			const auto iterPair = commandMap.find(tokens.front());
			if (iterPair == commandMap.end()) throw std::runtime_error{"Invalid command."};
			const auto& [command, function] = *iterPair;
			// tokens == {command, [argument1, argument2, ...]}
			auto iterArg = tokens.begin();
			std::advance(iterArg, 1); // argument1
			function(std::vector<std::string>(iterArg, tokens.end()));
		}

		std::unordered_map<std::string, std::function<void(std::vector<std::string>)>> commandMap;
};
bool Shell::isInstantiated = false;
const uint32_t Shell::maxArgs = 2; // Maximum number of arguments including the command


int main() // Accept arguments
{
	std::cout << "hello";
	//while (true)
	//{
	//	try
	//	{
	//		auto system = System::getInstance();
	//		auto shell = Shell::getInstance();
	//		shell.run();
	//	}
	//	catch (const std::runtime_error& error)
	//	{
	//		std::cout << "* error\n";
	//	}
	//}
}



#pragma once

#include <unordered_map>
#include <sstream>
#include <cassert>
#include <iostream>

using CommandFunction = std::function<void(System&, std::vector<std::string>)>;

namespace
{
	[[nodiscard]] std::unordered_map<std::string, CommandFunction> getCommandMap(); // Forwarded declaration
}

class Shell // Singleton
{
	public:
		~Shell(){};

		void run(System& system)
		{
			preRead();
			auto command = readCommand();
			auto tokens = parseCommand(command);
			runCommand(tokens, system);
		}

		[[nodiscard]] static auto getInstance()
		{
			if (!isInstantiated)
			{
				isInstantiated = true;
				return Shell{};
			}
			else assert(false); // Programmer's error
		};

		static bool isInstantiated;

	private:
		Shell() : commandMap{getCommandMap()}{};

		void preRead()
		{
			std::cout << "> ";
		}

		[[nodiscard]] std::string readCommand() const
		{
			auto command = std::string{};
			std::getline(std::cin, command);
			return command;
		}

		[[nodiscard]] std::vector<std::string> parseCommand(std::string command)
		{
			auto commandStream = std::istringstream{command};
			auto tokens = std::vector<std::string>{};
			auto token = std::string{};
			while (commandStream >> token) // commandStream's delim is space, use getline() if \n is delim
			{
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


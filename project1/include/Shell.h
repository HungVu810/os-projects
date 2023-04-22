#pragma once

#include <unordered_map>
#include <sstream>
#include <cassert>
#include <iostream>

//#ifdef _NDEBUG
//	constexpr auto printExceptionMsg = true;
//#else
//	constexpr auto printExceptionMsg = false;
//#endif

namespace
{
	using CommandFunction = std::function<void(System&, std::vector<std::string>)>;

	[[nodiscard]] std::unordered_map<std::string, CommandFunction> getCommandMap()
	{
		return {
			{"cr", std::mem_fn(&System::create)}
			, {"de", std::mem_fn(&System::destroy)}
			, {"rq", std::mem_fn(&System::request)}
			, {"rl", std::mem_fn(&System::release)}
			, {"to", std::mem_fn(&System::timeout)}
			, {"in", std::mem_fn(&System::init)}
			//, {"pp", std::mem_fn(&System::printProcesses)}
			//, {"pr", std::mem_fn(&System::printResources)}
			//, {"ppi", std::mem_fn(&System::printProcess)}
			//, {"pri", std::mem_fn(&System::printResource)}
			//, {"pl", std::mem_fn(&System::printReadyList)}
		};
	}
}

class Shell // Singleton
{
	public:
		~Shell(){};

		void run(System& system)
		{
			while (true)
			{
				try
				{
					preRead();
					auto command = readCommand();
					auto tokens = parseCommand(command);
					runCommand(tokens, system);
					std::cout << system.getRunningProcess() << '\n';
				}
				catch (const std::runtime_error& error)
				{
					// const auto prompt = std::string_view{"* error"};
					const auto prompt = std::string_view{error.what()};
					std::cout << prompt << '\n';
				}
			}
		}

		void run(System& system, std::string_view filePath)
		{
			const auto inputPath = std::filesystem::path{filePath};
			auto inputFile = std::ifstream{inputPath};
			if (!inputFile) throw std::runtime_error{"Invalid input file."};

			auto outputFile = std::ofstream{inputPath.parent_path()/"output.txt"}; // Create an output file, std::fstream{path, std::ios::out};

			auto command = std::string{};
			while (std::getline(inputFile, command) || !inputFile.eof())
			{
				if (command.empty()) outputFile << '\n'; // Blank space seperating the input sequences
				else
				{
					auto outputContent = std::stringstream{};
					try
					{
						auto tokens = parseCommand(command);
						runCommand(tokens, system);
						outputContent << system.getRunningProcess();
					}
					catch (const std::runtime_error& error)
					{
						outputContent << -1;
					}
					outputFile << outputContent.str() << ' ';
				}
			}
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


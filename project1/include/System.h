#pragma once

#include <vector>
#include <queue>
#include <cassert>
#include <cerrno>

namespace
{
	auto inline toProcessIndex(std::string_view string)
	{
		const auto processIndex = ProcessIndex{std::atoi(string.data())};
		if (errno == ERANGE) throw std::runtime_error{"Failed to convert argument to uint32_t."};
		return processIndex;
	}
	void inline checkArgumentSize(const std::vector<std::string>& arguments, size_t desiredSize)
	{
		if (arguments.size() != desiredSize) throw std::runtime_error{"Invalid number of arguments."};
	}
}


class System // Singleton
{
	public:
		~System(){};

		void create(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 0);
			processes.push_back(PCB{});
			//readyList.push_back(PCB{});
			//const auto numProcess = readyLists.size() + waitList.size() - 1;
			//std::cout << "process " << numProcess << " created\n";
		}

		void destroy(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 1);
			const auto processIndex = toProcessIndex(arguments.front());
			// destroy process 0?
		}

		void request(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 1);
			const auto processIndex = toProcessIndex(arguments.front());

		}

		void release(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 1);
			const auto processIndex = toProcessIndex(arguments.front());

		}

		void timeout(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 0);

		}

		void init(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 0);
			//readyList.clear();
			//readyList.push_back(PCB{});
			//waitList.clear();
			//std::cout << "process 0 running\n";
		}

		[[nodiscard]] static auto getInstance()
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


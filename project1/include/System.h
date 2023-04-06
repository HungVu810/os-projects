#pragma once

#include <vector>
#include <queue>
#include <cassert>
#include <cerrno>
#include <array>
#include <string>

constexpr uint32_t NUM_PROCESS = 16;
constexpr uint32_t NUM_RESOURCE = 4;

namespace
{
	auto inline toProcessIndex(std::string_view string)
	{
		try
		{
			const auto maybeUnsignedInt = std::stoi(string.data());
			if (maybeUnsignedInt < 0 || maybeUnsignedInt >= NUM_PROCESS) throw std::runtime_error{"Invalid process index."};
			return static_cast<ProcessIndex>(maybeUnsignedInt);
		}
		catch (...)
		{
			throw std::runtime_error{"Failed to convert string to int."};
		}
	}
	auto inline toResourceIndex(std::string_view string)
	{
		try
		{
			const auto maybeUnsignedInt = std::stoi(string.data());
			if (maybeUnsignedInt < 0 || maybeUnsignedInt >= NUM_RESOURCE) throw std::runtime_error{"Invalid resource index."};
			return static_cast<ResourceIndex>(maybeUnsignedInt);
		}
		catch (...)
		{
			throw std::runtime_error{"Failed to convert string to int."};
		}
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
			std::cout << "create() called\n";
			checkArgumentSize(arguments, 0);
			//processes.push_back(PCB{});
			//readyList.push_back(PCB{});
			//const auto numProcess = readyLists.size() + waitList.size() - 1;
			//std::cout << "process " << numProcess << " created\n";
		}

		void destroy(const std::vector<std::string>& arguments)
		{
			std::cout << "destroy() called\n";
			checkArgumentSize(arguments, 1);
			const auto processIndex = toProcessIndex(arguments.front());
			// destroy process 0?
		}

		void request(const std::vector<std::string>& arguments)
		{
			std::cout << "request() called\n";
			checkArgumentSize(arguments, 1);
			const auto processIndex = toResourceIndex(arguments.front());

		}

		void release(const std::vector<std::string>& arguments)
		{
			std::cout << "release() called\n";
			checkArgumentSize(arguments, 1);
			const auto processIndex = toResourceIndex(arguments.front());

		}

		void timeout(const std::vector<std::string>& arguments)
		{
			std::cout << "timeout() called\n";
			checkArgumentSize(arguments, 0);

		}

		void init(const std::vector<std::string>& arguments)
		{
			std::cout << "init() called\n";
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
		System() : processes{}, resources{}, readyList{}{};

		inline ProcessIndex getRunningProcess()
		{
			return readyList.front();
		}

		void scheduler()
		{

		}

		std::array<PCB, NUM_PROCESS> processes;
		std::array<RCB, NUM_RESOURCE> resources;
		std::queue<ProcessIndex> readyList; // Current running process is at the head of the readyList
		// TODO: size of readyList is always NUM_PROCESS - 1, process 0 excluded
		// std::array<std::queue<ProcessIndex>, 3>
};
bool System::isInstantiated = false;


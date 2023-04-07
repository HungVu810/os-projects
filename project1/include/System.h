#pragma once

#include <vector>
#include <queue>
#include <cassert>
#include <cerrno>
#include <array>
#include <string>
#include <ranges>

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
		//try
		//{
			const auto maybeUnsignedInt = std::stoi(string.data());
			if (maybeUnsignedInt < 0 || maybeUnsignedInt >= NUM_RESOURCE) throw std::runtime_error{"Invalid resource index."};
			return static_cast<ResourceIndex>(maybeUnsignedInt);
		//}
		//catch (...)
		//{
		//	throw std::runtime_error{"Failed to convert string to int."};
		//}
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
			const auto processIndex = getNewProcessIndex();
			setProcessReady(processIndex);
			std::cout << "process " << std::to_string(processIndex) << " created\n";
		}

		void destroy(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 1);
			const auto processIndex = toProcessIndex(arguments.front());
			// destroy process 0?
			// destroy
			//assert(processes[processIndex].childs.empty());
			//assert(processes[processIndex].parent == std::numeric_limits<ProcessIndex>::max());
			//assert(processes[processIndex].state == PCB::State::New);
		}

		void request(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 1);
			const auto resourceIndex = toResourceIndex(arguments.front());

		}

		void release(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 1);
			const auto resourceIndex = toResourceIndex(arguments.front());

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

		// For testing


	private:
		System() : processes{}, resources{}, readyList{}{};

		inline ProcessIndex getRunningProcess()
		{
			return readyList.front();
		}

		inline ProcessIndex getNewProcessIndex()
		{
			const auto toState = [](const PCB& process)
			{
				return process.state;
			};
			const auto iterProcess = std::ranges::find(processes, PCB::State::New, toState);
			if (iterProcess == processes.end()) throw std::runtime_error{"All of the processes are in used."};
			else return std::distance(processes.begin(), iterProcess);
		}

		inline void setProcessReady(ProcessIndex processIndex)
		{
			assert(processes[processIndex].state == PCB::State::New);
			processes[processIndex].state = PCB::State::Ready;
			readyList.push(processIndex);
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


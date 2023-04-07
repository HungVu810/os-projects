#pragma once

#include <vector>
#include <list>
#include <cassert>
#include <cerrno>
#include <array>
#include <string>
#include <concepts>
#include <ranges>

namespace
{
	template<typename T>
	concept TypeID = std::same_as<T, ProcessID> || std::same_as<T, ResourceID>;

	template<TypeID T>
	inline auto toID(std::string_view string)
	{
		try
		{
			const auto maybeUnsignedInt = std::stoi(string.data());
			if (maybeUnsignedInt < 0 || maybeUnsignedInt >= T::MAX_EXCLUSIVE) throw std::runtime_error{"Invalid process index."};
			return static_cast<T>(maybeUnsignedInt);
		}
		catch (...)
		{
			throw std::runtime_error{"Failed to convert string to int."};
		}
	}
	inline void checkArgumentSize(const std::vector<std::string>& arguments, size_t desiredSize)
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
			const auto freeProcess = getFreeProcess();
			const auto runningProcess = getRunningProcess();
			processes[freeProcess].parent = runningProcess;
			processes[runningProcess].childs.push_back(freeProcess);
			readyProcess(freeProcess);
			std::cout << "process " << std::to_string(freeProcess) << " created\n";
		}

		void destroy(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 1);
			const auto process = toID<ProcessID>(arguments.front());
			destroyProcess(process);
		}

		void request(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 1);
			const auto resourceIndex = toID<ResourceID>(arguments.front());

		}

		void release(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 1);
			const auto resourceIndex = toID<ResourceID>(arguments.front());

		}

		void timeout(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 0);

		}

		void init(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 0);
			//readyList.clear();
			//waitList.clear();

			// setProcessReady(0);
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
		const auto getReadyList() const noexcept
		{
			return readyList;
		}

		const auto getProcesses() const noexcept
		{
			return processes;
		}

		const auto getResources() const noexcept
		{
			return resources;
		}


	private:
		System()
		: processes{}
		, resources{}
		, readyList{}
		{
			auto id = uint32_t{0};
			for (PCB& process : processes) process.id = id++;
			id = 0;
			for (RCB& resource : resources) resource.id = id++;
			readyProcess(processes.front().id);
		};

		[[nodiscard]] inline ProcessID getRunningProcess()
		{
			return readyList.front();
		}

		[[nodiscard]] inline ProcessID getFreeProcess()
		{
			const auto iterProcess = std::ranges::find(processes, PCB::State::Free, &PCB::state);
			if (iterProcess == processes.end()) throw std::runtime_error{"All of the processes are in used."};
			else return iterProcess->id;
		}

		inline void readyProcess(ProcessID process)
		{
			assert(processes[process].state == PCB::State::Free);
			processes[process].state = PCB::State::Ready;
			readyList.push_back(process);
		}

		void destroyProcess(ProcessID process)
		{
			auto& theProcess = processes[process];
			if (theProcess.parent.has_value()) theProcess.parent = std::nullopt; // In case this is the process 0
			for (ProcessID child : theProcess.childs) destroyProcess(child);
			theProcess.childs.clear();
			for (ResourceID resource : theProcess.resources)
			{
				auto& theResource = resources[resource];
				const auto iterProcess = std::ranges::find(theResource.waitList, theProcess.id);
				theResource.waitList.erase(iterProcess);
			}
			theProcess.resources.clear();
			theProcess.state = PCB::State::Free;
		}

		void scheduler()
		{

		}

		std::array<PCB, ProcessID::MAX_EXCLUSIVE> processes;
		std::array<RCB, ResourceID::MAX_EXCLUSIVE> resources;
		std::list<ProcessID> readyList; // Current running process is at the head of the readyList
};
bool System::isInstantiated = false;


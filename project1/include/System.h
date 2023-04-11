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
			std::cout << "process " << freeProcess << " created\n";
		}

		void destroy(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 1);
			const auto process = toID<ProcessID>(arguments.front());
			if (processes[process].state == PCB::State::Free) throw std::runtime_error{"Attempted to destroy a free process"};
			std::cout << destroyProcess(process) << " processes destroyed\n"; // The process (+1) and its childs
		}

		void request(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 1);
			const auto resource = toID<ResourceID>(arguments.front());
			auto& theResource = resources[resource];
			const auto process = getRunningProcess();
			auto& theProcess = processes[process];
			const auto iterResource = std::ranges::find(theProcess.resources, resource);
			if (iterResource != theProcess.resources.end()) throw std::runtime_error{"Running process already owns this resource."};
			if (theResource.state == RCB::State::Free)
			{
				theResource.state = RCB::State::Allocated;
				theProcess.resources.push_back(resource);
				std::cout << "resource " << resource << " allocated\n";
			}
			else
			{
				theProcess.state = PCB::State::Blocked;
				removeFromReadyList(process);
				theResource.waitList.push_back(process);
				std::cout << "process " << process << " blocked\n";
				scheduler();
			}
		}

		void release(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 1);
			const auto resource = toID<ResourceID>(arguments.front());
			auto& theResource = resources[resource];
			const auto process = getRunningProcess();
			auto& theProcess = processes[process];
			releashResource(theProcess, resource);
			assert(theResource.state == RCB::State::Allocated); // Sanity check
			if (theResource.waitList.empty()) theResource.state = RCB::State::Free;
			else unBlockProcess(theResource);
			std::cout << "resource " << resource << " released\n";
		}

		void timeout(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 0);
			const auto& process = getRunningProcess();
			readyList.erase(readyList.begin());
			readyList.push_back(process);
			scheduler();
		}

		void init(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 0);
			const auto resetProcess = [](PCB& process)
			{
				process.parent = std::nullopt;
				process.childs.clear();
				process.resources.clear();
				process.state = PCB::State::Free;
			};
			std::ranges::for_each(processes, resetProcess);
			const auto resetResource = [](RCB& resource)
			{
				resource.waitList.clear();
				resource.state = RCB::State::Free;
			};
			std::ranges::for_each(resources, resetResource);
			readyList.clear();
			readyProcess(processes.front().id);
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
		const auto& getReadyList() const noexcept
		{
			return readyList;
		}

		const auto& getProcesses() const noexcept
		{
			return processes;
		}

		const auto& getResources() const noexcept
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

		void inline scheduler()
		{
			const auto process = getRunningProcess();
			std::cout << "process " << process << " running\n";
		}
		 
		[[nodiscard]] inline ProcessID getRunningProcess()
		{
			if (readyList.empty()) throw std::runtime_error{"None of the process is ready."};
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
			assert(processes[process].state != PCB::State::Ready);
			processes[process].state = PCB::State::Ready;
			readyList.push_back(process);
		}

		inline void releashResource(PCB& process, ResourceID resource)
		{
			const auto iterResource = std::ranges::find(process.resources, resource);
			if (iterResource == process.resources.end()) throw std::runtime_error{"The current running process doesn't hold that resource"};
			process.resources.erase(iterResource);
		}
		inline void unBlockProcess(RCB& resource)
		{
			const auto blockedProcess = resource.waitList.front();
			resource.waitList.pop_front();
			readyProcess(blockedProcess);
			processes[blockedProcess].resources.push_back(resource.id);
		}

		inline void removeParent(PCB& process, PCB& parent)
		{
			auto& parentChilds = parent.childs;
			const auto iterChild = std::ranges::find(parentChilds, process.id);
			assert(iterChild != parentChilds.end()); // Sanity check
			parentChilds.erase(iterChild);
			process.parent = std::nullopt; // Remove parent
		}
		inline uint32_t removeChilds(PCB& process)
		{
			auto childs = process.childs; // Make a copy. When destroy a child and disconnect it from this parent process via removeParent, the ranged loop will become UB
			auto processDestroyed = uint32_t{0};
			for (ProcessID child : childs)
			{
				processDestroyed += destroyProcess(child);
			}
			process.childs.clear();
			return processDestroyed;
		}
		inline void removeResources(PCB& process)
		{
			for (ResourceID resource : process.resources) // Remove from the resources' waitlist
			{
				auto& theResource = resources[resource];
				const auto iterProcess = std::ranges::find(theResource.waitList, process.id);
				theResource.waitList.erase(iterProcess);
			}
			process.resources.clear();
		}
		inline void removeFromReadyList(ProcessID process)
		{
			const auto iterProcess = std::ranges::find(readyList, process);
			assert(iterProcess != readyList.end());
			readyList.erase(iterProcess);
		}
		[[nodiscardd]] uint32_t destroyProcess(ProcessID process) // Return the number of processes destroyed
		{
			auto& theProcess = processes[process];
			auto processDestroyed = uint32_t{1}; // This process
			if (theProcess.parent.has_value()) removeParent(theProcess, processes[theProcess.parent.value()]); // In case this is the process 0
			processDestroyed += removeChilds(theProcess);
			removeResources(theProcess);
			removeFromReadyList(process);
			theProcess.state = PCB::State::Free;
			return processDestroyed;
		}

		std::array<PCB, ProcessID::MAX_EXCLUSIVE> processes;
		std::array<RCB, ResourceID::MAX_EXCLUSIVE> resources;
		std::list<ProcessID> readyList; // Current running process is at the head of the readyList
};
bool System::isInstantiated = false;


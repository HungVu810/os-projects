#pragma once

#include <vector>
#include <list>
#include <cassert>
#include <cerrno>
#include <array>
#include <string>
#include <concepts>
#include <ranges>
#include <algorithm>
#include <cmath>

namespace
{
	template<typename T>
	concept TypeID = std::same_as<T, ProcessID>
		|| std::same_as<T, ResourceID>
		|| std::same_as<T, PriorityID>;

	template<TypeID T>
	inline auto toID(std::string_view string)
	{
		const auto maybeID = std::stof(string.data());
		if (maybeID < 0 || maybeID >= T::MAX_EXCLUSIVE) throw std::runtime_error{"Invalid index."};
		if (maybeID - std::floor(maybeID) != 0) throw std::runtime_error{"Fractions are not allowed."};
		return static_cast<T>(maybeID);
	}

	inline auto toUnits(ResourceID resource, std::string_view string)
	{
		const auto maybeUnits = std::stof(string.data());
		if (maybeUnits < 0 || maybeUnits > unitMap[resource]) throw std::runtime_error{"Invalid units."};
		if (maybeUnits - std::floor(maybeUnits) != 0) throw std::runtime_error{"Fractions are not allowed."};
		return static_cast<Units>(maybeUnits);
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
			checkArgumentSize(arguments, 1);
			const auto priorityID = toID<PriorityID>(arguments.front());
			const auto freeProcess = getFreeProcess();
			const auto runningProcess = getRunningProcess();
			processes[runningProcess].childs.push_back(freeProcess);
			processes[freeProcess].parent = runningProcess;
			processes[freeProcess].priority = priorityID;
			readyProcess(freeProcess);
			std::cout << "process " << freeProcess << " created\n";
			scheduler();
		}

		void destroy(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 1);
			const auto process = toID<ProcessID>(arguments.front());
			if (process == 0) throw std::runtime_error{"Can't destroy process 0."}; // There should never be an empty ready list-- process 0 cannot be deleted, blocked, etc.
			const auto runningProcess = getRunningProcess();
			const auto& runningProcessChilds = processes[runningProcess].childs;
			const auto isChild = std::ranges::find(runningProcessChilds, process) != runningProcessChilds.end();
			if (process == runningProcess || isChild)
			{
				assert(processes[process].state != PCB::State::Free);
				std::cout << destroyProcess(process) << " processes destroyed\n";
				scheduler();
			}
			else throw std::runtime_error{"Specified process is not the running process or a child of such process."};
		}

		void request(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 2);

			const auto resource = toID<ResourceID>(arguments[0]);
			auto& theResource = resources[resource];

			const auto units = toUnits(resource, arguments[1]);
			if (units == 0) throw std::runtime_error{"Attempted to request 0 units"};

			const auto process = getRunningProcess();
			if (process == 0) throw std::runtime_error{"Attempted to requesting resource for process 0 which can causes deadlock"};
			auto& theProcess = processes[process];

			if (theResource.state == RCB::State::Free) theResource.state = RCB::State::Allocated;
			const auto toResourceID = [](const auto& pair)
			{
				const auto& [resourceID, units] = pair;
				return resourceID;
			};
			const auto iterPair = std::ranges::find(theProcess.resources, resource, toResourceID);
			if (iterPair != theProcess.resources.end() && iterPair->second == unitMap[resource]) throw std::runtime_error{"All ready own maximum number of units of this resource"};

			if (theResource.remain >= units)
			{
				if (iterPair == theProcess.resources.end()) ownResource(theProcess, resource, units); // First time owning this resource
				else // Accumulate the amount of units owns
				{
					iterPair->second += units;
					resources[resource].remain -= units;
				}
				std::cout << units << " units of resource " << resource << " allocated\n";
			}
			else
			{
				theProcess.state = PCB::State::Blocked;
				removeFromReadyList(theProcess);
				theResource.waitList.push_back({process, units});
				std::cout << "process " << process << " blocked\n";
				scheduler();
			}
		}

		void release(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 2);

			const auto resource = toID<ResourceID>(arguments[0]);
			auto& theResource = resources[resource];

			const auto units = toUnits(resource, arguments[1]);
			if (units == 0) throw std::runtime_error{"Attempted to release 0 units"};

			const auto process = getRunningProcess();
			auto& theProcess = processes[process];

			auto isFullyReleased = releaseResource(theProcess, resource, units);
			assert(theResource.state == RCB::State::Allocated); // Sanity check

			if (theResource.waitList.empty())
			{
				if (isFullyReleased) theResource.state = RCB::State::Free;
			}
			else tryUnblockProcesses(theResource);

			std::cout << units << " units of resource " << resource << " released\n";

			scheduler();
		}

		void timeout(const std::vector<std::string>& arguments)
		{
			checkArgumentSize(arguments, 0);
			const auto& process = getRunningProcess();
			const auto level = processes[process].priority;
			readyList[level].erase(readyList[level].begin());
			readyList[level].push_back(process);
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
				process.priority = 0;
			};
			std::ranges::for_each(processes, resetProcess);
			for (int i = 0; i < ResourceID::MAX_EXCLUSIVE; i++) // Can't do zip because g++ in UCI doesn't allow it
			{
				resources[i].waitList.clear();
				resources[i].state = RCB::State::Free;
				resources[i].remain = unitMap[i];
			}
			for (auto& list : readyList) {list.clear();}
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

		// For testing and convience
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

		[[nodiscard]] inline ProcessID getRunningProcess()
		{
			for (int level = PriorityID::MAX_EXCLUSIVE - 1; level >= 0; level--)
			{
				// Return the process at the highest priority level
				if (!readyList[level].empty()) return readyList[level].front();
			}
			throw std::runtime_error{"None of the process is ready."};
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
			for (RCB& resource : resources)
			{
				resource.id = id++;
				resource.remain = unitMap[resource.id];
			}
			readyProcess(processes.front().id);
		};

		void inline scheduler()
		{
			const auto process = getRunningProcess();
			std::cout << "process " << process << " running\n";
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
			readyList[processes[process].priority].push_back(process);
		}

		inline void removeFromReadyList(PCB& process)
		{
			const auto iterProcess = std::ranges::find(readyList[process.priority], process.id);
			assert(iterProcess != readyList[process.priority].end());
			readyList[process.priority].erase(iterProcess);
		}

		[[nodiscard]] bool releaseResource(PCB& process, ResourceID resource, Units units)
		{
			bool isFullyReleased = false;
			const auto toResourceID = [](const auto& pair)
			{
				const auto& [resourceID, units] = pair;
				return resourceID;
			};
			const auto iterPair = std::ranges::find(process.resources, resource, toResourceID);
			if (iterPair == process.resources.end()) throw std::runtime_error{"The current running process doesn't hold that resource"};
			auto& [resourceID, ownUnits] = *iterPair;
			if (ownUnits < units) throw std::runtime_error{"Attempting to release more resource than the number of owned resource"};
			else if (ownUnits == units)
			{
				isFullyReleased = true;
				process.resources.erase(iterPair);
			}
			else ownUnits -= units; // Don't remove yet because we still hold the resource
			// Refund the units to the resource
			resources[resource].remain += units;
			return isFullyReleased;
		}

		inline void ownResource(PCB& process, ResourceID resource, Units units)
		{
			process.resources.push_back({resource, units});
			resources[resource].remain -= units;
		}
		inline void tryUnblockProcesses(RCB& resource) // Can potentially unblock processes but depend on the number of units freed
		{
			do
			{
				const auto [blockedProcess, units] = resource.waitList.front(); // Make a copy because we will pop it
				if (resource.remain >= units)
				{
					resource.waitList.pop_front();
					readyProcess(blockedProcess);
					ownResource(processes[blockedProcess], resource.id, units);
				}
				else return;
			} while (resource.remain != 0);
			// Unblocked processes are transferred to ready state and own this resources # units
		}

		inline void removeParent(PCB& process, PCB& parent)
		{
			auto& parentChilds = parent.childs;
			const auto iterChild = std::ranges::find(parentChilds, process.id);
			assert(iterChild != parentChilds.end()); // Sanity check
			parentChilds.erase(iterChild);
			process.parent = std::nullopt; // Remove parent
		}
		[[nodiscard]] inline uint32_t removeChilds(PCB& process)
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
		inline void releaseResources(PCB& process)
		{
			for (const auto& [resource, units] : process.resources)
			{
				auto& theResource = resources[resource];
				auto isFullyReleased = releaseResource(process, resource, units); // Always perform a full release so we don't need to use the return value
				assert(isFullyReleased); // Sanity check
				if (theResource.waitList.empty()) theResource.state = RCB::State::Free;
				else tryUnblockProcesses(theResource);
			}
			process.resources.clear();
		}
		inline void removeFromList(PCB& process) // Either remove from the readyList or the waitList
		{
			const auto iterProcess = std::ranges::find(readyList[process.priority], process.id);
			if (iterProcess != readyList[process.priority].end())
			{
				readyList[process.priority].erase(iterProcess);
				return;
			}
			for (auto& resource : resources)
			{
				const auto toID = [](const auto& pair){ return pair.first; };
				const auto iterPair = std::ranges::find(resource.waitList, process.id, toID);
				if (iterPair != resource.waitList.end())
				{
					resource.waitList.erase(iterPair);
					return;
				}
			}
			assert(false); // Can't reach here because this mean the process isn't in the RL or the WL
		}
		[[nodiscard]] uint32_t destroyProcess(ProcessID process) // Return the number of processes destroyed
		{
			auto& theProcess = processes[process];
			auto processDestroyed = uint32_t{1}; // This process
			if (theProcess.parent.has_value()) removeParent(theProcess, processes[theProcess.parent.value()]); // In case this is the process 0
			processDestroyed += removeChilds(theProcess);
			releaseResources(theProcess);
			removeFromList(theProcess);
			theProcess.state = PCB::State::Free;
			theProcess.priority = 0;
			return processDestroyed;
		}

		std::array<PCB, ProcessID::MAX_EXCLUSIVE> processes;
		std::array<RCB, ResourceID::MAX_EXCLUSIVE> resources;
		std::array<std::list<ProcessID>, PriorityID::MAX_EXCLUSIVE> readyList; // Current running process is at the head of the readyList
};
bool System::isInstantiated = false;


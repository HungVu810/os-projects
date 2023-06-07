#include <filesystem>
#include <fstream>
#include <exception>
#include <string>
#include <array>
#include <vector>
#include <optional>
#include <cassert>
#include <algorithm>
#include <iterator>

// The starting location of a segment's PT = physicalMemory[getSegmentFrameLocation(segmentNumber)] * 512
// The starting location of a segment's page = physicalMemory[getWordLocation(segmentNumber, pageNumber, 0)]

class MemoryManager
{
public:
	MemoryManager() :
		physicalMemory(1024 * 512, -1) // 1024 frames, each frame has 512 words
		, disk(1024, std::vector<int>(512, -1)) // 1024 blocks, each block has 512 words
		, freeFrames(1024, true) // Keep track of the free frames in the physical memory, assuming that a free frame is always available
	{}

    void init(std::filesystem::path initFilePath)
    {
		auto inputFile = std::ifstream{initFilePath};
		if (!inputFile) throw std::runtime_error{"Invalid input file."};
		auto segmentCommand = std::string{};
		auto pageCommand = std::string{};
		if (!std::getline(inputFile, segmentCommand)) throw std::runtime_error{"Failed to get input command to initialize a segment table."};
		if (!std::getline(inputFile, pageCommand)) throw std::runtime_error{"Failed to get input command to initialize page tables."};
		const auto segmentInfos = toInfos<SegmentInfo>(tokenizeCommand(segmentCommand));
		const auto pageInfos = toInfos<PageInfo>(tokenizeCommand(pageCommand));
		initPhysicalMemory(segmentInfos, pageInfos);
    }

	void parseVirtualAddresses(std::filesystem::path vaFilePath)
	{
		auto vaFile = std::ifstream{vaFilePath};
		if (!vaFile) throw std::runtime_error{"Invalid input file."};
		auto outputFile = std::ofstream{vaFilePath.parent_path()/"output.txt"}; // Create an output file
		auto command = std::string{};
		while (std::getline(vaFile, command))
		{
			const auto vaStrings = tokenizeCommand(command);
			for (const auto& vaString : vaStrings)
			{
				const auto vaTranslateInfo = translateVirtualAddress(static_cast<uint32_t>(std::stoul(vaString)));
				const auto pa = getPhysicalAddress(vaTranslateInfo);
				if (pa.has_value()) outputFile << pa.value() << " ";
				else outputFile << -1 << " ";
			}
		}
	}

private:
	struct SegmentInfo // A segment at 'frame' owns multiples pages. The pages are resided at different frame and may/may not be contiguous to one another
	{
		uint32_t number; // Segment number index
		uint32_t size; // Size of the segment in term of word
		int frame; // Residing location, positive if on physical memory, negative if on disk
	};
	struct PageInfo
	{
		uint32_t segment; // Owner
		uint32_t number; // Page number index
		int frame; // Residing location, positive if on physical memory, negative if on disk
	};
	struct TranslateInfo
	{
		uint32_t s;
		uint32_t w;
		uint32_t p;
		uint32_t pw;
		uint32_t va; // Original virtual address
	};
 
	inline auto getSegmentSizeLocation(uint32_t segmentNumber)
	{
		return 2 * segmentNumber;
	}
	inline auto getSegmentFrameLocation(uint32_t segmentNumber)
	{
		return 2 * segmentNumber + 1;
	}
	inline auto getPageFrameLocation(uint32_t segmentNumber, uint32_t pageNumber) // The location to a segment's pages within a PT
	{
		const auto segmentFrameLocation = physicalMemory[getSegmentFrameLocation(segmentNumber)];
		return segmentFrameLocation * 512 + (segmentFrameLocation < 0 ? -1 : 1) * static_cast<int>(pageNumber); // Can be negative
	}
	inline auto getWordLocation(uint32_t segmentNumber, uint32_t pageNumber, uint32_t wordOffset) // The location to a segment's words within a page within a PT
	{
		const auto pageFrameLocation = getPageFrameLocation(segmentNumber, pageNumber);
		assert(pageFrameLocation > 0); // Must be updated with a valid page frame location when used in getPhysicalAddress
		return physicalMemory[pageFrameLocation] * 512 + wordOffset; // wordOffset [0, 511], PT of pageNumber occupied locations 
	}

	inline auto allocateFreeFrameLocation()
	{
		const auto frameIter = std::ranges::find(freeFrames, true);
		assert(frameIter != freeFrames.end()); // Failed assumption
		*frameIter = false;
		return std::distance(freeFrames.begin(), frameIter);
	}

	void initPhysicalMemory(const std::vector<SegmentInfo>& segmentInfos, const std::vector<PageInfo>& pageInfos)
	{
		for (const auto& segmentInfo : segmentInfos)
		{
			physicalMemory[getSegmentSizeLocation(segmentInfo.number)] = segmentInfo.size; // PM[2s] = segmentSize
			physicalMemory[getSegmentFrameLocation(segmentInfo.number)] = segmentInfo.frame; // PM[2s + 1] = segmentFrame
			if (segmentInfo.frame >= 0) freeFrames[segmentInfo.frame] = false;
		}
		for (const auto& pageInfo : pageInfos)
		{
			const auto pageFrameLocation = getPageFrameLocation(pageInfo.segment, pageInfo.number); // PT
			if (pageFrameLocation < 0) disk[std::abs(pageFrameLocation) / 512][std::abs(pageFrameLocation) % 512] = pageInfo.frame;
			else physicalMemory[pageFrameLocation] = pageInfo.frame;

			freeFrames[pageInfo.number] = false;
			if (pageInfo.frame >= 0) freeFrames[pageInfo.frame] = false;
		}
	}

	std::optional<uint32_t> getPhysicalAddress(const TranslateInfo& va)
	{
		if (va.pw >= physicalMemory[getSegmentSizeLocation(va.s)]) return std::nullopt;

		// Only frames/pages are either valid (uint32_t) or not valid (negative int)
		if (physicalMemory[getSegmentFrameLocation(va.s)] < 0)
		{
			const auto segmentBlock = std::abs(physicalMemory[getSegmentFrameLocation(va.s)]);
			const auto freeFrameLocation = allocateFreeFrameLocation();
			readBlock(segmentBlock, freeFrameLocation);
			physicalMemory[getSegmentFrameLocation(va.s)] = freeFrameLocation;
			//Allocate free frame f1 using list of free frames
			//Update list of free frames
			//Read disk block b = |PM[2s + 1]| into PM staring at location f1*512
			//PM[2s + 1] = f1
		}
		//const auto pageFrame = std::get_if<uint32_t>(&physicalMemory[physicalMemory[2ull * va.s + 1ull] * 512ull + va.p]);
		if (physicalMemory[getPageFrameLocation(va.s, va.p)] < 0)
		{
			const auto pageBlock = std::abs(physicalMemory[getPageFrameLocation(va.s, va.p)]);
			const auto freeFrameLocation = allocateFreeFrameLocation();
			readBlock(pageBlock, freeFrameLocation);
			physicalMemory[getPageFrameLocation(va.s, va.p)] = freeFrameLocation;
			//Allocate free frame f2 using list of free frames
			//Update list of free frames
			//Read disk block b = |PM[PM[2s + 1]*512 + p]| into PM staring at f2*512
			//PM[PM[2s + 1]*512 + p] = f2
		}
		return static_cast<uint32_t>(getWordLocation(va.s, va.p, va.w));

		// PA = PM[PM[2s+1]*512+p]*512+w, check for page fault
		// s: 9 bit, p: 9 bit, w:; 9 bit, present bit: 1 bit
		// The sign bit is used as the present bit (negative = not resident
	}

	TranslateInfo translateVirtualAddress(uint32_t va)
	{
		// 32 bits = (5 empty bits) (9 bits = s) (9 bits = p) (9 bits = w)
		return TranslateInfo{
			va >> 18 // s
			, va & 0x1FF // w
			, (va >> 9) & 0x1FF // p
			, va & 0x3FFFF // pw
			, va // va
		};
	}

	void readBlock(uint32_t b, uint32_t m) // Copy block b from disk to a frame at address m into the physical memory
	{
		for (int i = 0; i < disk[b].size(); i++)
		{
			physicalMemory[m * 512 + i] = disk[b][i];
		}
	}

	[[nodiscard]] std::vector<std::string> tokenizeCommand(std::string_view command)
	{
		auto commandStream = std::istringstream{command.data()};
		auto tokens = std::vector<std::string>{};
		auto token = std::string{};
		while (commandStream >> token)
		{
			tokens.push_back(token);
		}
		return tokens;
	}

	template<typename Info>
	[[nodiscard]] std::vector<Info> toInfos(const std::vector<std::string>& tokenizedCommand)
	{
		if (tokenizedCommand.size() % 3 != 0) throw std::runtime_error{"Input command doesn't follow the required format."};
		auto infos = std::vector<Info>(tokenizedCommand.size() / 3);
		for (size_t i = 0; i < infos.size(); i++)
		{
			const auto residingFrame = std::stol(tokenizedCommand[i * 3 + 2].data());
			infos[i] = Info{
				static_cast<uint32_t>(std::stoul(tokenizedCommand[i * 3].data()))
				, static_cast<uint32_t>(std::stoul(tokenizedCommand[i * 3 + 1].data()))
				, static_cast<int>(residingFrame)
			};
		}
		return infos;
	}

	std::vector<int> physicalMemory; // Use a lot of stack memory, using the heap instead
	std::vector<std::vector<int>> disk; // Use a lot of stack memory, using the heap instead
	std::vector<bool> freeFrames;
};



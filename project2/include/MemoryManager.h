#include <filesystem>
#include <fstream>
#include <exception>
#include <string>
#include <array>
#include <vector>
#include <optional>
#include <variant>
#include <cassert>

class MemoryManager
{
public:
	MemoryManager() :
		physicalMemory(1024 * 512, int{-1}) // 1024 frames, each frame has 512 words
		, disk(1024, std::vector<uint32_t>(512)) // 1024 blocks, each block has 512 words
		, freeFrames(1024, true) // 1024 free blocks in the disk
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
				const auto vaTranslateInfo = translateVirtualAddress(std::stoul(vaString));
				const auto pa = getPhysicalAddress(vaTranslateInfo);
				if (pa.has_value()) outputFile << pa.value() << " ";
				else outputFile << -1;
			}
		}
	}

private:
	struct SegmentInfo // A segment at 'frame' owns multiples pages. The pages are resided at different frame and may/may not be contiguous to one another
	{
		uint32_t number;
		uint32_t size;
		std::variant<uint32_t, int> frame; // Residing location, uint32_t if on physical memory, int negative if on disk
	};
	struct PageInfo
	{
		uint32_t segment; // Owner
		uint32_t number;
		std::variant<uint32_t, int> frame; // Residing location, uint32_t if on physical memory, int negative if on disk
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
		return 2ull * segmentNumber;
	}
	inline auto getSegmentFrameLocation(uint32_t segmentNumber)
	{
		return 2ull * segmentNumber + 1ull;
	}
	inline auto getPageFrameLocation(uint32_t segmentNumber, uint32_t pageNumber)
	{
		return std::get<uint32_t>(physicalMemory[getSegmentFrameLocation(segmentNumber)]) * 512ull + pageNumber;
	}
	inline auto getWordLocation(uint32_t segmentNumber, uint32_t pageNumber, uint32_t wordOffset)
	{
		return std::get<uint32_t>(physicalMemory[getPageFrameLocation(segmentNumber, pageNumber)]) * 512ull + wordOffset;
	}

	void initPhysicalMemory(const std::vector<SegmentInfo>& segmentInfos, const std::vector<PageInfo>& pageInfos)
	{
		for (const auto& segmentInfo : segmentInfos)
		{
			physicalMemory[getSegmentSizeLocation(segmentInfo.number)] = segmentInfo.size; // PM[2s] = segmentSize
			physicalMemory[getSegmentFrameLocation(segmentInfo.number)] = segmentInfo.frame; // PM[2s + 1] = segmentFrame
		}
		for (const auto& pageInfo : pageInfos)
		{
			physicalMemory[getPageFrameLocation(pageInfo.segment, pageInfo.number)] = pageInfo.frame; // PM[PM[2s + 1] * 512 + p] = w
		}
	}

	std::optional<uint32_t> getPhysicalAddress(const TranslateInfo& va)
	{
		if (va.pw >= std::get<uint32_t>(physicalMemory[getSegmentSizeLocation(va.s)])) return std::nullopt;

		// Only frames/pages are either valid (uint32_t) or not valid (negative int)
		auto segmentFrame = std::get_if<uint32_t>(&physicalMemory[getSegmentFrameLocation(va.s)]);
		if (segmentFrame == nullptr)
		{
			const auto segmentBlock = std::abs(std::get<int>(physicalMemory[getSegmentFrameLocation(va.s)]));
			assert(freeFrames[segmentBlock]);
			freeFrames[segmentBlock] = false;
			readBlock(segmentBlock, getSegmentFrameLocation(va.s));
			//• Allocate free frame f1 using list of free frames
			//• Update list of free frames
			//• Read disk block b = |PM[2s + 1]| into PM staring at location f1*512
			//• PM[2s + 1] = f1
		}
		//const auto pageFrame = std::get_if<uint32_t>(&physicalMemory[physicalMemory[2ull * va.s + 1ull] * 512ull + va.p]);
		const auto pageFrame = std::get_if<uint32_t>(&physicalMemory[getPageFrameLocation(va.s, va.p)]);
		if (pageFrame == nullptr)
		{
			const auto pageBlock = std::abs(std::get<int>(physicalMemory[getPageFrameLocation(va.s, va.p)]));
			assert(freeFrames[pageBlock]);
			freeFrames[pageBlock] = false;
			readBlock(pageBlock, getPageFrameLocation(va.s, va.p));
			//• Allocate free frame f2 using list of free frames
			//• Update list of free frames
			//• Read disk block b = |PM[PM[2s + 1]*512 + p]| into PM staring at f2*512
			//• PM[PM[2s + 1]*512 + p] = f2
		}
		return static_cast<uint32_t>(getWordLocation(va.s, va.p, va.w));

		//// PA = PM[PM[2s+1]*512+p]*512+w, check for page fault
		//// s: 9 bit, p: 9 bit, w:; 9 bit, present bit: 1 bit
		//// The sign bit is used as the present bit (negative = not resident

		////uint32_t address // user input
		////std::numeric_limit<uint32_t>()
		////
		////[s, p, w] = char* addressBytes[sizeof(uint32_t)] divided among s(sz (7), frame# to p (7) -> p (9) -> w within page (9)
		////
		////
	}

	TranslateInfo translateVirtualAddress(uint32_t va)
	{
		// 32 bits = (5 empty bits) (9 bits = s) (9 bits = p) (9 bits = w)
		return TranslateInfo{
			va >> 18 // s
			, va & 0x1FF // w
			, (va >> 9) & 0x1FF // p
			, va & 0x3FFF // pw
			, va // va
		};
	}

	void readBlock(uint32_t b, uint32_t m) // Copy block b from disk to a frame at address m into the physical memory
	{
		for (int i = 0; i < disk[b].size(); i++)
		{
			physicalMemory[m + i] = disk[b][i];
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
			const auto residingLoc = std::stol(tokenizedCommand[i * 3 + 2].data());
			infos[i] = Info{
				std::stoul(tokenizedCommand[i * 3].data())
				, std::stoul(tokenizedCommand[i * 3 + 1].data())
				, residingLoc < 0 ? static_cast<int>(residingLoc) : static_cast<uint32_t>(residingLoc)
			};
		}
		return infos;
	}

	std::vector<std::variant<uint32_t, int>> physicalMemory; // Use a lot of stack memory, using the heap instead
	std::vector<std::vector<uint32_t>> disk; // Use a lot of stack memory, using the heap instead
	std::vector<bool> freeFrames;
};



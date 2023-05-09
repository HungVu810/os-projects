#include <filesystem>
#include <fstream>
#include <exception>
#include <string>
#include <array>
#include <vector>

//uint32_t address // user input
//std::numeric_limit<uint32_t>()
//
//[s, p, w] = char* addressBytes[sizeof(uint32_t)] divided among s(sz (7), frame# to p (7) -> p (9) -> w within page (9)
//
//
//      s       p      w
//    9 bit   9 bit   9 bit

class MemoryManager
{
public:
	MemoryManager() :
		physicalMemory(1024 * 512) // 1024 frames, each frame has 512 words
		, disk(512, std::vector<uint32_t>(1024))
	{}

    void init(std::filesystem::path initFile, std::filesystem::path vaFile)
    {
		auto inputFile = std::ifstream{initFile};
		if (!inputFile) throw std::runtime_error{"Invalid input file."};
		auto outputFile = std::ofstream{initFile.parent_path()/"output.txt"}; // Create an output file, std::fstream{path, std::ios::out};
		auto segmentCommand = std::string{};
		auto pageCommand = std::string{};
		if (!std::getline(inputFile, segmentCommand)) throw std::runtime_error{"Failed to get input command to initialize a segment table."};
		if (!std::getline(inputFile, pageCommand)) throw std::runtime_error{"Failed to get input command to initialize page tables."};
		const auto segmentInfos = toInfos<SegmentInfo>(tokenizeCommand(segmentCommand));
		const auto pageInfos = toInfos<PageInfo>(tokenizeCommand(pageCommand));
		initPhysicalMemory(segmentInfos, pageInfos);
    }


private:
	struct SegmentInfo // A segment at 'frame' owns multiples pages. The pages are resided at different frame and may/may not be contiguous to one another
	{
		uint32_t number;
		uint32_t size;
		uint32_t frame; // Residing location
	};
	struct PageInfo
	{
		uint32_t number;
		uint32_t segment; // Owner
		uint32_t frame; // Residing location
	};
	struct TranslateInfo
	{
		uint32_t s;
		uint32_t p;
		uint32_t w;
		uint32_t pw;
	};

	void initPhysicalMemory(const std::vector<SegmentInfo>& segmentInfos, const std::vector<PageInfo>& pageInfos)
	{
		for (const auto& segmentInfo : segmentInfos)
		{
			physicalMemory[2ull * segmentInfo.number] = segmentInfo.size;
			physicalMemory[2ull * segmentInfo.number + 1ull] = segmentInfo.frame;
		}

		for (const auto& pageInfo : pageInfos)
		{
			physicalMemory[(2ull * pageInfo.segment + 1ull) * 512ull + pageInfo.number] = pageInfo.frame;
		}
	}

	void interpretVirtualAddress(const TranslateInfo& va)
	{
		// PA = PM[PM[2s+1]*512+p]*512+w, check for page fault
		// s: 9 bit, p: 9 bit, w:; 9 bit, present bit: 1 bit
		// The sign bit is used as the present bit (negative = not resident
	}

	TranslateInfo translateVirtualAddress(uint32_t va)
	{
		return TranslateInfo{
			va >> 18
			, va & 0x1FF
			, (va >> 9) & 0x1FF
			, va & 0x3FFF
		};
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
			infos[i] = Info{
				std::stoul(tokenizedCommand[i * 3].data())
				, std::stoul(tokenizedCommand[i * 3 + 1].data())
				, std::stoul(tokenizedCommand[i * 3 + 2].data())
			};
		}
		return infos;
	}

	std::vector<uint32_t> physicalMemory; // Use a lot of stack memory, using the heap instead
	std::vector<std::vector<uint32_t>> disk; // Use a lot of stack memory, using the heap instead
};



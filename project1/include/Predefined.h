#pragma once
#include <cassert>
#include <sstream>
#include <iostream>
#include <ranges>

struct ProcessID
{
	ProcessID() = default;

	ProcessID(uint32_t i) : id{i}{};

	ProcessID& operator=(uint32_t inID)
	{
		assert(inID >= 0 && inID < MAX_EXCLUSIVE);
		id = inID;
		return *this;
	}

	operator uint32_t() const noexcept {return id;}

	uint32_t id;
	static const uint32_t MAX_EXCLUSIVE = 16; // [0, MAX_EXCLUSIVE]
};

struct ResourceID
{
	ResourceID() = default;

	ResourceID(uint32_t i) : id{i}{};

	ResourceID& operator=(uint32_t inID)
	{
		assert(inID >= 0 && inID < MAX_EXCLUSIVE);
		id = inID;
		return *this;
	}

	operator uint32_t() const noexcept {return id;}

	uint32_t id;
	static const uint32_t MAX_EXCLUSIVE = 4;
};

using Priority = uint8_t;
constexpr uint32_t NUM_PRIORITY = 3;

class OutputCapture // HAVE A singleton base class
{
public:
	OutputCapture() :
	localBuffer{}
	, coutBuffer{nullptr}
	{}

	~OutputCapture()
	{
		std::cout.rdbuf(coutBuffer);
	};

	void capture()
	{
		coutBuffer = std::cout.rdbuf();
		std::cout.rdbuf(localBuffer.rdbuf());
	}

	std::string getOutput(uint32_t offset = 0) // Return the end line, or a line X offseted by "offset" from the end line
	{
		// auto output = localBuffer.str();
		//output.pop_back(); // Remove the end \n
		//const auto reverseIterChar = std::find(output.rbegin(), output.rend(), '\n');
		//if (reverseIterChar == output.rend()) return output; // Output contains only one line
		//return std::string(reverseIterChar.base(), output.end());

		if (offset >= std::ranges::count(localBuffer.view(), '\n')) throw std::runtime_error{"offset has to be within [0, lines - 1]"};
		auto lines = std::vector<std::string_view>{};
		for (const auto line : localBuffer.view() | std::views::split('\n'))
		{
			lines.push_back(std::string_view(line.begin(), line.end()));
		}
		return std::string{*(lines.rbegin() + 1 + offset)}; // Add 1 because the string_view at the end always empty (ie: "line1"\n"line2"\n -> {"line1", "line2", ""})
	}

private:
	std::ostringstream localBuffer;
	std::streambuf* coutBuffer;
};


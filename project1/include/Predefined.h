#pragma once
#include <cassert>
#include <sstream>
#include <iostream>
#include <cassert>

struct ProcessID
{
	constexpr ProcessID() : id{0}{};

	constexpr ProcessID(uint32_t inID) : id{inID}
	{
		assert(inID >= 0 && inID < MAX_EXCLUSIVE);
	};

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
	constexpr ResourceID() : id{0}{};

	constexpr ResourceID(uint32_t inID) : id{inID}
	{
		assert(inID >= 0 && inID < MAX_EXCLUSIVE);
	};

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

struct PriorityID
{
	constexpr PriorityID() : id{0}{};

	constexpr PriorityID(uint32_t inID) : id{inID}
	{
		assert(inID >= 0 && inID < MAX_EXCLUSIVE);
	};

	PriorityID& operator=(uint32_t inID)
	{
		assert(inID >= 0 && inID < MAX_EXCLUSIVE);
		id = inID;
		return *this;
	}

	operator uint32_t() const noexcept {return id;}

	uint32_t id;
	static const uint32_t MAX_EXCLUSIVE = 3;
};

#ifdef CATCH_CONFIG_MAIN
#include <ranges>
#include <algorithm>
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
		if (offset >= std::ranges::count(localBuffer.view(), '\n')) throw std::runtime_error{"offset has to be within [0, lines - 1]"};
		auto lines = std::vector<std::string_view>{};
		for (const auto& line : localBuffer.view() | std::views::split('\n'))
		{
			lines.push_back(std::string_view(line.begin(), line.end()));
		}
		return std::string{*(lines.rbegin() + 1 + offset)}; // Add 1 because the string_view at the end always empty (ie: "line1"\n"line2"\n -> {"line1", "line2", ""})
	}

private:
	std::ostringstream localBuffer;
	std::streambuf* coutBuffer;
};
#endif



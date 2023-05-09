//#include <iostream>
#include <MemoryManager.h>
#include <vector>

int main(int argc, const char *const *const argv)
{
	auto arguments = std::vector<std::string_view>(argv, argv + argc);

	auto memoryManager = MemoryManager{};
	if (arguments.size() != 2) throw std::runtime_error{"Missing input file name"};
	memoryManager.init(arguments[1]);
}

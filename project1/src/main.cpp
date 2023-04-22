#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <functional>
#include <exception>
#include <filesystem>
#include <fstream>

// https://slideplayer.com/slide/3334835/
// g++ -std=c++20

#include "Predefined.h"
#include "PCB.h"
#include "RCB.h"
#include "System.h"
#include "Shell.h"

int main(int argc, const char *const *const argv)
{
	auto system = System::getInstance();
	auto shell = Shell::getInstance();

	auto arguments = std::vector<std::string_view>(argv, argv + argc);
	// The first arguments is always the name of the program

	if (arguments.size() > 2) throw std::runtime_error{"Only a file name or a path to a file contains input info is needed."};
	else if (arguments.size() == 1) shell.run(system);
	else shell.run(system, arguments[1]);
}



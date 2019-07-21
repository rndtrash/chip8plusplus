#include <stdexcept>
#include <fstream>
#include <iostream>
#include <ios>
#include <array>

#include "chip8.hpp"

std::vector<uint8_t> readFile(const std::string filename, std::ios_base::openmode flags)
{
	std::ifstream stream(filename, std::ios::in | std::ios::binary);
	std::vector<uint8_t> contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

	return contents;
}


int main(int argc, char* argv[])
{
	bool is_running = true;

	if (argc < 2)
		return 1;

	Chip8 c8 = Chip8();
	c8.load(readFile(argv[1], std::ios::binary));

	while (is_running)
	{
		c8.tick();
		if (c8.isScreenChanged())
		{
			std::array<std::array<bool, 64>, 32> s = c8.getScreen();
			for (int y = 0; y < s.size(); y++)
			{
				for (int x = 0; x < s[y].size(); x++)
				{
					std::string c = (s[y][x]) ? "\u2588" : " ";
					std::cout << c;
				}
				std::cout << std::endl;
			}
			std::cout << "---" << std::endl;
		}
	}

	return 0;
}

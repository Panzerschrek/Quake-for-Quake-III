#include "BspcLibIncludes.hpp"
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>


void ReadFile(FILE& f, const uint64_t offset, const uint64_t size, void* const out_data)
{
	std::fseek(&f, long(offset), SEEK_SET);
	std::fread(out_data, size, 1, &f); // TODO - read in loop
}

int main(const int argc, const char* const argv[])
{
	std::string in_file_name;
	std::string out_file_name_base;
	std::string palette_file_name;
	for(int i= 1; i < argc; ++i)
	{
		if(std::strcmp(argv[i], "-i") == 0)
		{
			++i;
			if(i >= argc)
			{
				std::cerr << "Expected input file name after -i." << std::endl;
				return -1;
			}
			in_file_name= argv[i];
		}
		else if(std::strcmp(argv[i], "-o") == 0)
		{
			++i;
			if(i >= argc)
			{
				std::cerr << "Expected ouptut file name base after -o." << std::endl;
				return -1;
			}
			out_file_name_base= argv[i];
		}
		else if(std::strcmp(argv[i], "-p") == 0)
		{
			++i;
			if(i >= argc)
			{
				std::cerr << "Expected palette file after -p." << std::endl;
				return -1;
			}
			palette_file_name= argv[i];
		}
		else
		{
			std::cerr << "Unrecognized option \"" << argv[i] << "\"." << std::endl;
			return -1;
		}
	}

	if(in_file_name.empty())
	{
		std::cerr << "No input file provided." << std::endl;
		return -1;
	}
	if(out_file_name_base.empty())
	{
		std::cerr << "No output file provided." << std::endl;
		return -1;
	}
	if(palette_file_name.empty())
	{
		std::cerr << "No palette file provided." << std::endl;
		return -1;
	}

	// Load palette
	FILE* const palette_file= std::fopen(palette_file_name.c_str(), "rb");
	if(palette_file == nullptr)
	{
		std::cerr << "Failed to open \"" << palette_file << "\"." << std::endl;
		return -1;
	}

	byte palette[256*3];
	ReadFile(*palette_file, 0, sizeof(palette), &palette);
	std::fclose(palette_file);

	// Load input file.
	FILE* const in_file= std::fopen(in_file_name.c_str(), "rb");
	if(in_file == nullptr)
	{
		std::cerr << "Failed to open \"" << in_file_name << "\"." << std::endl;
		return -1;
	}
	std::fseek(in_file, 0, SEEK_END);
	const auto in_file_size= std::ftell(in_file);

	std::vector<byte> file_data;
	file_data.resize(size_t(in_file_size));

	ReadFile(*in_file, 0, size_t(in_file_size), file_data.data());

	std::fclose(in_file);

	// TODO - read sprite here.
}

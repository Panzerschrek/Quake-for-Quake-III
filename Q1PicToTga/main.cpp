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

	// Convert texture into RGB.
	const auto miptex= reinterpret_cast<const q1_miptex_t*>(file_data.data());

	std::vector<byte> tmp_tex_data;
	tmp_tex_data.resize(miptex->width * miptex->height * 4);

	const byte* const src_tex_data= reinterpret_cast<const byte*>(miptex) + miptex->offsets[0];
	// Flip image to convert to OpenGL coordinate system (used in Q3).
	for(unsigned int y= 0; y < miptex->height; ++y)
	for(unsigned int x= 0; x < miptex->width ; ++x)
	{
		const unsigned int dst= x + y * miptex->width;
		const unsigned int src= x + (miptex->height - 1 - y) * miptex->width;
		const byte color_index= src_tex_data[src];
		tmp_tex_data[ dst * 4     ]= palette[ color_index * 3    ];
		tmp_tex_data[ dst * 4 + 1 ]= palette[ color_index * 3 + 1 ];
		tmp_tex_data[ dst * 4 + 2 ]= palette[ color_index * 3 + 2 ];
		tmp_tex_data[ dst * 4 + 3 ]= 255;
	}

	// Save result TGA.
	WriteTGA((out_file_name_base + ".tga").c_str(), tmp_tex_data.data(), int(miptex->width), int(miptex->height));

	// Extract fullbright pixels. use alpha = 0 for regular pixels, alpha = 1 for fullbright pixels.
	unsigned int num_fullbright_pixels= 0;
	for(unsigned int y= 0; y < miptex->height; ++y)
	for(unsigned int x= 0; x < miptex->width ; ++x)
	{
		const unsigned int dst= x + y * miptex->width;
		const unsigned int src= x + (miptex->height - 1 - y) * miptex->width;
		const byte color_index= src_tex_data[src];
		if (color_index < 256 - 16 * 2)
		{
			tmp_tex_data[ dst * 4     ]= 0;
			tmp_tex_data[ dst * 4 + 1 ]= 0;
			tmp_tex_data[ dst * 4 + 2 ]= 0;
			tmp_tex_data[ dst * 4 + 3 ]= 0;
		}
		else
		{
			++num_fullbright_pixels;
			tmp_tex_data[ dst * 4     ]= palette[ color_index * 3    ];
			tmp_tex_data[ dst * 4 + 1 ]= palette[ color_index * 3 + 1 ];
			tmp_tex_data[ dst * 4 + 2 ]= palette[ color_index * 3 + 2 ];
			tmp_tex_data[ dst * 4 + 3 ]= 255;
		}
	}

	if (num_fullbright_pixels > 0)
	{
		// Save result TGA of fullbright image.
		WriteTGA((out_file_name_base + "_fb.tga").c_str(), tmp_tex_data.data(), int(miptex->width), int(miptex->height));
	}
}

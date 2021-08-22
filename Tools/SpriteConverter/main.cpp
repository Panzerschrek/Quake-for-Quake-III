#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

typedef unsigned char byte;

#define SPRITE_VERSION	1

// must match definition in modelgen.h
#ifndef SYNCTYPE_T
#define SYNCTYPE_T
typedef enum {ST_SYNC=0, ST_RAND } synctype_t;
#endif

// TODO: shorten these?
typedef struct {
	int			ident;
	int			version;
	int			type;
	float		boundingradius;
	int			width;
	int			height;
	int			numframes;
	float		beamlength;
	synctype_t	synctype;
} dsprite_t;

#define SPR_VP_PARALLEL_UPRIGHT		0
#define SPR_FACING_UPRIGHT			1
#define SPR_VP_PARALLEL				2
#define SPR_ORIENTED				3
#define SPR_VP_PARALLEL_ORIENTED	4

typedef struct {
	int			origin[2];
	int			width;
	int			height;
} dspriteframe_t;

typedef struct {
	int			numframes;
} dspritegroup_t;

typedef struct {
	float	interval;
} dspriteinterval_t;

typedef enum { SPR_SINGLE=0, SPR_GROUP } spriteframetype_t;

typedef struct {
	spriteframetype_t	type;
} dspriteframetype_t;

#define IDSPRITEHEADER	(('P'<<24)+('S'<<16)+('D'<<8)+'I') // little-endian "IDSP"

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
	const auto& sprite_header = *reinterpret_cast<const dsprite_t*>(file_data.data());

	if (sprite_header.ident != IDSPRITEHEADER)
	{
		std::cerr << "\""<< in_file_name << "\" is not a Quake sprite file." << std::endl;
		return -1;
	}
	std::cout << "Sprite ident: " << sprite_header.ident << ", num frames: " << sprite_header.numframes << std::endl;

}

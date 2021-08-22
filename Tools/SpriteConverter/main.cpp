#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

// Avoid includeng BSPC includes to prevent name conflicts.
typedef unsigned char byte;
extern "C" void WriteTGA (const char *filename, byte *data, int width, int height);

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

#define MAX_SPRITE_SIZE 64

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

	const auto& sprite_header = *reinterpret_cast<const dsprite_t*>(file_data.data());
	// TODo - do byteswap.

	if (sprite_header.ident != IDSPRITEHEADER)
	{
		std::cerr << "\""<< in_file_name << "\" is not a Quake sprite file." << std::endl;
		return -1;
	}
	if (sprite_header.width > MAX_SPRITE_SIZE || sprite_header.height > MAX_SPRITE_SIZE)
	{
		std::cerr << "Sprite is too big - " << sprite_header.width << "x" << sprite_header.height << ", maximum size is " << MAX_SPRITE_SIZE << "x" << MAX_SPRITE_SIZE << std::endl;
		return 01;
	}

	size_t frame_number= 0;
	std::vector<byte> data_rgba;
	data_rgba.resize(MAX_SPRITE_SIZE * MAX_SPRITE_SIZE * 4);

	auto ptr= file_data.data() + sizeof(dsprite_t);
	for( int i= 0; i < sprite_header.numframes; ++i )
	{
		const auto frametype= *reinterpret_cast<const spriteframetype_t*>(ptr);
		ptr+= sizeof(spriteframetype_t);

		const auto process_frame =
		[&]
		{
			const auto& frame = *reinterpret_cast<const dspriteframe_t*>(ptr);
			ptr+= sizeof(dspriteframe_t);

			const auto frame_data= ptr;
			const int size = frame.width * frame.height;

			std::memset(data_rgba.data(), 0, data_rgba.size());

			const int start_x = (MAX_SPRITE_SIZE - sprite_header.width ) / 2 + frame.width  / 2 + frame.origin[0];
			const int start_y = (MAX_SPRITE_SIZE - sprite_header.height) / 2 + frame.height / 2 - frame.origin[1];
			for( int y= 0; y < frame.height; ++y )
			for( int x= 0; x < frame.width ; ++x )
			{
				const int dst_pixel = x + start_x  + (MAX_SPRITE_SIZE - 1 - (y + start_y)) * MAX_SPRITE_SIZE;
				const byte pixel= frame_data[ x + y * frame.width ];
				data_rgba[ dst_pixel * 4 + 0 ] = palette[ pixel * 3 + 0 ];
				data_rgba[ dst_pixel * 4 + 1 ] = palette[ pixel * 3 + 1 ];
				data_rgba[ dst_pixel * 4 + 2 ] = palette[ pixel * 3 + 2 ];
				data_rgba[ dst_pixel * 4 + 3 ] = pixel == 255 ? 0 : 255;
			}

			WriteTGA((out_file_name_base + std::to_string(frame_number) + ".tga").c_str(), data_rgba.data(), MAX_SPRITE_SIZE, MAX_SPRITE_SIZE);
			++frame_number;

			ptr+= size;
		};

		if(frametype == SPR_SINGLE)
			process_frame();
		else if(frametype == SPR_GROUP)
		{
			const auto& group = *reinterpret_cast<const dspritegroup_t*>(ptr);
			ptr+= sizeof(dspritegroup_t);

			for( int j= 0; j < group.numframes; ++j )
			{
				const auto& interval = reinterpret_cast<const dspriteinterval_t*>(ptr);
				ptr+= sizeof(dspriteinterval_t);
			}

			for( int j= 0; j < group.numframes; ++j )
				process_frame();
		}
		else
		{
			std::cerr << "Unknown frame type: " << frametype << std::endl;
			return -1;
		}
	}
}

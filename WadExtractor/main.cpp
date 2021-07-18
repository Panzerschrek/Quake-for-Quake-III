#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

struct WadHeader
{
	char id[4];
	uint32_t numlumps;
	uint32_t infotableofs;
};

struct Wad2FileLump
{
	uint32_t filepos;
	uint32_t disksize;
	uint32_t size;
	char type;
	char compression;
	char pad1, pad2;
	char name[16];
};

void ReadFile(FILE& f, const uint64_t offset, const uint64_t size, void* const out_data)
{
	std::fseek(&f, long(offset), SEEK_SET);
	std::fread(out_data, size, 1, &f); // TODO - read in loop
}

void WriteFileAll(FILE& f, const uint64_t size, const void* const data)
{
	std::fwrite(data, size, 1, &f);
}

int main(const int argc, const char* const argv[])
{
	std::string file_name;
	std::string out_dir;
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
			file_name= argv[i];
		}
		else if(std::strcmp(argv[i], "-o") == 0)
		{
			++i;
			if(i >= argc)
			{
				std::cerr << "Expected ouptut directory after -o." << std::endl;
				return -1;
			}
			out_dir= argv[i];
		}
		else
		{
			std::cerr << "Unrecognized option \"" << argv[i] << "\"." << std::endl;
			return -1;
		}
	}

	if(file_name.empty())
	{
		std::cerr << "No input file provided." << std::endl;
		return -1;
	}
	if(out_dir.empty())
	{
		std::cerr << "No output directory provided." << std::endl;
		return -1;
	}

	FILE* const f= std::fopen(file_name.c_str(), "rb");
	if(f == nullptr)
	{
		std::cerr << "Failed to open \"" << file_name << "\"." << std::endl;
		return -1;
	}

	WadHeader header{};
	ReadFile(*f, 0, sizeof(header), &header);

	const char wad2_id[4] = { 'W', 'A', 'D', '2' };
	if(std::memcmp(header.id, wad2_id, sizeof(wad2_id)) != 0)
	{
		std::cerr << "\"" << file_name << "\" is not a WAD2 file." << std::endl;
		std::fclose(f);
		return -1;
	}

	std::vector<Wad2FileLump> lumps;
	lumps.resize(header.numlumps);
	ReadFile(*f, header.infotableofs, header.numlumps * sizeof(Wad2FileLump), lumps.data());

	std::vector<uint8_t> file_buffer;
	for(const Wad2FileLump& lump : lumps)
	{
		char name_nt[17]{};
		std::strncpy(name_nt, lump.name, sizeof(lump.name));
		std::cout << "Extracting \"" << name_nt << "\"." << std::endl;

		const std::string out_file_name= out_dir + "/" + name_nt;
		FILE* const out_file= std::fopen(out_file_name.c_str(), "wb");
		if(out_file == nullptr)
		{
			std::cerr << "Failed to open \"" << out_file_name << "\"." << std::endl;
			continue;
		}

		file_buffer.resize(lump.size);

		ReadFile(*f, lump.filepos, lump.size, file_buffer.data());
		WriteFileAll(*out_file, lump.size, file_buffer.data());

		std::fclose(out_file);
	}

	std::fclose(f);
}

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
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

int main(const int argc, const char* const argv[])
{
	(void)argv;
	const char* file_name= "QUAKE101.WAD";
	for(int i= 0; i < argc; ++i)
	{
	}

	FILE* const f= std::fopen(file_name, "rb");
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

	for(const Wad2FileLump& lump : lumps)
	{
		char name_nt[17]{};
		std::strncpy(name_nt, lump.name, sizeof(lump.name));
		std::cout << name_nt << "\n";
	}

	std::fclose(f);
}

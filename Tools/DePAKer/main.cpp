#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <BspcLibIncludes.hpp>

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

	dpackheader_t pak_header{};
	ReadFile(*f, 0, sizeof(pak_header), &pak_header);

	const int pak_id = ('P' << 0) | ('A' << 8) | ('C' << 16) | ('K' << 24);
	if(pak_header.ident != pak_id)
	{
		std::cerr << "File is not Quake PACK file." << std::endl;
		std::fclose(f);
		return -1;
	}

	pak_header.dirofs = LittleLong (pak_header.dirofs);
	pak_header.dirlen = LittleLong (pak_header.dirlen);

	const int numpackfiles = pak_header.dirlen / sizeof(dpackfile_t);
	std::cout << "Num PAK files: " << numpackfiles << std::endl;
	if(numpackfiles < 0)
	{
		std::cerr << "Negative number of PAK files." << std::endl;
		std::fclose(f);
		return -1;
	}

	std::vector<dpackfile_t> pak_files;
	pak_files.resize(numpackfiles);
	ReadFile(*f, pak_header.dirofs, pak_files.size() * sizeof(dpackfile_t), pak_files.data());

	std::vector<uint8_t> file_buffer;
	for(const dpackfile_t& pak_file : pak_files)
	{
		std::cout << "Extracting \"" << pak_file.name << "\"." << std::endl;

		const int len = LittleLong(pak_file.filelen);
		const int off = LittleLong(pak_file.filepos);
		file_buffer.resize(len);

		ReadFile(*f, off, len, file_buffer.data());

		const std::string result_file_path = out_dir + "/" + pak_file.name;

		// TODO - create directories here.

		FILE* const out_f = std::fopen(result_file_path.c_str(), "wb");
		if(out_f == nullptr)
		{
			std::cerr << "Can't open \"" << result_file_path << "\" for writing" << std::endl;
			continue;
		}

		WriteFileAll(*out_f, len, file_buffer.data());
		std::fclose(out_f);
	}


	std::fclose(f);
}

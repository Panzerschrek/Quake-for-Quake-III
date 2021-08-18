import argparse
import subprocess
import os
import sys
from shaders import generate_shader_file


g_wad_extractor_executable= "WadExtractor"
g_q1_pic_to_tga_executable= "Q1PicToTga"


def main():
	parser= argparse.ArgumentParser(description= 'Converter script.')
	parser.add_argument("--input-file", help= "input WAD file with Quake textures", type=str)
	parser.add_argument("--output-dir", help= "output directory", type=str)
	parser.add_argument("--output-shader-file", help= "output shader file", type=str)

	args= parser.parse_args()

	output_dir = args.output_dir

	if output_dir[-1] == '/' or output_dir[-1] == '\\':
		output_dir_intermediate = output_dir[0:len(output_dir)-1] + "_intermediate"
	else:
		output_dir_intermediate = output_dir + "_intermediate"

	os.makedirs(output_dir, exist_ok= True)
	os.makedirs(output_dir_intermediate, exist_ok= True)

	# Extract all files into intermediate directory.
	subprocess.call([g_wad_extractor_executable, "-i", args.input_file, "-o", output_dir_intermediate])

	# Convert picture files into TGA (ignore palette file)
	palette_file_name = os.path.join(output_dir_intermediate, "PALETTE")
	#palette_file_name = "gfx/palette.lmp"
	for file_name in os.listdir(output_dir_intermediate):
		if file_name == "PALETTE":
			continue

		file_path_in = os.path.join(output_dir_intermediate, file_name)
		file_path_base_out= os.path.join(output_dir, file_name)
		subprocess.call([g_q1_pic_to_tga_executable, "-i", file_path_in, "-o", file_path_base_out, "-p", palette_file_name])

	generate_shader_file(output_dir, args.output_shader_file)

	return 0

if __name__ == "__main__":
	sys.exit(main())

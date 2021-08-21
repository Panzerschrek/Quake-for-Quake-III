import argparse
import subprocess
import os
import sys


g_q1_lmp_pic_to_tga_executable= "Q1LmpPicToTga"

def main():
	parser= argparse.ArgumentParser(description= 'Converter script.')
	parser.add_argument("--input-dir", help= "input directory with Quake models", type=str)
	parser.add_argument("--output-dir", help= "output directory for models/textures", type=str)

	args= parser.parse_args()

	input_dir = args.input_dir
	output_dir = args.output_dir

	os.makedirs(output_dir, exist_ok= True)

	palette_file_name = os.path.join(input_dir, "palette.lmp")

	for file_name in os.listdir(input_dir):
		in_file = os.path.join(input_dir, file_name)
		if in_file == palette_file_name:
			continue;

		out_file = os.path.join(output_dir, file_name.replace(".lmp", ".tga"))

		subprocess.call([g_q1_lmp_pic_to_tga_executable, "-i", in_file, "-o", out_file, "-p", palette_file_name])

	return 0

if __name__ == "__main__":
	sys.exit(main())

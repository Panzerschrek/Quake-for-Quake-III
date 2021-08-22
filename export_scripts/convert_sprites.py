import argparse
import subprocess
import os
import sys
from shaders import generate_sprites_shader_file


g_sprites_converter_executable= "SpriteConverter"

def main():
	parser= argparse.ArgumentParser(description= 'Converter script.')
	parser.add_argument("--input-dir", help= "input directory with Quake sprites", type=str)
	parser.add_argument("--output-dir", help= "output directory for sprite images", type=str)
	parser.add_argument("--output-shader-file", help= "output shader file", type=str)

	args= parser.parse_args()

	input_dir = args.input_dir
	output_dir = args.output_dir

	os.makedirs(output_dir, exist_ok= True)

	palette_file = os.path.join(input_dir, "palette.lmp")

	for file_name in os.listdir(input_dir):
		in_file = os.path.join(input_dir, file_name)

		if file_name == "palette.lmp":
			continue

		out_file_base = os.path.join(output_dir, file_name.replace(".spr", ""))
		subprocess.call([g_sprites_converter_executable, "-i", in_file, "-o", out_file_base, "-p" , palette_file])

	generate_sprites_shader_file(output_dir, args.output_shader_file)

	return 0

if __name__ == "__main__":
	sys.exit(main())

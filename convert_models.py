import argparse
import subprocess
import os
import sys
from shaders import generate_models_shader_file


g_models_converter_executable= "qwalk_converter"

def main():
	parser= argparse.ArgumentParser(description= 'Converter script.')
	parser.add_argument("--input-dir", help= "input directory with Quake models", type=str)
	parser.add_argument("--output-dir", help= "output directory for models/textures", type=str)
	parser.add_argument("--output-shader-file", help= "output shader file", type=str)

	args= parser.parse_args()

	input_dir = args.input_dir
	output_dir = args.output_dir
	textures_dir = "textures"

	os.makedirs(output_dir, exist_ok= True)
	os.makedirs(textures_dir, exist_ok= True)

	for file_name in os.listdir(input_dir):
		in_file = os.path.join(input_dir, file_name)
		out_file = os.path.join(output_dir, file_name.replace(".mdl", ".md3"))
		out_texture_file = os.path.join(textures_dir, file_name.replace(".mdl", ""))
		subprocess.call([g_models_converter_executable, "-i", in_file, "-outtex", out_texture_file, "-force", out_file])

	generate_models_shader_file(textures_dir, args.output_shader_file)

	return 0

if __name__ == "__main__":
	sys.exit(main())

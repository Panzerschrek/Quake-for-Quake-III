import argparse
import subprocess
import os
import sys


g_wad_extractor_executable= "WadExtractor"
g_q1_pic_to_tga_executable= "Q1PicToTga"


shader_base_template = """
"textures/%(shader_name)"
{
	{
		map $lightmap
		rgbGen identity
	}
	{
		map "textures/%(file_name)"
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
}
"""

shader_with_fullbrights_template = """
"textures/%(shader_name)"
{
	{
		map $lightmap
		rgbGen identity
	}
	{
		map "textures/%(file_name)"
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map "textures/%(fullbrights_file_name)"
		blendFunc GL_ONE GL_ONE
		rgbGen identity
	}
}"""

shader_water_template = """
"textures/%(shader_name)"
{
	q3map_globaltexture
	q3map_tessSize 96
	surfaceparm noimpact
	surfaceparm nolightmap
	surfaceparm nonsolid
	surfaceparm trans
	surfaceparm water
	cull disable
	{
		map "textures/%(file_name)"
		tcmod turb 0 0.125 0.0 0.125
		blendFunc GL_ONE GL_ZERO
		rgbGen identity
	}
}"""

def generate_shader_file(tga_textures_dir, out_shader_file):

	with open(out_shader_file, mode = "w") as f:
		for file_name in os.listdir(tga_textures_dir):

			file_name_without_extension = file_name.replace(".tga", "")
			fullbrights_file_name = file_name_without_extension + "_fb.tga"

			if file_name.endswith("_fb.tga") :
				continue # ignore fullbright textures
			elif file_name.startswith("*") and file_name.find("WATER") != -1:
				shader_descr = shader_water_template.replace("%(shader_name)", file_name_without_extension).replace("%(file_name)", file_name)
			elif os.path.exists(os.path.join(tga_textures_dir, fullbrights_file_name)):
				shader_descr = shader_with_fullbrights_template.replace("%(shader_name)", file_name_without_extension).replace("%(file_name)", file_name).replace("%(fullbrights_file_name)", fullbrights_file_name)
			else:
				continue # Ignore regular textures without special effects and fullbrights
			#	shader_descr = shader_base_template.replace("%(shader_name)", file_name_without_extension).replace("%(file_name)", file_name)

			f.write(shader_descr + "\n")


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

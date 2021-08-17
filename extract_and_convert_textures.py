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

shader_slime_template = """
"textures/%(shader_name)"
{
	q3map_globaltexture
	q3map_tessSize 96
	surfaceparm noimpact
	surfaceparm nolightmap
	surfaceparm nonsolid
	surfaceparm trans
	surfaceparm slime
	cull disable
	{
		map "textures/%(file_name)"
		tcmod turb 0 0.125 0.0 0.125
		blendFunc GL_ONE GL_ZERO
		rgbGen identity
	}
}"""

shader_lava_template = """
"textures/%(shader_name)"
{
	q3map_globaltexture
	q3map_tessSize 96
	surfaceparm nodlight
	surfaceparm noimpact
	surfaceparm nolightmap
	surfaceparm nonsolid
	surfaceparm trans
	surfaceparm lava
	cull disable
	{
		map "textures/%(file_name)"
		tcmod turb 0 0.125 0.0 0.125
		blendFunc GL_ONE GL_ZERO
		rgbGen identity
	}
}"""

shader_generic_turb_template = """
"textures/%(shader_name)"
{
	q3map_globaltexture
	q3map_tessSize 96
	surfaceparm noimpact
	surfaceparm nolightmap
	surfaceparm nonsolid
	surfaceparm trans
	cull disable
	{
		map "textures/%(file_name)"
		tcmod turb 0 0.125 0.0 0.125
		blendFunc GL_ONE GL_ZERO
		rgbGen identity
	}
}"""

shader_animated_template = """
"textures/%(shader_name)"
{
	{
		map $lightmap
		rgbGen identity
	}
	{
		animMap 2 %(files_list)
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
}
"""

shader_animated_fullbright_template = """
"textures/%(shader_name)"
{
	{
		map $lightmap
		rgbGen identity
	}
	{
		animMap 2 %(files_list)
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		animMap 2 %(fullbright_files_list)
		blendFunc GL_ONE GL_ONE
		rgbGen identity
	}
}
"""
def generate_shader_file(tga_textures_dir, out_shader_file):

	with open(out_shader_file, mode = "w") as f:
		for file_name in os.listdir(tga_textures_dir):

			file_name_without_extension = file_name.replace(".tga", "")
			fullbrights_file_name = file_name_without_extension + "_fb.tga"

			shader_descr = ""
			if file_name.endswith("_fb.tga"):
				continue # ignore fullbright textures
			elif file_name.startswith("SKY"): # ignore skies for now
				continue
			elif file_name.startswith("*"):
				if file_name.find("WATER") != -1:
					shader_descr = shader_water_template.replace("%(shader_name)", file_name_without_extension).replace("%(file_name)", file_name)
				elif file_name.find("SLIME") != -1:
					shader_descr = shader_slime_template.replace("%(shader_name)", file_name_without_extension).replace("%(file_name)", file_name)
				elif file_name.find("LAVA") != -1:
					shader_descr = shader_lava_template.replace("%(shader_name)", file_name_without_extension).replace("%(file_name)", file_name)
				else:
					shader_descr = shader_generic_turb_template.replace("%(shader_name)", file_name_without_extension).replace("%(file_name)", file_name)
			elif file_name.startswith("+"):
				# animated texture

				if file_name[1] >= '0' and file_name[1] <= '9':
					frame_base_symbols = "0123456789"
				else:
					frame_base_symbols = "ABCDEFGHIJ"

				frames_list = []
				fulbright_frames_list = []
				have_any_fullbright_frame = False
				for i in range(10):
					frame_file_name = "+" + frame_base_symbols[i] + file_name[2:]

					if os.path.exists(os.path.join(tga_textures_dir, frame_file_name)):
						frames_list.append(frame_file_name)
						fullbright_frame_file_name = "+" + str(i) + file_name_without_extension[2:] + "_fb.tga"
						if os.path.exists(os.path.join(tga_textures_dir, fullbright_frame_file_name)):
							fulbright_frames_list.append(fullbright_frame_file_name)
							have_any_fullbright_frame = True
						else:
							fulbright_frames_list.append("blackimage.tha") # This frame have no fullbright image, so, replace it with dummy black image.

				if len(frames_list) > 1:
					if have_any_fullbright_frame:
						# Generate animation with both regular and fullbright frames
						files_list = []
						for frame in frames_list:
							files_list.append("\"textures/" + frame + "\"")
						fullbright_files_list = []
						for frame in fulbright_frames_list:
							fullbright_files_list.append("\"textures/" + frame + "\"")
						shader_descr = shader_animated_fullbright_template.replace("%(shader_name)", file_name_without_extension).replace("%(files_list)", " ".join(files_list)).replace("%(fullbright_files_list)", " ".join(fullbright_files_list))
					else:
						# Generate regular animation
						files_list = []
						for frame in frames_list:
							files_list.append("\"textures/" + frame + "\"")
						shader_descr = shader_animated_template.replace("%(shader_name)", file_name_without_extension).replace("%(files_list)", " ".join(files_list))
					f.write(shader_descr + "\n")
					continue

			# Not a sky, turb or animation - try to create shader with only fullbright component.
			if shader_descr == "" and os.path.exists(os.path.join(tga_textures_dir, fullbrights_file_name)):
				shader_descr = shader_with_fullbrights_template.replace("%(shader_name)", file_name_without_extension).replace("%(file_name)", file_name).replace("%(fullbrights_file_name)", fullbrights_file_name)

			# Do not create shader for regular textures without turb, animation, fullbright pixels.
			if shader_descr != "":
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

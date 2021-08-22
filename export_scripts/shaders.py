import os

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
}
"""

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
}
"""

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
}
"""

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
}
"""

shader_lava_fullbright_template = """
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
	{
		map "textures/%(fullbrights_file_name)"
		tcmod turb 0 0.125 0.0 0.125
		blendFunc GL_ONE GL_ONE
		rgbGen identity
	}
}
"""

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
}
"""

shader_for_models_with_fullbrights_template = """
"models_textures/%(shader_name)"
{
	{
		map "models_textures/%(file_name)"
		rgbGen lightingDiffuse
	}
	{
		map "models_textures/%(fullbrights_file_name)"
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

			shader = ""
			if file_name.endswith("_fb.tga"):
				continue # ignore fullbright textures
			elif file_name.startswith("SKY"): # ignore skies for now
				continue
			elif file_name.startswith("*"):
				if file_name.find("WATER") != -1:
					shader = shader_water_template.replace("%(shader_name)", file_name_without_extension).replace("%(file_name)", file_name)
				elif file_name.find("SLIME") != -1:
					shader = shader_slime_template.replace("%(shader_name)", file_name_without_extension).replace("%(file_name)", file_name)
				elif file_name.find("LAVA") != -1:
					if os.path.exists(os.path.join(tga_textures_dir, fullbrights_file_name)):
						shader = shader_lava_fullbright_template.replace("%(shader_name)", file_name_without_extension).replace("%(file_name)", file_name).replace("%(fullbrights_file_name)", fullbrights_file_name)
					else:
						shader = shader_lava_template.replace("%(shader_name)", file_name_without_extension).replace("%(file_name)", file_name)
				else:
					shader = shader_generic_turb_template.replace("%(shader_name)", file_name_without_extension).replace("%(file_name)", file_name)
			elif file_name.startswith("+"):
				# animated texture

				seq_symbols = "0123456789"
				alt_seq_symbols = "ABCDEFGHIJ"

				frames_list = []
				fulbright_frames_list = []
				have_any_fullbright_frame = False
				alternative_frames_list = []
				alternative_fulbright_frames_list = []
				have_any_alternative_fullbright_frame = False
				for i in range(10):
					frame_file_name = "+" + seq_symbols[i] + file_name[2:]

					if os.path.exists(os.path.join(tga_textures_dir, frame_file_name)):
						frames_list.append(frame_file_name)
						fullbright_frame_file_name = "+" + seq_symbols[i] + file_name_without_extension[2:] + "_fb.tga"
						if os.path.exists(os.path.join(tga_textures_dir, fullbright_frame_file_name)):
							fulbright_frames_list.append(fullbright_frame_file_name)
							have_any_fullbright_frame = True
						else:
							fulbright_frames_list.append("blackimage.tga") # This frame have no fullbright image, so, replace it with dummy black image.

					alternative_frame_file_name = "+" + alt_seq_symbols[i] + file_name[2:]

					if os.path.exists(os.path.join(tga_textures_dir, alternative_frame_file_name)):
						alternative_frames_list.append(alternative_frame_file_name)
						fullbright_alternative_frame_file_name = "+" + alt_seq_symbols[i] + file_name_without_extension[2:] + "_fb.tga"
						if os.path.exists(os.path.join(tga_textures_dir, fullbright_alternative_frame_file_name)):
							alternative_fulbright_frames_list.append(fullbright_alternative_frame_file_name)
							have_any_alternative_fullbright_frame = True
						else:
							alternative_fulbright_frames_list.append("blackimage.tga") # This frame have no fullbright image, so, replace it with dummy black image.

				shader = "\n\"textures/" + file_name_without_extension + "\"\n"
				shader += "{\n"

				if len(alternative_frames_list) > 0:
					# Use entity color to swith between regular and alternative textures.

					# Regular frames
					if len(frames_list) == 1:
						shader += "\t{\n\t\tmap \"textures/" + frames_list[0] + "\""
					else:
						shader += "\t{\n\t\tanimMap 2"
						for frame in frames_list:
							shader+= " \"textures/" + frame + "\""
					shader += "\n\t\trgbgen entity\n\t}\n"

					# Alternative frames
					if len(alternative_frames_list) == 1:
						shader += "\t{\n\t\tmap \"textures/" + alternative_frames_list[0] + "\""
					else:
						shader += "\t{\n\t\tanimMap 2"
						for frame in alternative_frames_list:
							shader+= " \"textures/" + frame + "\""
					shader += "\n\t\trgbgen oneminusentity\n\t\tblendFunc GL_ONE GL_ONE\n\t}\n"

					# Lightmap
					shader += "\t{\n\t\tmap $lightmap\n\t\tblendFunc GL_DST_COLOR GL_ZERO\n\t}\n"

					#fullbrights
					if have_any_fullbright_frame:
						if len(fulbright_frames_list) == 1:
							shader += "\t{\n\t\tmap \"textures/" + fulbright_frames_list[0] + "\""
						else:
							shader += "\t{\n\t\tanimMap 2"
							for frame in fulbright_frames_list:
								shader+= " \"textures/" + frame + "\""
						shader += "\n\t\trgbgen entity\n\t\tblendFunc GL_ONE GL_ONE\n\t}\n"

					# Alternative fullbrights
					if have_any_alternative_fullbright_frame:
						if len(alternative_fulbright_frames_list) == 1:
							shader += "\t{\n\t\tmap \"textures/" + alternative_fulbright_frames_list[0] + "\""
						else:
							shader += "\t{\n\t\tanimMap 2"
							for frame in alternative_fulbright_frames_list:
								shader+= " \"textures/" + frame + "\""
						shader += "\n\t\trgbgen oneminusentity\n\t\tblendFunc GL_ONE GL_ONE\n\t}\n"

				else:
					shader += "\t{\n\t\tmap $lightmap\n\t}\n"
					if len(frames_list) == 1:
						shader += "\t{\n\t\tmap \"textures/" + frames_list[0] + "\""
					else:
						shader += "\t{\n\t\tanimMap 2"
						for frame in frames_list:
							shader+= " \"textures/" + frame + "\""
					shader += "\n\t\tblendFunc GL_DST_COLOR GL_ZERO\n\t}\n"

					if have_any_fullbright_frame:
						if len(fulbright_frames_list) == 1:
							shader += "\t{\n\t\tmap \"textures/" + fulbright_frames_list[0] + "\""
						else:
							shader += "\t{\n\t\tanimMap 2"
							for frame in fulbright_frames_list:
								shader+= " \"textures/" + frame + "\""
						shader += "\n\t\tblendFunc GL_ONE GL_ONE\n\t}\n"

				shader += "}\n"

				if len(frames_list) <= 1 and len(alternative_frames_list) == 0 and not have_any_fullbright_frame and not have_any_alternative_fullbright_frame:
					shader = "" # Reset shader if it is not animated, with alternatives or with fullbrights.

			# Not a sky, turb or animation - try to create shader with only fullbright component.
			if shader == "" and os.path.exists(os.path.join(tga_textures_dir, fullbrights_file_name)):
				shader = shader_with_fullbrights_template.replace("%(shader_name)", file_name_without_extension).replace("%(file_name)", file_name).replace("%(fullbrights_file_name)", fullbrights_file_name)

			# Do not create shader for regular textures without turb, animation, fullbright pixels.
			if shader != "":
				f.write(shader)


def generate_models_shader_file(tga_textures_dir, out_shader_file):

	with open(out_shader_file, mode = "w") as f:
		for file_name in os.listdir(tga_textures_dir):

			file_name_without_extension = file_name.replace(".tga", "")
			fullbrights_file_name = file_name_without_extension + "_fb.tga"

			shader = ""
			if file_name.endswith("_fb.tga"):
				continue # ignore fullbright textures
			elif os.path.exists(os.path.join(tga_textures_dir, fullbrights_file_name)):
				shader = shader_for_models_with_fullbrights_template.replace("%(shader_name)", file_name_without_extension).replace("%(file_name)", file_name).replace("%(fullbrights_file_name)", fullbrights_file_name)

			# Do not create shader for regular textures without turb, animation, fullbright pixels.
			if shader != "":
				f.write(shader)


def generate_sprites_shader_file(tga_sprites_dir, out_shader_file):

	with open(out_shader_file, mode = "w") as f:
		for file_name in os.listdir(tga_sprites_dir):

			file_name_without_extension = file_name.replace(".tga", "")

			if not file_name.endswith("0.tga"):
				continue # Generate shaders once per group

			shader_name = file_name.replace("0.tga", "")

			shader  = "\"progs/" + shader_name + "\"\n"
			shader += "{\n"
			shader += "\t{\n"
			shader += "\t{\n\t\tanimMap 4"
			for i in range(10):
				frame_name = shader_name + str(i) + ".tga"
				if os.path.exists(os.path.join(tga_sprites_dir, frame_name)):
					shader+= " \"sprites/" + frame_name + "\""

			shader+= "\n"
			shader+= "\t\tblendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA\n"
			shader+= "\t}\n"
			shader+= "}\n\n"

			f.write(shader)

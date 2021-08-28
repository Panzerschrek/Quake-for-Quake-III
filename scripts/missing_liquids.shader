// Shaders for missing in QUAKE101.wad water/slime textures.

"textures/*SLIME0"
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
		map "textures/*SLIME2.tga"
		tcmod turb 0 0.125 0.0 0.125
		blendFunc GL_ONE GL_ZERO
		rgbGen identity
	}
}

"textures/*04WATER1"
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
		map "textures/*WATER1.tga"
		tcmod turb 0 0.125 0.0 0.125
		blendFunc GL_ONE GL_ZERO
		rgbGen identity
	}
}

"textures/*04WATER2"
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
		map "textures/*WATER2.tga"
		tcmod turb 0 0.125 0.0 0.125
		blendFunc GL_ONE GL_ZERO
		rgbGen identity
	}
}

"textures/*04AWATER1"
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
		map "textures/*WATER1.tga"
		tcmod turb 0 0.125 0.0 0.125
		blendFunc GL_ONE GL_ZERO
		rgbGen identity
	}
}

"textures/*04MWAT1"
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
		map "textures/*WATER1.tga"
		tcmod turb 0 0.125 0.0 0.125
		blendFunc GL_ONE GL_ZERO
		rgbGen identity
	}
}

"textures/*04MWAT2"
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
		map "textures/*WATER2.tga"
		tcmod turb 0 0.125 0.0 0.125
		blendFunc GL_ONE GL_ZERO
		rgbGen identity
	}
}

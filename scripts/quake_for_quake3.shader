textures/CLIP
{
	qer_trans 0.40
	surfaceparm nolightmap
	surfaceparm nomarks
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm playerclip
	surfaceparm noimpact
}

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

textures/SKY1
{
	q3map_globaltexture
	surfaceparm noimpact
	surfaceparm nolightmap
	surfaceparm nomarks
	surfaceparm sky
	skyparms - 256 -
	{
		map textures/SKY1_bg.tga
		blendFunc GL_ONE GL_ZERO
		rgbGen identity
		tcMod scale -16 -16
		tcMod scroll 0.0625 0.0625
	}
	{
		map textures/SKY1_fg.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen identity
		tcMod scale -16 -16
		tcMod scroll 0.125 0.125
	}
}

textures/SKY4
{
	q3map_globaltexture
	surfaceparm noimpact
	surfaceparm nolightmap
	surfaceparm nomarks
	surfaceparm sky
	skyparms - 256 -
	{
		map textures/SKY4_bg.tga
		blendFunc GL_ONE GL_ZERO
		tcMod scale -16 -16
		tcMod scroll 0.0625 0.0625
	}
	{
		map textures/SKY4_fg.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen identity
		tcMod scale -16 -16
		tcMod scroll 0.125 0.125
	}
}

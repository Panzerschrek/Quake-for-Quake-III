textures/particle
{
	{
		map textures/particle.tga
		rgbgen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

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

"textures/*LAVA1"
{
	q3map_globaltexture
	q3map_tessSize 96
	surfaceparm noimpact
	surfaceparm lava
	surfaceparm nolightmap
	cull disable
	{
		map "textures/*LAVA1.tga"
		tcmod turb 0 0.125 0.0 0.125
		blendFunc GL_ONE GL_ZERO
		rgbGen identity
	}
}


"textures/*LAVA2"
{
	q3map_globaltexture
	q3map_tessSize 96
	surfaceparm noimpact
	surfaceparm lava
	surfaceparm nolightmap
	cull disable
	{
		map "textures/*LAVA2.tga"
		tcmod turb 0 0.125 0.0 0.125
		blendFunc GL_ONE GL_ZERO
		rgbGen identity
	}
}


"textures/*SLIME0"
{
	q3map_globaltexture
	q3map_tessSize 96
	surfaceparm noimpact
	surfaceparm slime
	surfaceparm nolightmap
	cull disable
	{
		map "textures/*SLIME0.tga"
		tcmod turb 0 0.125 0.0 0.125
		blendFunc GL_ONE GL_ZERO
		rgbGen identity
	}
}


"textures/*SLIME1"
{
	q3map_globaltexture
	q3map_tessSize 96
	surfaceparm noimpact
	surfaceparm slime
	surfaceparm nolightmap
	cull disable
	{
		map "textures/*SLIME1.tga"
		tcmod turb 0 0.125 0.0 0.125
		blendFunc GL_ONE GL_ZERO
		rgbGen identity
	}
}


"textures/*SLIME2"
{
	q3map_globaltexture
	q3map_tessSize 96
	surfaceparm noimpact
	surfaceparm slime
	surfaceparm nolightmap
	cull disable
	{
		map "textures/*SLIME2.tga"
		tcmod turb 0 0.125 0.0 0.125
		blendFunc GL_ONE GL_ZERO
		rgbGen identity
	}
}


"textures/*WATER0"
{
	q3map_globaltexture
	q3map_tessSize 96
	surfaceparm noimpact
	surfaceparm water
	surfaceparm nolightmap
	cull disable
	{
		map "textures/*WATER0.tga"
		tcmod turb 0 0.125 0.0 0.125
		blendFunc GL_ONE GL_ZERO
		rgbGen identity
	}
}


"textures/*WATER1"
{
	q3map_globaltexture
	q3map_tessSize 96
	surfaceparm noimpact
	surfaceparm water
	surfaceparm nolightmap
	cull disable
	{
		map "textures/*WATER1.tga"
		tcmod turb 0 0.125 0.0 0.125
		blendFunc GL_ONE GL_ZERO
		rgbGen identity
	}
}


"textures/*WATER2"
{
	q3map_globaltexture
	q3map_tessSize 96
	surfaceparm noimpact
	surfaceparm water
	surfaceparm nolightmap
	cull disable
	{
		map "textures/*WATER2.tga"
		tcmod turb 0 0.125 0.0 0.125
		blendFunc GL_ONE GL_ZERO
		rgbGen identity
	}
}

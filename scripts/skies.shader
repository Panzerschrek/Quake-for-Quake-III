// Skies of original Quake.
// These shaders requires modified sky textures - with separate backgound and foregraund layers and added alpha for foreground layer.

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
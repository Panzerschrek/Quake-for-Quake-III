/*
    QShed <http://www.icculus.org/qshed>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* TGA loading code hastily ripped from DarkPlaces */
/* TODO - rewrite it to be shorter, speed isn't as important when we're not loading hundreds of them */

#include <string.h>

#include "global.h"
#include "image.h"

image_rgba_t *image_tga_load(mem_pool_t *pool, void *filedata, size_t filesize, char **out_error)
{
	image_rgba_t *image;
	unsigned char *f = (unsigned char*)filedata;
	unsigned char *endf = f + filesize;
	int id_length;
	//FIXME int colormap_type;
	int image_type;
	int colormap_index;
	int colormap_length;
	int colormap_size;
	//FIXME int x_origin;
	//FIXME int y_origin;
	int width;
	int height;
	int pixel_size;
	int attributes;
	bool_t bottom_to_top;
	bool_t compressed;
	int x, y, readpixelcount, red, green, blue, alpha, runlen, row_inc;
	unsigned char palette[256*4], *p;
	unsigned char *pixbuf;

	if (filesize < 19)
		return NULL;

/* load header manually because not all of these values are aligned */
	id_length       = f[0];
	//FIXME colormap_type   = f[1];
	image_type      = f[2];
	colormap_index  = f[3] + f[4] * 256;
	colormap_length = f[5] + f[6] * 256;
	colormap_size   = f[7];
	//FIXME x_origin        = f[8] + f[9] * 256;
	//FIXME y_origin        = f[10] + f[11] * 256;
	width           = f[12] + f[13] * 256;
	height          = f[14] + f[15] * 256;
	pixel_size      = f[16];
	attributes      = f[17];

	f += 18;

	f += id_length; /* skip comment */

	if (width > 4096 || height > 4096 || width <= 0 || height <= 0)
	{
		if (out_error)
			*out_error = msprintf("tga: bad dimensions");
		return NULL;
	}

	if (attributes & 0x10) /* this bit indicates origin on the right, which we don't support */
	{
		if (out_error)
			*out_error = msprintf("tga: bad origin (must be top left or bottom left)");
		return NULL;
	}

/* if bit 5 of attributes isn't set, the image has been stored from bottom to top */
	bottom_to_top = (attributes & 0x20) == 0;

	compressed = false;
	x = 0;
	y = 0;
	red = 255;
	green = 255;
	blue = 255;
	alpha = 255;
	switch (image_type)
	{
	default:
		if (out_error)
			*out_error = msprintf("tga: bad image type");
		return NULL;

	/* colormapped */
	case 9: compressed = true;
	case 1:
		if (pixel_size != 8)
		{
			if (out_error)
				*out_error = msprintf("tga: bad pixel_size");
			return NULL;
		}
		if (colormap_length != 256)
		{
			if (out_error)
				*out_error = msprintf("tga: bad colormap_length");
			return NULL;
		}
		if (colormap_index != 0)
		{
			if (out_error)
				*out_error = msprintf("tga: bad colormap_index");
			return NULL;
		}

		switch (colormap_size)
		{
		default:
			if (out_error)
				*out_error = msprintf("tga: bad colormap_size");
			return NULL;
		case 24:
			for (x = 0; x < colormap_length; x++)
			{
				palette[x*4+2] = *f++;
				palette[x*4+1] = *f++;
				palette[x*4+0] = *f++;
				palette[x*4+3] = 255;
			}
			break;
		case 32:
			for (x = 0; x < colormap_length; x++)
			{
				palette[x*4+2] = *f++;
				palette[x*4+1] = *f++;
				palette[x*4+0] = *f++;
				palette[x*4+3] = *f++;
			}
			break;
		}

		image = image_alloc(pool, width, height);
		if (!image)
		{
			if (out_error)
				*out_error = msprintf("tga: out of memory");
			return NULL;
		}

		if (bottom_to_top)
		{
			pixbuf = image->pixels + (height - 1) * width * 4;
			row_inc = -width * 4 * 2;
		}
		else
		{
			pixbuf = image->pixels;
			row_inc = 0;
		}

		if (compressed)
		{
			while (y < height)
			{
				readpixelcount = 1000000;
				runlen = 1000000;

				if (f < endf)
				{
					runlen = *f++;
				/* high bit indicates this is an RLE compressed run */
					if (runlen & 0x80)
						readpixelcount = 1;
					runlen = 1 + (runlen & 0x7f);
				}

				while ((runlen--) && y < height)
				{
					if (readpixelcount > 0)
					{
						readpixelcount--;

						red = 255;
						green = 255;
						blue = 255;
						alpha = 255;

						if (f < endf)
						{
							p = palette + (*f++) * 4;
							red = p[0];
							green = p[1];
							blue = p[2];
							alpha = p[3];
						}
					}

					pixbuf[0] = red;
					pixbuf[1] = green;
					pixbuf[2] = blue;
					pixbuf[3] = alpha;
					pixbuf += 4;

					if (++x == width)
					{
					/* end of line, advance to next */
						x = 0;
						y++;
						pixbuf += row_inc;
					}
				}
			}
		}
		else
		{
			while (y < height)
			{
				red = 255;
				green = 255;
				blue = 255;
				alpha = 255;

				if (f < endf)
				{
					p = palette + (*f++) * 4;
					red = p[0];
					green = p[1];
					blue = p[2];
					alpha = p[3];
				}

				pixbuf[0] = red;
				pixbuf[1] = green;
				pixbuf[2] = blue;
				pixbuf[3] = alpha;
				pixbuf += 4;

				if (++x == width)
				{
				/* end of line, advance to next */
					x = 0;
					y++;
					pixbuf += row_inc;
				}
			}
		}
		break;

	/* BGR or BGRA */
	case 10: compressed = true;
	case 2:
		if (pixel_size != 24 && pixel_size != 32)
		{
			if (out_error)
				*out_error = msprintf("tga: bad pixel_size");
			return NULL;
		}

		image = image_alloc(pool, width, height);
		if (!image)
		{
			if (out_error)
				*out_error = msprintf("tga: out of memory");
			return NULL;
		}

		if (bottom_to_top)
		{
			pixbuf = image->pixels + (height - 1) * width * 4;
			row_inc = -width * 4 * 2;
		}
		else
		{
			pixbuf = image->pixels;
			row_inc = 0;
		}

		if (compressed)
		{
			while (y < height)
			{
				readpixelcount = 1000000;
				runlen = 1000000;

				if (f < endf)
				{
					runlen = *f++;
				/* high bit indicates this is an RLE compressed run */
					if (runlen & 0x80)
						readpixelcount = 1;
					runlen = 1 + (runlen & 0x7f);
				}

				while ((runlen--) && y < height)
				{
					if (readpixelcount > 0)
					{
						readpixelcount--;

						blue  = (f < endf) ? *f++ : 255;
						green = (f < endf) ? *f++ : 255;
						red   = (f < endf) ? *f++ : 255;
						alpha = (f < endf && pixel_size == 32) ? *f++ : 255;
					}

					pixbuf[0] = red;
					pixbuf[1] = green;
					pixbuf[2] = blue;
					pixbuf[3] = alpha;
					pixbuf += 4;

					if (++x == width)
					{
					/* end of line, advance to next */
						x = 0;
						y++;
						pixbuf += row_inc;
					}
				}
			}
		}
		else
		{
			while (y < height)
			{
				pixbuf[2] = (f < endf) ? *f++ : 255;
				pixbuf[1] = (f < endf) ? *f++ : 255;
				pixbuf[0] = (f < endf) ? *f++ : 255;
				pixbuf[3] = (f < endf && pixel_size == 32) ? *f++ : 255;
				pixbuf += 4;

				if (++x == width)
				{
				/* end of line, advance to next */
					x = 0;
					y++;
					pixbuf += row_inc;
				}
			}
		}
		break;

	/* greyscale */
	case 11: compressed = true;
	case 3:
		if (pixel_size != 8)
		{
			if (out_error)
				*out_error = msprintf("tga: bad pixel_size");
			return NULL;
		}

		image = image_alloc(pool, width, height);
		if (!image)
		{
			if (out_error)
				*out_error = msprintf("tga: out of memory");
			return NULL;
		}

		if (bottom_to_top)
		{
			pixbuf = image->pixels + (height - 1) * width * 4;
			row_inc = -width * 4 * 2;
		}
		else
		{
			pixbuf = image->pixels;
			row_inc = 0;
		}

		if (compressed)
		{
			while (y < height)
			{
				readpixelcount = 1000000;
				runlen = 1000000;

				if (f < endf)
				{
					runlen = *f++;
				/* high bit indicates this is an RLE compressed run */
					if (runlen & 0x80)
						readpixelcount = 1;
					runlen = 1 + (runlen & 0x7f);
				}

				while ((runlen--) && y < height)
				{
					if (readpixelcount > 0)
					{
						readpixelcount--;

						red = (f < endf) ? *f++ : 255;
					}

					*pixbuf++ = red;

					if (++x == width)
					{
					/* end of line, advance to next */
						x = 0;
						y++;
						pixbuf += row_inc;
					}
				}
			}
		}
		else
		{
			while (y < height)
			{
				*pixbuf++ = (f < endf) ? *f++ : 255;

				if (++x == width)
				{
				/* end of line, advance to next */
					x = 0;
					y++;
					pixbuf += row_inc;
				}
			}
		}
		break;
	}

	return image;
}

bool_t image_tga_save(const image_rgba_t *image, xbuf_t *xbuf, char **out_error)
{
	bool_t hasalpha;
	unsigned char header[18];
	int x, y, i;
	int runlen;

/* see if the TGA should contain an alpha channel */
	hasalpha = false;
	for (i = 0; i < image->width * image->height; i++)
		if (image->pixels[i*4+3] != 0xff)
			hasalpha = true;

/* write header */
	memset(header, 0, 18);
	header[2] = 10; /* compressed BGR(A) */
	header[12] = image->width & 0xff;
	header[13] = (image->width >> 8) & 0xff;
	header[14] = image->height & 0xff;
	header[15] = (image->height >> 8) & 0xff;
	header[16] = hasalpha ? 32 : 24;
	header[17] = hasalpha ? 8 : 0;

	xbuf_write_data(xbuf, 18, header);

/* don't let runs span multiple lines, because apparently that's against the specs */
	for (y = 0; y < image->height; y++)
	{
		const unsigned char *pix = image->pixels + (image->height - 1 - y) * image->width * 4; /* store bottom to top */

		for (x = 0; x < image->width; x += runlen)
		{
		/* count how many identical pixels in a row */
			for (runlen = 1; runlen < 128 && x + runlen < image->width; runlen++)
				if (pix[x*4+0] != pix[(x+runlen)*4+0] || pix[x*4+1] != pix[(x+runlen)*4+1] || pix[x*4+2] != pix[(x+runlen)*4+2] || pix[x*4+3] != pix[(x+runlen)*4+3])
					break;

			if (runlen == 1)
			{
			/* count how many unique pixels in a row */
				for (runlen = 1; runlen < 128 && x + runlen < image->width; runlen++)
					if (pix[(x+runlen-1)*4+0] == pix[(x+runlen)*4+0] && pix[(x+runlen-1)*4+1] == pix[(x+runlen)*4+1] && pix[(x+runlen-1)*4+2] == pix[(x+runlen)*4+2] && pix[(x+runlen-1)*4+3] == pix[(x+runlen)*4+3])
						break;
				if (runlen > 1)
					runlen--; /* chop the last one off, since it's the beginning of a new run */

				xbuf_write_byte(xbuf, runlen - 1);

				for (i = x; i < x + runlen; i++)
				{
					xbuf_write_byte(xbuf, pix[i*4+2]);
					xbuf_write_byte(xbuf, pix[i*4+1]);
					xbuf_write_byte(xbuf, pix[i*4+0]);
					if (hasalpha)
						xbuf_write_byte(xbuf, pix[i*4+3]);
				}
			}
			else
			{
				xbuf_write_byte(xbuf, 0x80 | (runlen - 1));

				xbuf_write_byte(xbuf, pix[x*4+2]);
				xbuf_write_byte(xbuf, pix[x*4+1]);
				xbuf_write_byte(xbuf, pix[x*4+0]);
				if (hasalpha)
					xbuf_write_byte(xbuf, pix[x*4+3]);
			}
		}
	}

/* done */
	return true;
}

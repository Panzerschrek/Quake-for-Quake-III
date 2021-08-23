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

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "global.h"
#include "model.h"

extern const char *g_skinpath;
extern const char *g_skin_base_name;

typedef struct md3_vertex_s
{
	short origin[3];
	short normalpitchyaw;
} md3_vertex_t;

typedef struct md3_frameinfo_s
{
	float mins[3];
	float maxs[3];
	float origin[3];
	float radius;
	char name[16];
} md3_frameinfo_t;

typedef struct md3_tag_s
{
	char name[64];
	float origin[3];
	float rotationmatrix[9];
} md3_tag_t;

typedef struct md3_shader_s
{
	char name[64];
	int shadernum; /* not used by the disk format */
} md3_shader_t;

typedef struct md3_header_s
{
	char ident[4]; /* "IDP3" */
	int version; /* 15 */

	char name[64];

	int flags; /* unused by quake3, darkplaces uses it for quake-style modelflags (rocket trails, etc.) */

	int num_frames;
	int num_tags;
	int num_meshes;
	int num_skins; /* apparently unused */

/* lump offsets are relative to start of header (start of file) */
	int lump_frameinfo;
	int lump_tags;
	int lump_meshes;
	int lump_end;
} md3_header_t;

typedef struct md3_mesh_s
{
	char ident[4]; /* "IDP3" */

	char name[64];

	int flags; /* unused */

	int num_frames;
	int num_shaders;
	int num_vertices;
	int num_triangles;

/* lump offsets are relative to start of mesh */
	int lump_elements;
	int lump_shaders;
	int lump_texcoords;
	int lump_framevertices;
	int lump_end;
} md3_mesh_t;

bool_t model_md3_load(void *filedata, size_t filesize, model_t *out_model, char **out_error)
{
	unsigned char *f = (unsigned char*)filedata;
	mem_pool_t *pool;
	md3_header_t *header;
	int i, j;
	model_t model;

	header = (md3_header_t*)f;

/* validate format */
	if (memcmp(header->ident, "IDP3", 4))
		return (void)(out_error && (*out_error = msprintf("wrong format (not IDP3)"))), false;
	if (LittleLong(header->version) != 15)
		return (void)(out_error && (*out_error = msprintf("wrong format (version not 6)"))), false;

	pool = mem_create_pool();

/* byteswap header */
	header->version        = LittleLong(header->version);
	header->flags          = LittleLong(header->flags);
	header->num_frames     = LittleLong(header->num_frames);
	header->num_tags       = LittleLong(header->num_tags);
	header->num_meshes     = LittleLong(header->num_meshes);
	header->num_skins      = LittleLong(header->num_skins);
	header->lump_frameinfo = LittleLong(header->lump_frameinfo);
	header->lump_tags      = LittleLong(header->lump_tags);
	header->lump_meshes    = LittleLong(header->lump_meshes);
	header->lump_end       = LittleLong(header->lump_end);

/* read skins */
	model.total_skins = 0;
	model.num_skins = 0;
	model.skininfo = NULL;

/* read frames */
	model.total_frames = header->num_frames;

	model.num_frames = header->num_frames;
	model.frameinfo = (frameinfo_t*)mem_alloc(pool, sizeof(frameinfo_t) * model.num_frames);

	f = (unsigned char*)filedata + header->lump_frameinfo;
	for (i = 0; i < model.num_frames; i++)
	{
		md3_frameinfo_t *md3_frameinfo = (md3_frameinfo_t*)f;

		model.frameinfo[i].frametime = 0.1f;
		model.frameinfo[i].num_frames = 1;
		model.frameinfo[i].frames = (singleframe_t*)mem_alloc(pool, sizeof(singleframe_t));
		model.frameinfo[i].frames[0].name = mem_copystring(pool, md3_frameinfo->name);
		model.frameinfo[i].frames[0].offset = i;

		f += sizeof(md3_frameinfo_t);
	}

/* read tags */
	model.num_tags = header->num_tags;
	model.tags = (tag_t*)mem_alloc(pool, sizeof(tag_t) * model.num_tags);

	f = (unsigned char*)filedata + header->lump_tags;
	for (i = 0; i < header->num_tags; i++)
		model.tags[i].matrix = (mat4x4f_t*)mem_alloc(pool, sizeof(mat4x4f_t) * model.num_frames);
	for (i = 0; i < model.num_frames; i++)
	{
		for (j = 0; j < header->num_tags; j++)
		{
			md3_tag_t *md3_tag = (md3_tag_t*)f;
			tag_t *tag = &model.tags[j];

			if (i == 0)
				tag->name = mem_copystring(pool, md3_tag->name);

			tag->matrix[i].m[0][0] = LittleFloat(md3_tag->rotationmatrix[0]);
			tag->matrix[i].m[0][1] = LittleFloat(md3_tag->rotationmatrix[3]);
			tag->matrix[i].m[0][2] = LittleFloat(md3_tag->rotationmatrix[6]);
			tag->matrix[i].m[0][3] = LittleFloat(md3_tag->origin[0]);
			tag->matrix[i].m[1][0] = LittleFloat(md3_tag->rotationmatrix[1]);
			tag->matrix[i].m[1][1] = LittleFloat(md3_tag->rotationmatrix[4]);
			tag->matrix[i].m[1][2] = LittleFloat(md3_tag->rotationmatrix[7]);
			tag->matrix[i].m[1][3] = LittleFloat(md3_tag->origin[1]);
			tag->matrix[i].m[2][0] = LittleFloat(md3_tag->rotationmatrix[2]);
			tag->matrix[i].m[2][1] = LittleFloat(md3_tag->rotationmatrix[5]);
			tag->matrix[i].m[2][2] = LittleFloat(md3_tag->rotationmatrix[8]);
			tag->matrix[i].m[2][3] = LittleFloat(md3_tag->origin[2]);
			tag->matrix[i].m[3][0] = 0;
			tag->matrix[i].m[3][1] = 0;
			tag->matrix[i].m[3][2] = 0;
			tag->matrix[i].m[3][3] = 1;

			f += sizeof(md3_tag_t);
		}
	}

/* read meshes */
	model.num_meshes = header->num_meshes;
	model.meshes = (mesh_t*)mem_alloc(pool, sizeof(mesh_t) * model.num_meshes);

	f = (unsigned char*)filedata + header->lump_meshes;
	for (i = 0; i < header->num_meshes; i++)
	{
		md3_mesh_t *md3_mesh = (md3_mesh_t*)f;
		mesh_t *mesh = &model.meshes[i];
		md3_vertex_t *md3_vertex;

		md3_mesh->flags              = LittleLong(md3_mesh->flags);
		md3_mesh->num_frames         = LittleLong(md3_mesh->num_frames);
		md3_mesh->num_shaders        = LittleLong(md3_mesh->num_shaders);
		md3_mesh->num_vertices       = LittleLong(md3_mesh->num_vertices);
		md3_mesh->num_triangles      = LittleLong(md3_mesh->num_triangles);
		md3_mesh->lump_elements      = LittleLong(md3_mesh->lump_elements);
		md3_mesh->lump_shaders       = LittleLong(md3_mesh->lump_shaders);
		md3_mesh->lump_texcoords     = LittleLong(md3_mesh->lump_texcoords);
		md3_mesh->lump_framevertices = LittleLong(md3_mesh->lump_framevertices);
		md3_mesh->lump_end           = LittleLong(md3_mesh->lump_end);

/*		printf("mesh %d: \"%s\"\n", i, md3_mesh->name);*/

		mesh_initialize(&model, mesh);

		mesh->name = mem_copystring(pool, md3_mesh->name);

		mesh->num_vertices = md3_mesh->num_vertices;
		mesh->num_triangles = md3_mesh->num_triangles;

	/* load triangles */
		mesh->triangle3i = (int*)mem_alloc(pool, sizeof(int) * mesh->num_triangles * 3);

		for (j = 0; j < mesh->num_triangles; j++)
		{
			const int *p = (const int*)(f + md3_mesh->lump_elements) + j * 3;
			mesh->triangle3i[j*3+0] = LittleLong(p[0]);
			mesh->triangle3i[j*3+1] = LittleLong(p[1]);
			mesh->triangle3i[j*3+2] = LittleLong(p[2]);
		}

	/* load texcoords */
		mesh->texcoord2f = (float*)mem_alloc(pool, sizeof(float) * mesh->num_vertices * 2);

		for (j = 0; j < mesh->num_vertices; j++)
		{
			const float *p = (const float*)(f + md3_mesh->lump_texcoords) + j * 2;
			mesh->texcoord2f[j*2+0] = LittleFloat(p[0]);
			mesh->texcoord2f[j*2+1] = LittleFloat(p[1]);
		}

	/* load frames */
		mesh->vertex3f = (float*)mem_alloc(pool, sizeof(float) * model.num_frames * mesh->num_vertices * 3);
		mesh->normal3f = (float*)mem_alloc(pool, sizeof(float) * model.num_frames * mesh->num_vertices * 3);

		for (j = 0, md3_vertex = (md3_vertex_t*)(f + md3_mesh->lump_framevertices); j < model.num_frames * mesh->num_vertices; j++, md3_vertex++)
		{
			double npitch, nyaw;

			mesh->vertex3f[j*3+0] = (signed short)LittleShort(md3_vertex->origin[0]) * (1.0f / 64.0f);
			mesh->vertex3f[j*3+1] = (signed short)LittleShort(md3_vertex->origin[1]) * (1.0f / 64.0f);
			mesh->vertex3f[j*3+2] = (signed short)LittleShort(md3_vertex->origin[2]) * (1.0f / 64.0f);

		/* decompress the vertex normal */
			md3_vertex->normalpitchyaw = LittleShort(md3_vertex->normalpitchyaw);
			npitch = (md3_vertex->normalpitchyaw & 255) * (2 * M_PI) / 256.0;
			nyaw = ((md3_vertex->normalpitchyaw >> 8) & 255) * (2 * M_PI) / 256.0;

			mesh->normal3f[j*3+0] = (float)(sin(npitch) * cos(nyaw));
			mesh->normal3f[j*3+1] = (float)(sin(npitch) * sin(nyaw));
			mesh->normal3f[j*3+2] = (float)cos(npitch);
		}

	/* load shaders */
/*		for (j = 0; j < md3_mesh->num_shaders; j++)
		{
			md3_shader_t *md3_shader = (md3_shader_t*)(f + md3_mesh->lump_shaders) + j;

			printf(" - shader %d: \"%s\"\n", j, md3_shader->name);
		}*/

	/* load skin */
		mesh->skins = NULL;

		f += md3_mesh->lump_end;
	}

	mem_merge_pool(pool);

	model.flags = header->flags;
	model.synctype = 0;
	model.offsets[0] = 0.0f;
	model.offsets[1] = 0.0f;
	model.offsets[2] = 0.0f;

	*out_model = model;
	return true;
}

static unsigned short md3_encodenormal(const float n[3])
{
	int blat, blng;

/* check for singularities */
	if (n[0] == 0 && n[1] == 0)
	{
		if (n[2] > 0)
		{
			blat = 0;
			blng = 0;
		}
		else
		{
			blat = 0;
			blng = 128;
		}
	}
	else
	{
		blat = (int)(atan2(n[1], n[0]) * (255.0 / (2 * M_PI)));
		blng = (int)(acos(n[2]) * (255.0 / (2 * M_PI)));
	}

	return ((blat & 0xff) << 8) | (blng & 0xff);
}

static char *md3_create_skin_filename(const char *skinname)
{
	char temp[1024];
	char *c;

/* in case skinname is already a file path, strip path and extension (so "models/something/skin.pcx" becomes "skin") */
	if ((c = strrchr(skinname, '/')))
		strcpy(temp, c + 1);
	else
		strcpy(temp, skinname);

	if ((c = strchr(temp, '.')))
		*c = '\0';

	if (g_skinpath && g_skinpath[0])
		return msprintf("%s/%s.tga", g_skinpath, temp);
	else
		return msprintf("%s.tga", temp);
}

bool_t model_md3_save(const model_t *model, xbuf_t *xbuf, char **out_error)
{
	md3_header_t header;
	const frameinfo_t *frameinfo;
	char **skinshaders;
	const tag_t *tag;
	const mesh_t *mesh;
	int i, j, k, m, n;

	memcpy(header.ident, "IDP3", 4);
	header.version    = LittleLong(15);
	strlcpy(header.name, "md3model", sizeof(header.name));
	header.flags      = LittleLong(model->flags);
	header.num_frames = LittleLong(model->total_frames); // Export all frames (include all frames inside frame groups).
	header.num_tags   = LittleLong(model->num_tags);
	header.num_meshes = LittleLong(model->num_meshes);
	header.num_skins  = LittleLong(model->num_skins);

/* create 32-bit skins and save them to TGA files */
	skinshaders = (char**)qmalloc(sizeof(char*) * model->num_skins);
	for (i = 0; i < model->num_skins; i++)
	{
		skinshaders[i] = model->num_skins == 1 ? msprintf("%s", g_skin_base_name) : msprintf("%s%d", g_skin_base_name, i);

		char skin_image[1024];
		snprintf(skin_image, sizeof(skin_image), "%s.tga", skinshaders[i]);
		image_save(skin_image, model->meshes[0].skins[i].components[SKIN_DIFFUSE], out_error);

		if (model->meshes[0].skins[i].components[SKIN_FULLBRIGHT]->num_nonempty_pixels > 0)
		{
			snprintf(skin_image, sizeof(skin_image), "%s_fb.tga", skinshaders[i]);
			image_save(skin_image, model->meshes[0].skins[i].components[SKIN_FULLBRIGHT], out_error);
		}
	}

/* calculate lump offsets */
	i = sizeof(md3_header_t);

	header.lump_frameinfo = LittleLong(i);
	i += sizeof(md3_frameinfo_t) * model->total_frames;

	header.lump_tags = LittleLong(i);
	i += sizeof(md3_tag_t) * model->total_frames * model->num_tags;

	header.lump_meshes = i;
	for (j = 0, mesh = model->meshes; j < model->num_meshes; j++, mesh++)
	{
		i += sizeof(md3_mesh_t); /* mesh header */
		i += mesh->num_triangles * sizeof(int[3]); /* triangle elements */
		i += model->num_skins * sizeof(md3_shader_t);
		i += mesh->num_vertices * sizeof(float[2]); /* texcoords */
		i += mesh->num_vertices * model->total_frames * sizeof(md3_vertex_t); /* framevertices */
	}

	header.lump_end = i;

/* write header */
	xbuf_write_data(xbuf, sizeof(md3_header_t), &header);

/* write frameinfo */
	for (i = 0, frameinfo = model->frameinfo; i < model->num_frames; i++, frameinfo++)
	for( n = 0; n < frameinfo->num_frames; n++ )
	{
		const singleframe_t *singleframe = &frameinfo->frames[n];
		md3_frameinfo_t md3_frameinfo;
		float mins[3], maxs[3], dist[3];
		bool_t first = true;

		VectorClear(mins);
		VectorClear(maxs);

		for (j = 0, mesh = model->meshes; j < model->num_meshes; j++, mesh++)
		{
			const float *v = mesh->vertex3f + mesh->num_vertices * singleframe->offset * 3;

			for (k = 0; k < mesh->num_vertices; k++, v += 3)
			{
				for (m = 0; m < 3; m++)
				{
					mins[m] = first ? v[m] : min(mins[m], v[m]);
					maxs[m] = first ? v[m] : max(maxs[m], v[m]);
					first = false;
				}
			}
		}

		for (m = 0; m < 3; m++)
			dist[m] = (fabs(mins[m]) > fabs(maxs[m])) ? mins[m] : maxs[m];

		VectorCopy(md3_frameinfo.mins, mins);
		VectorCopy(md3_frameinfo.maxs, maxs);
		VectorClear(md3_frameinfo.origin);
		md3_frameinfo.radius = (float)sqrt(DotProduct(dist, dist));
		strlcpy(md3_frameinfo.name, singleframe->name, sizeof(md3_frameinfo.name));

		xbuf_write_data(xbuf, sizeof(md3_frameinfo_t), &md3_frameinfo);
	}

/* write tags */
	for (i = 0, frameinfo = model->frameinfo; i < model->total_frames; i++, frameinfo++)
	{
		for (j = 0, tag = model->tags; j < model->num_tags; j++, tag++)
		{
			int ofs = frameinfo->frames[0].offset;
			md3_tag_t md3_tag;

			strlcpy(md3_tag.name, tag->name, sizeof(md3_tag.name));

			md3_tag.rotationmatrix[0] = LittleFloat(tag->matrix[ofs].m[0][0]);
			md3_tag.rotationmatrix[3] = LittleFloat(tag->matrix[ofs].m[0][1]);
			md3_tag.rotationmatrix[6] = LittleFloat(tag->matrix[ofs].m[0][2]);
			md3_tag.origin[0] = LittleFloat(tag->matrix[ofs].m[0][3]);
			md3_tag.rotationmatrix[1] = LittleFloat(tag->matrix[ofs].m[1][0]);
			md3_tag.rotationmatrix[4] = LittleFloat(tag->matrix[ofs].m[1][1]);
			md3_tag.rotationmatrix[7] = LittleFloat(tag->matrix[ofs].m[1][2]);
			md3_tag.origin[1] = LittleFloat(tag->matrix[ofs].m[1][3]);
			md3_tag.rotationmatrix[2] = LittleFloat(tag->matrix[ofs].m[2][0]);
			md3_tag.rotationmatrix[5] = LittleFloat(tag->matrix[ofs].m[2][1]);
			md3_tag.rotationmatrix[8] = LittleFloat(tag->matrix[ofs].m[2][2]);
			md3_tag.origin[2] = LittleFloat(tag->matrix[ofs].m[2][3]);

			xbuf_write_data(xbuf, sizeof(md3_tag_t), &md3_tag);
		}
	}

/* write meshes */
	for (i = 0, mesh = model->meshes; i < model->num_meshes; i++, mesh++)
	{
		md3_mesh_t md3_mesh;

		memcpy(md3_mesh.ident, "IDP3", 4);
		strlcpy(md3_mesh.name, mesh->name, sizeof(md3_mesh.name));
		md3_mesh.flags = 0; /* unused */

		md3_mesh.num_frames = LittleLong(model->total_frames);
		md3_mesh.num_shaders = model->total_skins;
		md3_mesh.num_vertices = LittleLong(mesh->num_vertices);
		md3_mesh.num_triangles = LittleLong(mesh->num_triangles);

		j = sizeof(md3_mesh_t);

		md3_mesh.lump_elements = j;
		j += mesh->num_triangles * sizeof(int[3]);

		md3_mesh.lump_shaders = j;
		j += sizeof(md3_shader_t) * model->num_skins;

		md3_mesh.lump_texcoords = j;
		j += mesh->num_vertices * sizeof(float[2]);

		md3_mesh.lump_framevertices = j;
		j += mesh->num_vertices * model->total_frames * sizeof(md3_vertex_t);

		md3_mesh.lump_end = j;

		xbuf_write_data(xbuf, sizeof(md3_mesh_t), &md3_mesh);

	/* write triangles */
		for (j = 0; j < mesh->num_triangles; j++)
		{
			int triangle[3];

			triangle[0] = LittleLong(mesh->triangle3i[j*3+0]);
			triangle[1] = LittleLong(mesh->triangle3i[j*3+1]);
			triangle[2] = LittleLong(mesh->triangle3i[j*3+2]);

			xbuf_write_data(xbuf, sizeof(triangle), triangle);
		}

	/* write shaders */
		for (j = 0; j < model->total_skins; j++)
		{
			md3_shader_t md3_shader;
			strcpy(md3_shader.name, skinshaders[j]);
			xbuf_write_data(xbuf, sizeof(md3_shader), &md3_shader);
		}

	/* write texcoords */
		for (j = 0; j < mesh->num_vertices; j++)
		{
			float texcoord[2];

			texcoord[0] = LittleFloat(mesh->texcoord2f[j*2+0]);
			texcoord[1] = LittleFloat(mesh->texcoord2f[j*2+1]);

			xbuf_write_data(xbuf, sizeof(texcoord), texcoord);
		}

	/* write framevertices */
		for (j = 0, frameinfo = model->frameinfo; j < model->num_frames; j++, frameinfo++)
		for( n = 0; n < frameinfo->num_frames; n++ )
		{
			const singleframe_t *singleframe = &frameinfo->frames[n];
			const float *v = mesh->vertex3f + singleframe->offset * mesh->num_vertices * 3;
			const float *n = mesh->normal3f + singleframe->offset * mesh->num_vertices * 3;

			for (k = 0; k < mesh->num_vertices; k++, v += 3, n += 3)
			{
				md3_vertex_t md3_vertex;

				int x = (int)(v[0] * 64.0f);
				int y = (int)(v[1] * 64.0f);
				int z = (int)(v[2] * 64.0f);

				md3_vertex.origin[0] = LittleShort(bound(-32768, x, 32767));
				md3_vertex.origin[1] = LittleShort(bound(-32768, y, 32767));
				md3_vertex.origin[2] = LittleShort(bound(-32768, z, 32767));
				md3_vertex.normalpitchyaw = md3_encodenormal(n);

				xbuf_write_data(xbuf, sizeof(md3_vertex_t), &md3_vertex);
			}
		}
	}

	return true;
}

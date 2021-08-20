#include "global.h"
#include "model.h"

#undef copystring
#undef LittleShort
#undef LittleLong
#undef LittleFloat

#define false Q_false
#define true Q_true
#include "l_cmd.h"
#include "l_bsp_q1.h"

extern q1_dheader_t *q1_header;
extern int			q1_fileLength;

extern int Q1_CopyLump (int lump, void *dest, int size, int maxsize);

#undef true
#undef false

static bool_t load_q1bsp_moddel(void *filedata, size_t filesize, char **out_error)
{
	int i;

	//
	// load the file header
	//
	q1_fileLength = filesize;
	q1_header = (q1_dheader_t*)filedata;

// swap the header
	for (i=0 ; i< sizeof(q1_dheader_t)/4 ; i++)
		((int *)q1_header)[i] = LittleLong ( ((int *)q1_header)[i]);

	if (q1_header->version != Q1_BSPVERSION)
	{
		*out_error = msprintf("is version %i, not %i", i, Q1_BSPVERSION);
		return false;
	}

	q1_nummodels = Q1_CopyLump (Q1_LUMP_MODELS, q1_dmodels, sizeof(q1_dmodel_t), Q1_MAX_MAP_MODELS );
	q1_numvertexes = Q1_CopyLump (Q1_LUMP_VERTEXES, q1_dvertexes, sizeof(q1_dvertex_t), Q1_MAX_MAP_VERTS );
	q1_numplanes = Q1_CopyLump (Q1_LUMP_PLANES, q1_dplanes, sizeof(q1_dplane_t), Q1_MAX_MAP_PLANES );
	q1_numleafs = Q1_CopyLump (Q1_LUMP_LEAFS, q1_dleafs, sizeof(q1_dleaf_t), Q1_MAX_MAP_LEAFS );
	q1_numnodes = Q1_CopyLump (Q1_LUMP_NODES, q1_dnodes, sizeof(q1_dnode_t), Q1_MAX_MAP_NODES );
	q1_numtexinfo = Q1_CopyLump (Q1_LUMP_TEXINFO, q1_texinfo, sizeof(q1_texinfo_t), Q1_MAX_MAP_TEXINFO );
	q1_numclipnodes = Q1_CopyLump (Q1_LUMP_CLIPNODES, q1_dclipnodes, sizeof(q1_dclipnode_t), Q1_MAX_MAP_CLIPNODES );
	q1_numfaces = Q1_CopyLump (Q1_LUMP_FACES, q1_dfaces, sizeof(q1_dface_t), Q1_MAX_MAP_FACES );
	q1_nummarksurfaces = Q1_CopyLump (Q1_LUMP_MARKSURFACES, q1_dmarksurfaces, sizeof(q1_dmarksurfaces[0]), Q1_MAX_MAP_MARKSURFACES );
	q1_numsurfedges = Q1_CopyLump (Q1_LUMP_SURFEDGES, q1_dsurfedges, sizeof(q1_dsurfedges[0]), Q1_MAX_MAP_SURFEDGES );
	q1_numedges = Q1_CopyLump (Q1_LUMP_EDGES, q1_dedges, sizeof(q1_dedge_t), Q1_MAX_MAP_EDGES );

	q1_texdatasize = Q1_CopyLump (Q1_LUMP_TEXTURES, q1_dtexdata, 1, Q1_MAX_MAP_MIPTEX );
	q1_visdatasize = Q1_CopyLump (Q1_LUMP_VISIBILITY, q1_dvisdata, 1, Q1_MAX_MAP_VISIBILITY );
	q1_lightdatasize = Q1_CopyLump (Q1_LUMP_LIGHTING, q1_dlightdata, 1, Q1_MAX_MAP_LIGHTING );
	q1_entdatasize = Q1_CopyLump (Q1_LUMP_ENTITIES, q1_dentdata, 1, Q1_MAX_MAP_ENTSTRING );

	return true;
}


static void convert_q1bsp(model_t *out_model)
{
	mem_pool_t *pool;
	model_t model;
	int	i, j, k;
	const int	max_vertices = 1024;

	pool = mem_create_pool();

	memset(&model, 0, sizeof(model));

	model.num_frames = 1;
	model.frameinfo = (frameinfo_t*)mem_alloc(pool, sizeof(frameinfo_t));
	memset(model.frameinfo, 0, sizeof(frameinfo_t));
	model.frameinfo->num_frames = 1;
	model.frameinfo->frametime = 1.0f;

	model.frameinfo->frames = (singleframe_t*)mem_alloc(pool, sizeof(singleframe_t));
	memset(model.frameinfo->frames, 0, sizeof(singleframe_t));
	model.frameinfo->frames->offset = 0;
	model.frameinfo->frames->name = mem_copystring(pool, "some frame");

	model.num_meshes = q1_numtexinfo;
	model.meshes = (mesh_t*)mem_alloc(pool, model.num_meshes * sizeof(mesh_t));
	memset(model.meshes, 0, model.num_meshes * sizeof(mesh_t));

	for (i= 0; i < model.num_meshes; ++i)
	{
		model.meshes[i].name = mem_copystring(pool, "some mesh");
		model.meshes[i].vertex3f = mem_alloc(pool, max_vertices * sizeof(float) * 3);
		model.meshes[i].normal3f = mem_alloc(pool, max_vertices * sizeof(float) * 3);
		model.meshes[i].texcoord2f = mem_alloc(pool, max_vertices * sizeof(float) * 2);
		model.meshes[i].triangle3i = mem_alloc(pool, max_vertices * sizeof(int) * 3);
	}

	for (i = 0; i < q1_numfaces; ++i)
	{
		q1_dface_t* face;
		float normal_sign, normal[3], tex_scale[2];
		int edge_index, first_vertex;
		mesh_t* mesh;
		q1_texinfo_t* tex;
		q1_miptex_t* miptex;

		face = q1_dfaces + i;

		normal_sign= face->side == 0 ? 1.0f : -1.0f;
		for(j= 0; j < 3; ++j)
			normal[j]= q1_dplanes[face->planenum].normal[j] * normal_sign;

		tex= &q1_texinfo[face->texinfo];

		miptex= (q1_miptex_t*)(q1_dtexdata + ((q1_dmiptexlump_t*)q1_dtexdata)->dataofs[tex->miptex]);
		tex_scale[0]= 1.0f / miptex->width;
		tex_scale[1]= 1.0f / miptex->height;

		mesh = &model.meshes[0]; // TODO - use different meshes

		// Extract vertices.
		first_vertex = mesh->num_vertices;
		for(edge_index= 0; edge_index < face->numedges; ++edge_index)
		{
			int e, vert_num;
			q1_dvertex_t* in_vertex;

			e= q1_dsurfedges[face->firstedge + edge_index];
			in_vertex =
				e >= 0
					? &q1_dvertexes[ q1_dedges[+e].v[0] ]
					: &q1_dvertexes[ q1_dedges[-e].v[1] ];

			vert_num = first_vertex + edge_index;
			memcpy(mesh->vertex3f + vert_num * 3, in_vertex->point, sizeof(float) * 3);
			memcpy(mesh->normal3f + vert_num * 3, normal, sizeof(float) * 3);

			for(int k= 0; k < 2; ++k)
			{
				mesh->texcoord2f[vert_num * 2 + k]= tex_scale[k] * (
					tex->vecs[k][0] * in_vertex->point[0] +
					tex->vecs[k][1] * in_vertex->point[1] +
					tex->vecs[k][2] * in_vertex->point[2] +
					tex->vecs[k][3] );
			}
		}

		// Create triangles.
		int first_triangle = mesh->num_triangles;
		for(edge_index= 0; edge_index < face->numedges - 2; ++edge_index)
		{
			int triangle_num;
			triangle_num = first_triangle + edge_index;
			mesh->triangle3i[triangle_num * 3 + 0]= first_vertex;
			mesh->triangle3i[triangle_num * 3 + 1]= first_vertex + edge_index + 1;
			mesh->triangle3i[triangle_num * 3 + 2]= first_vertex + edge_index + 2;
		}

		mesh->num_vertices+= face->numedges;
		mesh->num_triangles+= face->numedges - 2;
	}

	*out_model= model;
}

bool_t model_q1bsp_load(void *filedata, size_t filesize, model_t *out_model, char **out_error)
{

	Q1_AllocMaxBSP();

	if (!load_q1bsp_moddel(filedata, filesize, out_error))
		return false;

	convert_q1bsp(out_model);

	return true;
}

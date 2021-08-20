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

bool_t model_q1bsp_load(void *filedata, size_t filesize, model_t *out_model, char **out_error)
{
	Q1_AllocMaxBSP();

	if (!load_q1bsp_moddel(filedata, filesize, out_error))
		return false;

	return true;
}

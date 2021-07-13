#include "BspcLibIncludes.hpp"

void AllocateQ3Map()
{
	q3_nummodels = 0;
	q3_dmodels = new q3_dmodel_t[MAX_MAP_MODELS];
	q3_numShaders = 0;
	q3_dshaders = new q3_dshader_t[Q3_MAX_MAP_SHADERS];
	q3_entdatasize = 0;
	q3_dentdata = new char[Q3_MAX_MAP_ENTSTRING];
	q3_numleafs = 0;
	q3_dleafs = new q3_dleaf_t[Q3_MAX_MAP_LEAFS];
	q3_numnodes = 0;
	q3_dnodes = new q3_dnode_t[Q3_MAX_MAP_NODES];
	q3_numleafsurfaces = 0;
	q3_dleafsurfaces = new int[Q3_MAX_MAP_LEAFFACES];
	q3_numleafbrushes = 0;
	q3_dleafbrushes = new int[Q3_MAX_MAP_LEAFBRUSHES];
	q3_numbrushes = 0;
	q3_dbrushes = new q3_dbrush_t[Q3_MAX_MAP_BRUSHES];
	q3_numbrushsides = 0;
	q3_dbrushsides = new q3_dbrushside_t[Q3_MAX_MAP_BRUSHSIDES];
	q3_numLightBytes = 0;
	q3_lightBytes = new byte[Q3_MAX_MAP_LIGHTING];
	q3_numGridPoints = 0;
	q3_gridData = new byte[Q3_MAX_MAP_LIGHTGRID];
	q3_numVisBytes = 0;
	q3_visBytes = new byte[Q3_MAX_MAP_VISIBILITY];
	q3_numDrawVerts = 0;
	q3_drawVerts = new q3_drawVert_t[Q3_MAX_MAP_DRAW_VERTS];
	q3_numDrawIndexes = 0;
	q3_drawIndexes = new int[Q3_MAX_MAP_DRAW_INDEXES];
	q3_numDrawSurfaces = 0;
	q3_drawSurfaces = new q3_dsurface_t[Q3_MAX_MAP_DRAW_SURFS];
	q3_numFogs = 0;
	q3_dfogs = new q3_dfog_t[Q3_MAX_MAP_FOGS];
}

void ConvertQ1MapIntoQ3Map()
{
}

int main()
{
	Q1_AllocMaxBSP();

	Q1_LoadBSPFile(const_cast<char*>("e1m1.bsp"), 0, 1365176);
	Q1_FreeMaxBSP();

	Q3_WriteBSPFile(const_cast<char*>("q3e1m1.bsp"));
}

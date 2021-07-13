#include "BspcLibIncludes.hpp"
#include  <cstring>

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

int ConvertLeaf(const q1_dleaf_t& in_leaf)
{
	q3_dleaf_t out_leaf{};
	std::memcpy(out_leaf.mins, in_leaf.mins, sizeof(float) * 3);
	std::memcpy(out_leaf.maxs, in_leaf.maxs, sizeof(float) * 3);
	// TODO - fill other fields.

	const int out_leaf_index = q3_numleafs;
	q3_dleafs[out_leaf_index]= out_leaf;
	++q3_numleafs;
	return out_leaf_index;
}

// Returns out node/leaf index.
int ConvertNode(const q1_dnode_t& in_node)
{
	q3_dnode_t out_node{};
	std::memcpy(out_node.mins, in_node.mins, sizeof(float) * 3);
	std::memcpy(out_node.maxs, in_node.maxs, sizeof(float) * 3);

	for(size_t child_index = 0; child_index < 2; ++child_index)
	{
		const short child = in_node.children[child_index];
		if(child < 0)
			out_node.children[child_index]= -1 - ConvertLeaf(q1_dleafs[-1 - child]);
		else
			out_node.children[child_index]= ConvertNode(q1_dnodes[child]);
	}

	const int out_node_index = q3_numnodes;
	q3_dnodes[out_node_index]= out_node;
	++q3_numnodes;
	return out_node_index;
}

void ConvertModel(const q1_dmodel_t& in_model)
{
	q3_dmodel_t out_model{};
	std::memcpy(out_model.mins, in_model.mins, sizeof(float) * 3);
	std::memcpy(out_model.maxs, in_model.maxs, sizeof(float) * 3);
	// TODO - fill other fields.

	ConvertNode(q1_dnodes[in_model.headnode[0]]);

	q3_dmodels[q3_nummodels]= out_model;
	++q3_nummodels;
}

void ConvertMap()
{
	for(int model_index = 0; model_index < q1_nummodels; ++model_index)
	{
		ConvertModel(q1_dmodels[model_index]);
	}
}

int main()
{
	Q1_AllocMaxBSP();

	Q1_LoadBSPFile(const_cast<char*>("e1m1.bsp"), 0, 1365176);
	AllocateQ3Map();
	ConvertMap();
	Q1_FreeMaxBSP();
	Q3_WriteBSPFile(const_cast<char*>("q3e1m1.bsp"));
}

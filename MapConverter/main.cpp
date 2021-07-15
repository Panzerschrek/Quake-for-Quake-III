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
	q3_numplanes = 0;
	q3_dplanes = new q3_dplane_t[Q3_MAX_MAP_PLANES];
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

void ConvertPlane(const q1_dplane_t& in_plane)
{
	q3_dplane_t out_plane{};
	std::memcpy(out_plane.normal, in_plane.normal, sizeof(float) * 3);
	out_plane.dist= in_plane.dist;

	q3_dplanes[q3_numplanes]= out_plane;
	++q3_numplanes;
}

void ConvertPlanes()
{
	// Make 1 to 1 planes conversion.
	for(int i= 0; i < q1_numplanes; ++i)
		ConvertPlane(q1_dplanes[i]);
}

void ConvertSurface(const q1_dface_t& in_surface)
{
	q3_dsurface_t out_surface{};
	out_surface.shaderNum = 0;

	out_surface.surfaceType = MST_PLANAR;

	const float normal_sign= in_surface.side == 0 ? 1.0f : -1.0f;
	float normal[3]{};

	for(int i= 0; i < 3; ++i)
		normal[i]= q1_dplanes[in_surface.planenum].normal[i] * normal_sign;

	const q1_texinfo_t& tex= q1_texinfo[in_surface.texinfo];
	// TODO - read scale to convert Q1 pixel coords to normalized (OpenGL) Q3 coords.
	const float tex_scale[2]{ 0.0625f, 0.0625f };

	// Extract vertices from edges and build polygon.
	out_surface.firstVert= q3_numDrawVerts;
	for(int edge_index= 0; edge_index < in_surface.numedges; ++edge_index)
	{
		const int e= q1_dsurfedges[in_surface.firstedge + edge_index];
		const q1_dvertex_t& in_vertex=
			e >= 0
				? q1_dvertexes[ q1_dedges[+e].v[0] ]
				: q1_dvertexes[ q1_dedges[-e].v[1] ];

		q3_drawVert_t out_vertex{};
		std::memcpy(out_vertex.xyz, in_vertex.point, sizeof(float) * 3);
		std::memcpy(out_vertex.normal, normal, sizeof(float) * 3);

		for(int i= 0; i < 2; ++i)
		{
			out_vertex.st[i]=
				tex.vecs[i][0] * in_vertex.point[0] +
				tex.vecs[i][1] * in_vertex.point[1] +
				tex.vecs[i][2] * in_vertex.point[2] +
				tex.vecs[i][3];
			out_vertex.st[i]*= tex_scale[i];
		}

		out_vertex.color[0]= 192;
		out_vertex.color[1]= 192;
		out_vertex.color[2]= 192;
		out_vertex.color[3]= 255;

		// TODO - fill other vertex fields.

		q3_drawVerts[q3_numDrawVerts]= out_vertex;
		++q3_numDrawVerts;
	}
	out_surface.numVerts= q3_numDrawVerts - out_surface.firstVert;

	// Generate simple indexes for polygon triangles.
	out_surface.firstIndex= q3_numDrawIndexes;
	const int triangle_count= out_surface.numVerts - 2;
	for(int i= 0; i < triangle_count; ++i)
	{
		q3_drawIndexes[ out_surface.firstIndex + i * 3 + 0 ]= 0;
		q3_drawIndexes[ out_surface.firstIndex + i * 3 + 1 ]= i + 1;
		q3_drawIndexes[ out_surface.firstIndex + i * 3 + 2 ]= i + 2;
	}
	q3_numDrawIndexes+= triangle_count * 3;
	out_surface.numIndexes= triangle_count * 3;

	// TODO - fill other fields.

	q3_drawSurfaces[q3_numDrawSurfaces]= out_surface;
	++q3_numDrawSurfaces;
}

void ConvertSurfaces()
{
	// Make 1 to 1 surfaces conversion.
	for(int i= 0; i < q1_numfaces; ++i)
		ConvertSurface(q1_dfaces[i]);
}

void ConvertLeaf(const q1_dleaf_t& in_leaf)
{
	q3_dleaf_t out_leaf{};
	for(int i= 0; i < 3; ++i)
	{
		out_leaf.mins[i]= in_leaf.mins[i];
		out_leaf.maxs[i]= in_leaf.maxs[i];
	}

	// Extract marksurfaces (indexes of surfaces related to this leaf).
	// It is fine to copy marksurfaces directly since we convert surfaces 1 to 1.
	out_leaf.firstLeafSurface= q3_numleafsurfaces;
	out_leaf.numLeafSurfaces= in_leaf.nummarksurfaces;
	for(int mark_surface_index= 0; mark_surface_index < in_leaf.nummarksurfaces; ++mark_surface_index)
	{
		const auto mark_surface = q1_dmarksurfaces[in_leaf.firstmarksurface + mark_surface_index];
		q3_dleafsurfaces[out_leaf.firstLeafSurface + mark_surface_index] = mark_surface;
	}
	q3_numleafsurfaces+= in_leaf.nummarksurfaces;

	// TODO - fill other fields.

	q3_dleafs[q3_numleafs]= out_leaf;
	++q3_numleafs;
}

void ConvertLeafs()
{
	for(int i= 0; i < q1_numleafs; ++i)
		ConvertLeaf(q1_dleafs[i]);
}

void ConvertNode(const q1_dnode_t& in_node)
{
	q3_dnode_t out_node{};
	for(int i= 0; i < 3; ++i)
	{
		out_node.mins[i]= in_node.mins[i];
		out_node.maxs[i]= in_node.maxs[i];
	}

	out_node.planeNum = in_node.planenum; // Planes converted 1 to 1

	for(size_t child_index = 0; child_index < 2; ++child_index)
	{
		const short child = in_node.children[child_index];
		out_node.children[child_index]= child; // Nodes and leafs converted 1 to 1
	}

	// TODO - fill other fields.

	q3_dnodes[q3_numnodes]= out_node;
	++q3_numnodes;
}

void ConvertNodes()
{
	for(int i= 0; i < q1_numnodes; ++i)
		ConvertNode(q1_dnodes[i]);
}

void ConvertModel(const q1_dmodel_t& in_model)
{
	q3_dmodel_t out_model{};
	std::memcpy(out_model.mins, in_model.mins, sizeof(float) * 3);
	std::memcpy(out_model.maxs, in_model.maxs, sizeof(float) * 3);

	out_model.firstSurface = in_model.firstface;
	out_model.numSurfaces = in_model.numfaces;

	// TODO - fill other fields.

	q3_dmodels[q3_nummodels]= out_model;
	++q3_nummodels;
}

void ConvertModels()
{
	for(int model_index = 0; model_index < q1_nummodels; ++model_index)
	{
		ConvertModel(q1_dmodels[model_index]);
	}
}

void PopulateShaders()
{
	q3_dshader_t out_shader{};
	std::strcpy(out_shader.shader, "base_wall/metaltech10final");
	q3_dshaders[q3_numShaders]= out_shader;
	++q3_numShaders;
}

void ConvertMap()
{
	ConvertPlanes();
	ConvertSurfaces();
	ConvertLeafs();
	ConvertNodes();
	ConvertModels();
	PopulateShaders();

	const char entities[]=
	R"(
		{
		"classname" "worldspawn"
		}

		{
		"origin" "0 0 0"
		"classname" "info_player_deathmatch"
		}
	)";

	std::strcpy(q3_dentdata, entities);
	q3_entdatasize = sizeof(entities);
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

#include "BspcLibIncludes.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>

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

// You must export textures before this call!
void ConvertSurface(const q1_dface_t& in_surface)
{
	q3_dsurface_t out_surface{};

	out_surface.surfaceType = MST_PLANAR;

	// Set lightmap params later.

	const float normal_sign= in_surface.side == 0 ? 1.0f : -1.0f;
	float normal[3]{};

	for(int i= 0; i < 3; ++i)
		normal[i]= q1_dplanes[in_surface.planenum].normal[i] * normal_sign;

	std::memcpy(out_surface.lightmapVecs[2], normal, sizeof(float) * 3);
	// TODO - set vec[0] and vec[1].

	const q1_texinfo_t& tex= q1_texinfo[in_surface.texinfo];

	const auto miptexlump= reinterpret_cast<const q1_dmiptexlump_t*>(q1_dtexdata);
	const auto miptex= reinterpret_cast<const q1_miptex_t*>(q1_dtexdata + miptexlump->dataofs[tex.miptex]);

	if(tex.miptex >= 0 && tex.miptex < q3_numShaders)
		out_surface.shaderNum = tex.miptex;
	else
		out_surface.shaderNum= 0;

	const float tex_scale[2]{ 1.0f / float(miptex->width), 1.0f / float(miptex->height) };

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

		// Set lightmap coords later.

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

int GetOrInserdPlane(const q3_dplane_t& plane)
{
	// TODO - search for exact plane.
	const int res= q3_numplanes;
	q3_dplanes[res]= plane;
	++q3_numplanes;
	return res;
}

void CreateSurfaceBrush(const q3_dsurface_t& surface)
{
	q3_dbrush_t out_brush{};
	out_brush.shaderNum= surface.shaderNum;

	out_brush.firstSide= q3_numbrushsides;

	if(false)
	{
		// Add surface plane.
		{
			q3_dplane_t surface_plane{};
			std::memcpy(surface_plane.normal, surface.lightmapVecs[2], sizeof(float) * 3);

			{
				const q3_drawVert_t& surface_vertex= q3_drawVerts[surface.firstVert];
				surface_plane.dist= DotProduct(surface_plane.normal, surface_vertex.xyz);
			}

			q3_dbrushside_t surface_side{};
			surface_side.planeNum= GetOrInserdPlane(surface_plane);
			surface_side.shaderNum= surface.shaderNum;

			q3_dbrushsides[q3_numbrushsides]= surface_side;
			++q3_numbrushsides;
		}

		// Calculate base vertex for side planes.
		float surface_center[3]{0.0f, 0.0f, 0.0f};
		for(int i= 0; i < surface.numVerts; ++i)
		{
			const q3_drawVert_t& v= q3_drawVerts[surface.firstVert + i];
			for(int j= 0; j < 3; ++j)
				surface_center[j]+= v.xyz[j];
		}
		for(int j= 0; j < 3; ++j)
			surface_center[j]/= float(surface.numVerts);

		float center_shifted[3];
		for(int j= 0; j < 3; ++j)
			center_shifted[j]= surface_center[j] - 0.1f * surface.lightmapVecs[2][j];

		// Create side planes
		for(int i= 0; i < surface.numVerts; ++i)
		{
			const q3_drawVert_t& v0= q3_drawVerts[surface.firstVert + i];
			const q3_drawVert_t& v1= q3_drawVerts[surface.firstVert + (i + 1) % surface.numVerts];

			float vecs[2][3];
			VectorSubtract(v0.xyz, center_shifted, vecs[0]);
			VectorSubtract(v1.xyz, center_shifted, vecs[1]);
			float normal[3];
			CrossProduct(vecs[0], vecs[1], normal);
			VectorNormalize(normal);
			if(DotProduct(normal, normal) <= 0)
				continue;

			q3_dplane_t edge_plane{};
			std::memcpy(edge_plane.normal, normal, sizeof(float) * 3);
			edge_plane.dist= DotProduct(center_shifted, normal);

			q3_dbrushside_t edge_side{};
			edge_side.planeNum= GetOrInserdPlane(edge_plane);
			edge_side.shaderNum= surface.shaderNum;

			q3_dbrushsides[q3_numbrushsides]= edge_side;
			++q3_numbrushsides;
		}
	}

	if(true)
	{
		float mins[3]{ +999999, +999999, +999999 };
		float maxs[3]{ -999999, -999999, -999999 };
		for(int i= 0; i < surface.numVerts; ++i)
		{
			const q3_drawVert_t& v= q3_drawVerts[surface.firstVert + i];
			for(int j= 0; j < 3; ++j)
			{
				if(v.xyz[j] < mins[j]) mins[j]= v.xyz[j];
				if(v.xyz[j] > maxs[j]) maxs[j]= v.xyz[j];
			}
		}

		for(int j= 0; j < 3; ++j)
		{
			mins[j]-= 1.0f;
			maxs[j]+= 1.0f;
		}

		for(int j= 0; j < 3; ++j)
		for(int k= 0; k < 2; ++k)
		{
			q3_dplane_t box_plane{};
			box_plane.normal[j]= k == 0 ? (-1.0f) : (+1.0f);
			box_plane.dist= -(k == 0 ? mins[j] : maxs[j]) * box_plane.normal[j];

			q3_dbrushside_t box_side{};
			box_side.planeNum= GetOrInserdPlane(box_plane);
			box_side.shaderNum= surface.shaderNum;

			q3_dbrushsides[q3_numbrushsides]= box_side;
			++q3_numbrushsides;
		}
	}

	out_brush.numSides= q3_numbrushsides - out_brush.firstSide;

	q3_dbrushes[q3_numbrushes]= out_brush;
	++q3_numbrushes;
}

void CreateLeafBrushes(q3_dleaf_t& leaf)
{
	// We have 1 to 1 brush to surface.
	leaf.firstLeafBrush= q3_numleafbrushes;
	for(int surface_index= leaf.firstLeafSurface; surface_index < leaf.firstLeafSurface + leaf.numLeafSurfaces; ++surface_index)
	{
		q3_dleafbrushes[q3_numleafbrushes]= surface_index;
		++q3_numleafbrushes;
	}
	leaf.numLeafBrushes= q3_numleafbrushes - leaf.firstLeafBrush;
}

void CreateBrushes()
{
	// Create own brush for each surface.
	for(int i= 0; i < q3_numDrawSurfaces; ++i)
		CreateSurfaceBrush(q3_drawSurfaces[i]);

	for(int i= 0; i < q3_numleafs; ++i)
		CreateLeafBrushes(q3_dleafs[i]);
}

struct SurfaceLightmapInfo
{
	int surface_index;
	int lightmap_size[2];
	int lightmap_mins[2];
};

// Sort lightmaps by height, than by width.
bool SurfaceLightmapInfoOdreding(const SurfaceLightmapInfo& l, const SurfaceLightmapInfo& r)
{
	if(l.lightmap_size[1] != r.lightmap_size[1])
		return l.lightmap_size[1] > r.lightmap_size[1];
	return l.lightmap_size[0] > r.lightmap_size[0];
}

// Surfaces should be converted before this call.
void ConvertLightmaps()
{
	std::vector<SurfaceLightmapInfo> surfaces_lightmap_info;
	surfaces_lightmap_info.reserve(size_t(q3_numDrawSurfaces));

	for(int i= 0; i < q3_numDrawSurfaces; ++i)
	{
		const q1_dface_t& q1_surface= q1_dfaces[i];
		if(q1_surface.lightofs < 0)
			continue;

		const q3_dsurface_t& surface= q3_drawSurfaces[i];
		const q1_texinfo_t& tex= q1_texinfo[q1_surface.texinfo]; // Should be correct texinfo for 1 to 1 surfaces export.

		SurfaceLightmapInfo out_info{};
		out_info.surface_index= i;

		// Calculate extents.
		// See also "CalcSurfaceExtents" from Quake1.

		float mins[2]{ +999999, +999999 };
		float maxs[2]{ -999999, -999999 };

		for(int v= surface.firstVert; v < surface.firstVert + surface.numVerts; ++v)
		{
			const q3_drawVert_t& vert= q3_drawVerts[v];
			for (int j=0; j < 2; ++j)
			{
				const float val =
					tex.vecs[j][0] * vert.xyz[0] +
					tex.vecs[j][1] * vert.xyz[1] +
					tex.vecs[j][2] * vert.xyz[2] +
					tex.vecs[j][3];
				if (val < mins[j])
					mins[j] = val;
				if (val > maxs[j])
					maxs[j] = val;
			}
		}

		for (int j=0; j < 2; ++j)
		{
			const int min= int(std::floor(mins[j] / 16.0f));
			const int max= int(std::ceil(maxs[j] / 16.0f));

			out_info.lightmap_mins[j]= min;
			out_info.lightmap_size[j] = 1 + max - min;
		}

		surfaces_lightmap_info.push_back(out_info);
	}

	std::sort(surfaces_lightmap_info.begin(), surfaces_lightmap_info.end(), SurfaceLightmapInfoOdreding);

	// Fill atlases line by line.
	int current_x= 0;
	int current_y= 0;
	int current_z= 0;
	int current_height= surfaces_lightmap_info.empty() ? 0 : surfaces_lightmap_info.front().lightmap_size[1];
	const int lightmap_atlas_size= 128; // Q3 atlas size
	const int lightmap_components= 3;
	for(const SurfaceLightmapInfo& info : surfaces_lightmap_info)
	{
		// Place lightmap in atlas.
		if(current_x + info.lightmap_size[0] > lightmap_atlas_size)
		{
			current_y+= current_height;
			current_x= 0;
			current_height= info.lightmap_size[1];
			if(current_y + current_height > lightmap_atlas_size)
			{
				current_y= 0;
				++current_z;
			}
		}
		assert(info.lightmap_size[1] <= current_height);

		const int surface_lightmap_pos[2]{ current_x, current_y };
		current_x+= info.lightmap_size[0];

		// Set surface parameters.

		q3_dsurface_t& surface= q3_drawSurfaces[info.surface_index];
		surface.lightmapNum= current_z;
		surface.lightmapWidth = info.lightmap_size[0];
		surface.lightmapHeight= info.lightmap_size[1];
		surface.lightmapX= surface_lightmap_pos[0];
		surface.lightmapY= surface_lightmap_pos[1];

		// Set lightmap coordinates for vertices.
		q1_dface_t& q1_surface= q1_dfaces[info.surface_index];
		const q1_texinfo_t& tex= q1_texinfo[q1_surface.texinfo]; // Should be correct texinfo for 1 to 1 surfaces export.
		for(int v= surface.firstVert; v < surface.firstVert + surface.numVerts; ++v)
		{
			q3_drawVert_t& vert= q3_drawVerts[v];

			for(int j= 0; j < 2; ++j)
			{
				const float tex_coord =
					tex.vecs[j][0] * vert.xyz[0] +
					tex.vecs[j][1] * vert.xyz[1] +
					tex.vecs[j][2] * vert.xyz[2] +
					tex.vecs[j][3];
				const float lightmap_coord= tex_coord / 16.0f - float(info.lightmap_mins[j]);
				vert.lightmap[j]= (float(surface_lightmap_pos[j]) + lightmap_coord + 0.5f) / float(lightmap_atlas_size);
			}
		}

		int style_count= 0;
		for(int i= 0; i < MAXLIGHTMAPS && q1_surface.styles[i] != 255; ++i)
			++style_count;
		const int style_step= info.lightmap_size[0] * info.lightmap_size[1];

		// Copy lighting data.
		const byte* const src_lightmap_data= q1_dlightdata + q1_surface.lightofs;
		byte* const dst_lightmap_data=
			q3_lightBytes + (surface_lightmap_pos[0] + surface_lightmap_pos[1] * lightmap_atlas_size + current_z * lightmap_atlas_size * lightmap_atlas_size) * lightmap_components;
		for(int y= 0; y < info.lightmap_size[1]; ++y)
		for(int x= 0; x < info.lightmap_size[0]; ++x)
		{
			const unsigned int light_scale= 192u; // Q3 clamps lightmap values a bit. So, reduce range.

			// Q3 does nto support lightstyles. So, just combone all lightmaps of this surface into one.
			const byte* const src_ptr= src_lightmap_data + x + y * info.lightmap_size[0];
			int src= 0;
			for(int i= 0; i < style_count; ++i)
				src+= src_ptr[i * style_step] * light_scale / 256u;
			if(src > 255)
				src= 255;

			byte* const dst= dst_lightmap_data + (x + y * lightmap_atlas_size) * lightmap_components;
			for(int j= 0; j < lightmap_components; ++j)
				dst[j]= byte(src);
		}
	}
	q3_numLightBytes= (current_z + 1) * lightmap_atlas_size * lightmap_atlas_size * lightmap_components;
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

	//out_model.firstBrush= 0;
	//out_model.numBrushes= q3_numbrushes;
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
	// Export textures 1 to 1.
	const auto miptexlump= reinterpret_cast<const q1_dmiptexlump_t*>(q1_dtexdata);

	for(int i= 0; i < miptexlump->nummiptex; ++i)
	{
		const auto miptex= reinterpret_cast<const q1_miptex_t*>(q1_dtexdata + miptexlump->dataofs[i]);

		q3_dshader_t out_shader{};
		std::snprintf(out_shader.shader, sizeof(out_shader.shader), "textures/q3e1m1/%s.tga", miptex->name);
		out_shader.contentFlags= CONTENTS_SOLID;
		q3_dshaders[q3_numShaders]= out_shader;
		++q3_numShaders;
	}
}

void ConvertEntities()
{
	std::memcpy(q3_dentdata, q1_dentdata, size_t(q1_entdatasize));
	q3_entdatasize= q1_entdatasize;
	//std::cout << q3_dentdata << std::endl;
}

using Palette= std::array<byte, 256*3>;

Palette LoadPalette()
{
	void* buffer= nullptr;
	LoadFile(const_cast<char*>("palette.lmp"), &buffer, 0, sizeof(Palette));
	Palette res;
	std::memcpy(&res, buffer, sizeof(Palette));
	FreeMemory(buffer);
	return res;
}

void ExtractTextures()
{
	const Palette palette= LoadPalette();

	std::vector<byte> tmp_tex_data;

	const auto miptexlump= reinterpret_cast<const q1_dmiptexlump_t*>(q1_dtexdata);

	for(int i= 0; i < miptexlump->nummiptex; ++i)
	{
		if(miptexlump->dataofs[i] < 0)
			continue;

		const auto miptex= reinterpret_cast<const q1_miptex_t*>(q1_dtexdata + miptexlump->dataofs[i]);

		tmp_tex_data.resize(miptex->width * miptex->height * 4);

		const byte* const src_tex_data= reinterpret_cast<const byte*>(miptex) + miptex->offsets[0];
		// Flip image to convert to OpenGL coordinate system (used in Q3).
		for(unsigned int y= 0; y < miptex->height; ++y)
		for(unsigned int x= 0; x < miptex->width ; ++x)
		{
			const unsigned int dst= x + y * miptex->width;
			const unsigned int src= x + (miptex->height - 1 - y) * miptex->width;
			const byte color_index= src_tex_data[src];
			tmp_tex_data[ dst * 4     ]= palette[ color_index * 3    ];
			tmp_tex_data[ dst * 4 + 1 ]= palette[ color_index * 3 + 1 ];
			tmp_tex_data[ dst * 4 + 2 ]= palette[ color_index * 3 + 2 ];
			tmp_tex_data[ dst * 4 + 3 ]= 255;
		}

		const std::string out_file_name = std::string("textures_extracted/") + miptex->name + ".tga";
		WriteTGA(out_file_name.c_str(), tmp_tex_data.data(), int(miptex->width), int(miptex->height));
	}
}

void ConvertMap()
{
	ExtractTextures();
	PopulateShaders();

	ConvertPlanes();
	ConvertSurfaces();
	ConvertLightmaps();
	ConvertLeafs();
	ConvertNodes();
	CreateBrushes();
	ConvertModels();
	ConvertEntities();
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

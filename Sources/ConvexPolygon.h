#pragma once

#include "Common.h"
#include "Vertex.h"

namespace MeshNinja
{
	class MESH_NINJA_API ConvexPolygon
	{
	public:
		ConvexPolygon();
		virtual ~ConvexPolygon();

		std::vector<Vertex>* vertexArray;
	};
}
#pragma once

#include "Common.h"
#include "Vector.h"

namespace MeshNinja
{
	class MESH_NINJA_API Vertex
	{
	public:
		Vertex();
		virtual ~Vertex();

		Vector point;
		Vector normal;
		Vector color;
		Vector texCoords;
	};
}
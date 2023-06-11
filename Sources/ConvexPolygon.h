#pragma once

#include "Common.h"
#include "Vector.h"

namespace MeshNinja
{
	// We assume here that the list of points are coplanar and form a
	// convex polygon.  If this is not the case, then the result of any
	// algorithm is left undefined.
	class MESH_NINJA_API ConvexPolygon
	{
	public:
		ConvexPolygon();
		virtual ~ConvexPolygon();

		std::vector<Vector>* vertexArray;
	};
}
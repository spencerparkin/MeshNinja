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
		ConvexPolygon(const ConvexPolygon& polygon);
		virtual ~ConvexPolygon();

		void Clear();
		void Compress(double eps = MESH_NINJA_EPS);

		std::vector<Vector>* vertexArray;
	};
}
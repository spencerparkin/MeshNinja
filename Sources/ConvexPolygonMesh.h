#pragma once

#include "Common.h"
#include "Vector.h"
#include "ConvexPolygon.h"

namespace MeshNinja
{
	// Each face is assumed to be a convex polygon.  If this is not the case, then
	// we simply leave the results of all algorithms as undefined.
	class MESH_NINJA_API ConvexPolygonMesh
	{
	public:
		ConvexPolygonMesh();
		virtual ~ConvexPolygonMesh();

		void ToConvexPolygonArray(std::vector<ConvexPolygon>& convexPolygonArray) const;
		void FromConvexPolygonArray(const std::vector<ConvexPolygon>& convexPolygonArray);

		class Facet
		{
		public:
			Facet();
			virtual ~Facet();

			std::vector<int> vertexArray;
		};

		std::vector<Facet>* facetArray;
		std::vector<Vector>* vertexArray;
	};
}
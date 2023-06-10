#pragma once

#include "Common.h"
#include "Vertex.h"
#include "ConvexPolygon.h"

namespace MeshNinja
{
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
		std::vector<Vertex>* vertexArray;
	};
}
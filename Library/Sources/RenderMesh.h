#pragma once

#include "Common.h"
#include "Vector.h"
#include "Transform.h"

namespace MeshNinja
{
	class ConvexPolygonMesh;

	// When doing most operations on meshes, we don't want to carry around any of
	// the extra baggage normally associated with a renderable mesh.  Operations that
	// involve the calculation of UVs or normal, etc., can be done here, usually once
	// the final shape of the mesh is decided upon.
	class MESH_NINJA_API RenderMesh
	{
	public:
		RenderMesh();
		virtual ~RenderMesh();

		void Clear();
		void FromConvexPolygonMesh(const ConvexPolygonMesh& mesh);
		void ToConvexPolygonMesh(ConvexPolygonMesh& mesh) const;
		void ApplyTransform(const Transform& transform);
		bool IsTriangleMesh() const;

		struct Facet
		{
			Facet();
			Facet(const Facet& facet);
			virtual ~Facet();

			int operator[](int i) const
			{
				return (*this->vertexArray)[i];
			}

			std::vector<int>* vertexArray;
			Vector color;
			Vector normal;
			Vector center;
		};

		struct Vertex
		{
			Vertex();
			Vertex(const Vertex& vertex);
			virtual ~Vertex();

			Vector position;
			Vector color;
			Vector normal;
			Vector texCoords;
		};

		std::vector<Facet>* facetArray;
		std::vector<Vertex>* vertexArray;
	};
}
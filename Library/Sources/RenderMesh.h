#pragma once

#include "Common.h"
#include "Vector.h"
#include "Transform.h"

namespace MeshNinja
{
	class ConvexPolygonMesh;
	class AxisAlignedBoundingBox;

	// When doing most operations on meshes, we don't want to carry around any of
	// the extra baggage normally associated with a renderable mesh.  Operations that
	// involve the calculation of UVs or normal, etc., can be done here, usually once
	// the final shape of the mesh is decided upon.
	class MESH_NINJA_API RenderMesh
	{
	public:
		RenderMesh();
		RenderMesh(const RenderMesh& renderMesh);
		virtual ~RenderMesh();

		struct Options
		{
			enum class NormalType
			{
				VERTEX_BASED,
				FACET_BASED
			};

			Options()
			{
				this->normalType = NormalType::FACET_BASED;
				this->color = Vector(1.0, 0.0, 0.0);
			}

			NormalType normalType;
			Vector color;
		};

		void Clear();
		void Copy(const RenderMesh& renderMesh);
		void FromConvexPolygonMesh(const ConvexPolygonMesh& mesh, const Options& options);
		void ToConvexPolygonMesh(ConvexPolygonMesh& mesh) const;
		void ApplyTransform(const Transform& transform);
		bool IsTriangleMesh() const;
		void SetColor(const Vector& color);
		void MakeRainbowColors();
		bool CalcBoundingBox(AxisAlignedBoundingBox& box) const;

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
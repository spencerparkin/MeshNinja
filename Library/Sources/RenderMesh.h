#pragma once

#include "ConvexPolygonMesh.h"

namespace MeshNinja
{
	// When doing operations on meshes, we don't want to carry around any of
	// the extra baggage normally associated with a renderable mesh.  But here
	// provide a means of carrying and calculating that extra baggage.
	class MESH_NINJA_API RenderMesh : public ConvexPolygonMesh
	{
	public:
		RenderMesh();
		virtual ~RenderMesh();

		virtual void ApplyTransform(const Transform& transform) override;

		void RegenerateNormals();

		// TODO: Add methods for calculating colors and tex-coords using polar-coordinates.

		void FixArraySizes();

		struct ExtraFaceData
		{
			Vector color;
			Vector normal;
			Vector center;
		};

		struct ExtraVertexData
		{
			Vector color;
			Vector normal;
			Vector texCoords;
		};

		std::vector<ExtraFaceData>* extraFaceDataArray;
		std::vector<ExtraVertexData>* extraVertexDataArray;
	};
}
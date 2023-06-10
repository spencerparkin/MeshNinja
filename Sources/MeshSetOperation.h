#pragma once

#include "Common.h"

namespace MeshNinja
{
	class ConvexPolygonMesh;

	class MESH_NINJA_API MeshSetOperation
	{
	public:
		MeshSetOperation();
		virtual ~MeshSetOperation();

		virtual bool Perform(const ConvexPolygonMesh& meshA, const ConvexPolygonMesh& meshB, ConvexPolygonMesh& resultingMesh) = 0;
	};

	class MESH_NINJA_API MeshUnion : public MeshSetOperation
	{
	public:
		MeshUnion();
		virtual ~MeshUnion();

		virtual bool Perform(const ConvexPolygonMesh& meshA, const ConvexPolygonMesh& meshB, ConvexPolygonMesh& resultingMesh) override;
	};

	class MESH_NINJA_API MeshIntersection : public MeshSetOperation
	{
	public:
		MeshIntersection();
		virtual ~MeshIntersection();

		virtual bool Perform(const ConvexPolygonMesh& meshA, const ConvexPolygonMesh& meshB, ConvexPolygonMesh& resultingMesh) override;
	};

	class MESH_NINJA_API MeshSubtraction : public MeshSetOperation
	{
	public:
		MeshSubtraction();
		virtual ~MeshSubtraction();

		virtual bool Perform(const ConvexPolygonMesh& meshA, const ConvexPolygonMesh& meshB, ConvexPolygonMesh& resultingMesh) override;
	};
}
#pragma once

#include "Common.h"

namespace MeshNinja
{
	class ConvexPolygonMesh;

	class MESH_NINJA_API MeshBinaryOperation
	{
	public:
		MeshBinaryOperation();
		virtual ~MeshBinaryOperation();

		virtual bool Perform(const ConvexPolygonMesh& meshA, const ConvexPolygonMesh& meshB, ConvexPolygonMesh& resultingMesh) = 0;
	};

	class MESH_NINJA_API MeshMergeOperation : public MeshBinaryOperation
	{
	public:
		MeshMergeOperation();
		virtual ~MeshMergeOperation();

		virtual bool Perform(const ConvexPolygonMesh& meshA, const ConvexPolygonMesh& meshB, ConvexPolygonMesh& resultingMesh) override;
	};
}
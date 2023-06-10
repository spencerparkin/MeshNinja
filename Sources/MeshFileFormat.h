#pragma once

#include "Common.h"

namespace MeshNinja
{
	class ConvexPolygonMesh;

	class MESH_NINJA_API MeshFileFormat
	{
	public:
		MeshFileFormat();
		virtual ~MeshFileFormat();

		virtual bool LoadMeshes(const std::string& filePath, std::vector<ConvexPolygonMesh*>& convexPolygonMeshArray) = 0;
		virtual bool SaveMeshes(const std::string& filePath, const std::vector<ConvexPolygonMesh*>& convexPolygonMeshArray) = 0;
	};

	class MESH_NINJA_API ObjFileFormat : public MeshFileFormat
	{
	public:
		ObjFileFormat();
		virtual ~ObjFileFormat();

		virtual bool LoadMeshes(const std::string& filePath, std::vector<ConvexPolygonMesh*>& convexPolygonMeshArray) override;
		virtual bool SaveMeshes(const std::string& filePath, const std::vector<ConvexPolygonMesh*>& convexPolygonMeshArray) override;
	};
}
#pragma once

#include "Common.h"

namespace MeshNinja
{
	class ConvexPolygonMesh;
	class RenderMesh;

	// For simplicity, we just associate one mesh per file, and we don't try to take
	// advantage of all features of each file format.  For files containing multiple
	// meshes, the given mesh number specifies which to load.
	class MESH_NINJA_API MeshFileFormat
	{
	public:
		MeshFileFormat();
		virtual ~MeshFileFormat();

		virtual std::string GetExtension() const = 0;

		virtual bool LoadMesh(const std::string& filePath, ConvexPolygonMesh& mesh, int meshNumber = 0) = 0;
		virtual bool SaveMesh(const std::string& filePath, const ConvexPolygonMesh& mesh) = 0;

		virtual bool LoadRenderMesh(const std::string& filePath, RenderMesh& mesh, int meshNumber = 0) = 0;
		virtual bool SaveRenderMesh(const std::string& filePath, const RenderMesh& mesh) = 0;
	};
}
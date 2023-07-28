#pragma once

#include "../MeshFileFormat.h"

namespace MeshNinja
{
	// See: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html
	// Render meshes saved out using this class should render out-of-the-box with THREE.js.
	class MESH_NINJA_API glTF_FileFormat : public MeshFileFormat
	{
	public:
		glTF_FileFormat();
		virtual ~glTF_FileFormat();

		virtual std::string GetExtension() const override;

		virtual bool LoadMesh(const std::string& filePath, ConvexPolygonMesh& mesh, int meshNumber = 0) override;
		virtual bool SaveMesh(const std::string& filePath, const ConvexPolygonMesh& mesh) override;

		virtual bool LoadRenderMesh(const std::string& filePath, RenderMesh& mesh, int meshNumber = 0) override;
		virtual bool SaveRenderMesh(const std::string& filePath, const RenderMesh& mesh) override;
	};
}
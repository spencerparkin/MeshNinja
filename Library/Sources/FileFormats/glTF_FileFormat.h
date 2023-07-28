#pragma once

#include "../MeshFileFormat.h"

#define MESH_NINJA_GLTF_ARRAY_BUFFER				34962
#define MESH_NINJA_GLTF_ELEMENT_ARRAY_BUFFER		34963
#define MESH_NINJA_GLTF_ACCESSOR_DT_SIGNED_BYTE		5120
#define MESH_NINJA_GLTF_ACCESSOR_DT_UNSIGNED_BYTE	5121
#define MESH_NINJA_GLTF_ACCESSOR_DT_SIGNED_SHORT	5122
#define MESH_NINJA_GLTF_ACCESSOR_DT_UNSIGNED_SHORT	5123
#define MESH_NINJA_GLTF_ACCESSOR_DT_UNSIGNED_INT	5125
#define MESH_NINJA_GLTF_ACCESSOR_DT_FLOAT			5126
#define MESH_NINJA_GLTF_TOPO_MODE_POINTS			0
#define MESH_NINJA_GLTF_TOPO_MODE_LINES				1
#define MESH_NINJA_GLTF_TOPO_MODE_LINE_LOOP			2
#define MESH_NINJA_GLTF_TOPO_MODE_LINE_STRIP		3
#define MESH_NINJA_GLTF_TOPO_MODE_TRIANGLES			4
#define MESH_NINJA_GLTF_TOPO_MODE_TRIANGLE_STRIP	5
#define MESH_NINJA_GLTF_TOPO_MODE_TRIANGLE_FAN		6

namespace MeshNinja
{
	class JsonObject;
	class JsonArray;

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

	private:

		bool WriteIndexBuffer(const RenderMesh& mesh, std::ofstream& binFileStream, JsonArray* jsonBufferViewsArray, JsonArray* jsonAccessorsArray, JsonObject* jsonPrim);
		bool WriteVertexBuffer(const RenderMesh& mesh, std::ofstream& binFileStream, JsonArray* jsonBufferViewsArray, JsonArray* jsonAccessorsArray, JsonObject* jsonPrim);
	};
}
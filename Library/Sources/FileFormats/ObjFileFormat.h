#pragma once

#include "../MeshFileFormat.h"

namespace MeshNinja
{
	class MESH_NINJA_API ObjFileFormat : public MeshFileFormat
	{
	public:
		ObjFileFormat();
		virtual ~ObjFileFormat();

		virtual std::string GetExtension() const override;

		virtual bool LoadMesh(const std::string& filePath, ConvexPolygonMesh& mesh, int meshNumber = 0) override;
		virtual bool SaveMesh(const std::string& filePath, const ConvexPolygonMesh& mesh) override;

		virtual bool LoadRenderMesh(const std::string& filePath, RenderMesh& mesh, int meshNumber = 0) override;
		virtual bool SaveRenderMesh(const std::string& filePath, const RenderMesh& mesh) override;

	protected:

		void TokenizeLine(const std::string& line, char delimeter, std::vector<std::string>& tokenArray, bool stripEmptyTokens);
		void ProcessLine(const std::vector<std::string>& tokenArray, ConvexPolygonMesh& mesh);
	};
}
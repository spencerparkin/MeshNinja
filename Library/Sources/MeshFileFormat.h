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

		virtual bool LoadMesh(const std::string& filePath, ConvexPolygonMesh& mesh) = 0;
		virtual bool SaveMesh(const std::string& filePath, const ConvexPolygonMesh& mesh) = 0;
	};

	class MESH_NINJA_API ObjFileFormat : public MeshFileFormat
	{
	public:
		ObjFileFormat();
		virtual ~ObjFileFormat();

		virtual bool LoadMesh(const std::string& filePath, ConvexPolygonMesh& mesh) override;
		virtual bool SaveMesh(const std::string& filePath, const ConvexPolygonMesh& mesh) override;

	protected:

		void TokenizeLine(const std::string& line, char delimeter, std::vector<std::string>& tokenArray, bool stripEmptyTokens);
		void ProcessLine(const std::vector<std::string>& tokenArray, ConvexPolygonMesh& mesh);
	};
}
#include "ObjFileFormat.h"
#include "../ConvexPolygonMesh.h"

using namespace MeshNinja;

ObjFileFormat::ObjFileFormat()
{
}

/*virtual*/ ObjFileFormat::~ObjFileFormat()
{
}

/*virtual*/ std::string ObjFileFormat::GetExtension() const
{
	return ".obj";
}

/*virtual*/ bool ObjFileFormat::LoadRenderMesh(const std::string& filePath, RenderMesh& mesh, int meshNumber /*= 0*/)
{
	return false;
}

/*virtual*/ bool ObjFileFormat::SaveRenderMesh(const std::string& filePath, const RenderMesh& mesh)
{
	return false;
}

void ObjFileFormat::TokenizeLine(const std::string& line, char delimeter, std::vector<std::string>& tokenArray, bool stripEmptyTokens)
{
	std::stringstream stringStream(line);
	std::string token;
	while (std::getline(stringStream, token, delimeter))
		if (!stripEmptyTokens || token.size() > 0)
			tokenArray.push_back(token);
}

/*virtual*/ bool ObjFileFormat::LoadMesh(const std::string& filePath, ConvexPolygonMesh& mesh, int meshNumber /*= 0*/)
{
	// TODO: Obey the given mesh number.

	mesh.Clear();

	std::ifstream fileStream(filePath, std::ios::in);
	if (!fileStream.is_open())
		return false;

	std::string line;
	while (std::getline(fileStream, line))
	{
		std::vector<std::string> tokenArray;
		this->TokenizeLine(line, ' ', tokenArray, true);
		this->ProcessLine(tokenArray, mesh);
	}

	fileStream.close();
	return true;
}

void ObjFileFormat::ProcessLine(const std::vector<std::string>& tokenArray, ConvexPolygonMesh& mesh)
{
	if (tokenArray.size() == 0 || tokenArray[0] == "#")
		return;

	if (tokenArray[0] == "v")
	{
		Vector vertex;

		vertex.x = (tokenArray.size() > 1) ? ::atof(tokenArray[1].c_str()) : 0.0;
		vertex.y = (tokenArray.size() > 2) ? ::atof(tokenArray[2].c_str()) : 0.0;
		vertex.z = (tokenArray.size() > 3) ? ::atof(tokenArray[3].c_str()) : 0.0;

		mesh.vertexArray->push_back(vertex);
	}
	else if (tokenArray[0] == "f" && tokenArray.size() > 1)
	{
		ConvexPolygonMesh::Facet facet;

		for (const std::string& token : tokenArray)
		{
			if (token == "f")
				continue;

			std::vector<std::string> vertexTokenArray;
			this->TokenizeLine(token, '/', vertexTokenArray, false);

			int i = (vertexTokenArray.size() > 0 && vertexTokenArray[0].size() > 0) ? ::atoi(vertexTokenArray[0].c_str()) : INT_MAX;

			if (i > 0)
				i--;
			else if (i < 0)
				i = int(mesh.vertexArray->size()) - 1;

			if (i < 0)
				i = 0;
			else if (i >= (signed)mesh.vertexArray->size())
				i = (int)mesh.vertexArray->size() - 1;

			facet.vertexArray->push_back(i);
		}

		mesh.facetArray->push_back(facet);
	}
}

/*virtual*/ bool ObjFileFormat::SaveMesh(const std::string& filePath, const ConvexPolygonMesh& mesh)
{
	std::ofstream fileStream(filePath);
	if (!fileStream.is_open())
		return false;

	fileStream << "# Generated by MeshNinja.\n";
	fileStream << "#\n";
	fileStream << "# Vertices: " << mesh.vertexArray->size() << "\n";
	fileStream << "# Faces: " << mesh.facetArray->size() << "\n\n";

	for (const Vector& vertex : *mesh.vertexArray)
		fileStream << "v " << vertex.x << " " << vertex.y << " " << vertex.z << "\n";

	fileStream << "\n";

	for (const ConvexPolygonMesh::Facet& facet : *mesh.facetArray)
	{
		fileStream << "f ";

		for (int i = 0; i < (signed)facet.vertexArray->size(); i++)
		{
			int j = (*facet.vertexArray)[i] + 1;
			fileStream << j;
			if (i < (signed)facet.vertexArray->size() - 1)
				fileStream << " ";
			else
				fileStream << "\n";
		}
	}

	return true;
}
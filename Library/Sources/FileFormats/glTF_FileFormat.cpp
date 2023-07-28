#include "glTF_FileFormat.h"

using namespace MeshNinja;

glTF_FileFormat::glTF_FileFormat()
{
}

/*virtual*/ glTF_FileFormat::~glTF_FileFormat()
{
}

/*virtual*/ std::string glTF_FileFormat::GetExtension() const
{
	return ".glTF";
}

/*virtual*/ bool glTF_FileFormat::LoadMesh(const std::string& filePath, ConvexPolygonMesh& mesh, int meshNumber /*= 0*/)
{
	return false;
}

/*virtual*/ bool glTF_FileFormat::SaveMesh(const std::string& filePath, const ConvexPolygonMesh& mesh)
{
	return false;
}

/*virtual*/ bool glTF_FileFormat::LoadRenderMesh(const std::string& filePath, RenderMesh& mesh, int meshNumber /*= 0*/)
{
	return false;
}

/*virtual*/ bool glTF_FileFormat::SaveRenderMesh(const std::string& filePath, const RenderMesh& mesh)
{
	return false;
}
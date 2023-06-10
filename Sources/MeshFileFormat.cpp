#include "MeshFileFormat.h"

using namespace MeshNinja;

//----------------------------------- MeshFileFormat -----------------------------------

MeshFileFormat::MeshFileFormat()
{
}

/*virtual*/ MeshFileFormat::~MeshFileFormat()
{
}

//----------------------------------- ObjFileFormat -----------------------------------

ObjFileFormat::ObjFileFormat()
{
}

/*virtual*/ ObjFileFormat::~ObjFileFormat()
{
}

/*virtual*/ bool ObjFileFormat::LoadMeshes(const std::string& filePath, std::vector<ConvexPolygonMesh*>& convexPolygonMeshArray)
{
	return true;
}

/*virtual*/ bool ObjFileFormat::SaveMeshes(const std::string& filePath, const std::vector<ConvexPolygonMesh*>& convexPolygonMeshArray)
{
	return true;
}
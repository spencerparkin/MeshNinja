#include "MeshBinaryOperation.h"

using namespace MeshNinja;

//----------------------------------- MeshBinaryOperation -----------------------------------

MeshBinaryOperation::MeshBinaryOperation()
{
}

/*virtual*/ MeshBinaryOperation::~MeshBinaryOperation()
{
}

//----------------------------------- MeshMergeOperation -----------------------------------

MeshMergeOperation::MeshMergeOperation()
{
}

/*virtual*/ MeshMergeOperation::~MeshMergeOperation()
{
}

/*virtual*/ bool MeshMergeOperation::Perform(const ConvexPolygonMesh& meshA, const ConvexPolygonMesh& meshB, ConvexPolygonMesh& resultingMesh)
{
	return true;
}
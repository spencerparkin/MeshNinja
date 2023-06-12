#include "MeshBinaryOperation.h"
#include "ConvexPolygonMesh.h"

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
	std::vector<ConvexPolygon> polygonArray;

	meshA.ToConvexPolygonArray(polygonArray);
	meshB.ToConvexPolygonArray(polygonArray);

	resultingMesh.FromConvexPolygonArray(polygonArray);

	return true;
}
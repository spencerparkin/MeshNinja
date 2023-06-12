#include "MeshSetOperation.h"
#include "ConvexPolygonMesh.h"

using namespace MeshNinja;

//----------------------------------- MeshSetOperation -----------------------------------

MeshSetOperation::MeshSetOperation()
{
}

/*virtual*/ MeshSetOperation::~MeshSetOperation()
{
}

//----------------------------------- MeshUnion -----------------------------------

MeshUnion::MeshUnion()
{
}

/*virtual*/ MeshUnion::~MeshUnion()
{
}

/*virtual*/ bool MeshUnion::Perform(const ConvexPolygonMesh& meshA, const ConvexPolygonMesh& meshB, ConvexPolygonMesh& resultingMesh)
{
	return true;
}

//----------------------------------- MeshIntersection -----------------------------------

MeshIntersection::MeshIntersection()
{
}

/*virtual*/ MeshIntersection::~MeshIntersection()
{
}

/*virtual*/ bool MeshIntersection::Perform(const ConvexPolygonMesh& meshA, const ConvexPolygonMesh& meshB, ConvexPolygonMesh& resultingMesh)
{
	return true;
}

//----------------------------------- MeshSubtraction -----------------------------------

MeshSubtraction::MeshSubtraction()
{
}

/*virtual*/ MeshSubtraction::~MeshSubtraction()
{
}

/*virtual*/ bool MeshSubtraction::Perform(const ConvexPolygonMesh& meshA, const ConvexPolygonMesh& meshB, ConvexPolygonMesh& resultingMesh)
{
	std::vector<ConvexPolygon> polygonArray;

	meshA.ToConvexPolygonArray(polygonArray);
	meshB.ToConvexPolygonArray(polygonArray);

	resultingMesh.Clear();

	resultingMesh.FromConvexPolygonArray(polygonArray);

	return true;
}
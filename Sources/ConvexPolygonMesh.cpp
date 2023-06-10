#include "ConvexPolygonMesh.h"

using namespace MeshNinja;

ConvexPolygonMesh::ConvexPolygonMesh()
{
	this->facetArray = new std::vector<Facet>();
	this->vertexArray = new std::vector<Vertex>();
}

/*virtual*/ ConvexPolygonMesh::~ConvexPolygonMesh()
{
	delete this->facetArray;
	delete this->vertexArray;
}

void ConvexPolygonMesh::ToConvexPolygonArray(std::vector<ConvexPolygon>& convexPolygonArray) const
{
}

void ConvexPolygonMesh::FromConvexPolygonArray(const std::vector<ConvexPolygon>& convexPolygonArray)
{
}

ConvexPolygonMesh::Facet::Facet()
{
}

/*virtual*/ ConvexPolygonMesh::Facet::~Facet()
{
}
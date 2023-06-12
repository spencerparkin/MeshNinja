#include "ConvexPolygonMesh.h"

using namespace MeshNinja;

//----------------------------------- ConvexPolygonMesh -----------------------------------

ConvexPolygonMesh::ConvexPolygonMesh()
{
	this->facetArray = new std::vector<Facet>();
	this->vertexArray = new std::vector<Vector>();
}

/*virtual*/ ConvexPolygonMesh::~ConvexPolygonMesh()
{
	delete this->facetArray;
	delete this->vertexArray;
}

void ConvexPolygonMesh::Clear()
{
	this->facetArray->clear();
	this->vertexArray->clear();
}

void ConvexPolygonMesh::Compress(double eps /*= MESH_NINJA_EPS*/)
{
	std::vector<ConvexPolygon> polygonArray;

	this->ToConvexPolygonArray(polygonArray);

	for (ConvexPolygon& polygon : polygonArray)
		polygon.Compress(eps);

	this->FromConvexPolygonArray(polygonArray);
}

void ConvexPolygonMesh::ToConvexPolygonArray(std::vector<ConvexPolygon>& convexPolygonArray) const
{
	for (const Facet& facet : *this->facetArray)
	{
		ConvexPolygon polygon;
		for (int i : facet.vertexArray)
			polygon.vertexArray->push_back((*this->vertexArray)[i]);

		convexPolygonArray.push_back(polygon);
	}
}

void ConvexPolygonMesh::FromConvexPolygonArray(const std::vector<ConvexPolygon>& convexPolygonArray)
{
	this->Clear();

	std::map<Vector, int> pointMap;
	int i = 0;

	for (const ConvexPolygon& polygon : convexPolygonArray)
	{
		Facet facet;

		for (const Vector& vertex : *polygon.vertexArray)
		{
			std::map<Vector, int>::iterator iter = pointMap.find(vertex);
			if (iter == pointMap.end())
			{
				pointMap.insert(std::pair<Vector, int>(vertex, i++));
				this->vertexArray->push_back(vertex);
				iter = pointMap.find(vertex);
			}

			facet.vertexArray.push_back(iter->second);
		}

		this->facetArray->push_back(facet);
	}
}

//----------------------------------- ConvexPolygonMesh::Facet -----------------------------------

ConvexPolygonMesh::Facet::Facet()
{
}

/*virtual*/ ConvexPolygonMesh::Facet::~Facet()
{
}
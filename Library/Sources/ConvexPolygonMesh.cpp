#include "ConvexPolygonMesh.h"
#include "LineSegment.h"

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

	// TODO: Here we should reduce the polygon array by merging all coplanar polygons into a single polygon.

	this->FromConvexPolygonArray(polygonArray);
}

void ConvexPolygonMesh::NormalizeEdges(double eps /*= MESH_NINJA_EPS*/)
{
	for (Facet& facet : *this->facetArray)
	{
		int i = 0;
		while (i < (signed)facet.vertexArray.size())
		{
			int j = (i + 1) % facet.vertexArray.size();

			LineSegment edge((*this->vertexArray)[facet.vertexArray[i]], (*this->vertexArray)[facet.vertexArray[j]]);

			bool foundInteriorPoint = false;
			for (int k = 0; k < (signed)this->vertexArray->size(); k++)
			{
				const Vector& vertex = (*this->vertexArray)[k];
				if (edge.IsInteriorPoint(vertex, eps))
				{
					foundInteriorPoint = true;
					facet.vertexArray.insert(facet.vertexArray.begin() + j, k);
					break;
				}
			}

			if (!foundInteriorPoint)
				i++;
		}
	}
}

void ConvexPolygonMesh::ToConvexPolygonArray(std::vector<ConvexPolygon>& convexPolygonArray) const
{
	for (const Facet& facet : *this->facetArray)
	{
		ConvexPolygon polygon;
		facet.MakePolygon(polygon, this);
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

void ConvexPolygonMesh::Facet::MakePolygon(ConvexPolygon& polygon, const ConvexPolygonMesh* mesh) const
{
	polygon.vertexArray->clear();
	for (int i : this->vertexArray)
		polygon.vertexArray->push_back((*mesh->vertexArray)[i]);
}
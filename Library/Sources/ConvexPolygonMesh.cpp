#include "ConvexPolygonMesh.h"
#include "LineSegment.h"
#include "Plane.h"

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
	this->UntessellateFaces(eps);
}

void ConvexPolygonMesh::UntessellateFaces(double eps /*= MESH_NINJA_EPS*/)
{
}

void ConvexPolygonMesh::TessellateFaces(double eps /*= MESH_NINJA_EPS*/)
{
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

bool ConvexPolygonMesh::GenerateConvexHull(const std::vector<Vector>& pointArray, double eps /*= MESH_NINJA_EPS*/)
{
	this->Clear();

	std::list<Triangle> triangleList;

	// Hmmmm...Is there a better/faster way to find the initial tetrahedron?
	auto findInitialTetrahedron = [&pointArray, &triangleList]() -> bool {
		for (int i = 0; i < (signed)pointArray.size(); i++)
		{
			const Vector& vertexA = pointArray[i];
			for (int j = i + 1; j < (signed)pointArray.size(); j++)
			{
				const Vector& vertexB = pointArray[j];
				for (int k = j + 1; k < (signed)pointArray.size(); k++)
				{
					const Vector& vertexC = pointArray[k];
					for (int l = k + 1; l < (signed)pointArray.size(); l++)
					{
						const Vector& vertexD = pointArray[l];

						Vector xAxis = vertexB - vertexA;
						Vector yAxis = vertexC - vertexA;
						Vector zAxis = vertexD - vertexA;

						double determinant = xAxis.Cross(yAxis).Dot(zAxis);
						if (determinant > 0.0)
						{
							triangleList.push_back(Triangle{ i, k, j });
							triangleList.push_back(Triangle{ i, j, l });
							triangleList.push_back(Triangle{ j, k, l });
							triangleList.push_back(Triangle{ i, l, k });
							return true;
						}
					}
				}
			}
		}

		return false;
	};

	if (!findInitialTetrahedron())
		return false;

	auto convexHullContainsPoint = [&pointArray, &triangleList, eps](const Vector& point) -> bool {
		for (const Triangle& triangle : triangleList)
		{
			Plane plane;
			triangle.MakePlane(plane, pointArray);
			if (plane.WhichSide(point, eps) == Plane::Side::FRONT)
				return false;
		}

		return true;
	};

	auto cancelExistingTriangle = [&triangleList](const Triangle& newTriangle) -> bool {
		for (std::list<Triangle>::iterator iter = triangleList.begin(); iter != triangleList.end(); iter++)
		{
			const Triangle& existingTriangle = *iter;
			if (existingTriangle.IsCanceledBy(newTriangle))
			{
				triangleList.erase(iter);
				return true;
			}
		}
		return false;
	};

	std::list<int> pointList;
	for (int i = 0; i < (signed)pointArray.size(); i++)
		pointList.push_back(i);

	while (pointList.size() > 0)
	{
		// Remove all points contained in or on the current convex hull.
		std::list<int>::iterator iter = pointList.begin();
		while (iter != pointList.end())
		{
			std::list<int>::iterator iterNext = iter;
			iterNext++;

			const Vector& point = pointArray[*iter];
			if (convexHullContainsPoint(point))
				pointList.erase(iter);

			iter = iterNext;
		}

		// We're done once we've processed all the given points.
		if (pointList.size() == 0)
			break;

		// The first point in the list, like all others, should be one we can use to expand the convex hull further.
		int i = *pointList.begin();
		const Vector& newPoint = pointArray[i];
		std::vector<Triangle> newTriangleArray;
		for (const Triangle& existingTriangle : triangleList)
		{
			Plane plane;
			existingTriangle.MakePlane(plane, pointArray);
			if (plane.WhichSide(newPoint) == Plane::Side::FRONT)
			{
				newTriangleArray.push_back(Triangle{ existingTriangle.vertex[0], existingTriangle.vertex[1], i });
				newTriangleArray.push_back(Triangle{ existingTriangle.vertex[1], existingTriangle.vertex[2], i });
				newTriangleArray.push_back(Triangle{ existingTriangle.vertex[2], existingTriangle.vertex[0], i });
				newTriangleArray.push_back(Triangle{ existingTriangle.vertex[0], existingTriangle.vertex[2], existingTriangle.vertex[1] });
			}
		}

		// Integrate the new triangles into the existing convex hull to build a bigger one.
		for (const Triangle& newTriangle : newTriangleArray)
		{
			// A new triangle is either added to our list, or it cancels an existing triangle.
			if (!cancelExistingTriangle(newTriangle))
				triangleList.push_back(newTriangle);
		}
	}

	// Finally, build our mesh from the found list of triangles.
	std::vector<ConvexPolygon> convexPolygonArray;
	for (const Triangle& triangle : triangleList)
	{
		ConvexPolygon polygon;
		triangle.MakePolygon(polygon, pointArray);
		convexPolygonArray.push_back(polygon);
	}

	// Note that the caller may want to untessillate the faces of the returned mesh.
	this->FromConvexPolygonArray(convexPolygonArray);
	return true;
}

//----------------------------------- ConvexPolygonMesh::Triangle -----------------------------------

bool ConvexPolygonMesh::Triangle::IsCanceledBy(const Triangle& triangle) const
{
	for (int i = 0; i < 3; i++)
	{
		int j = (i + 1) % 3;
		int k = (i + 2) % 3;

		if (this->vertex[i] == triangle.vertex[2] &&
			this->vertex[j] == triangle.vertex[1] &&
			this->vertex[k] == triangle.vertex[0])
		{
			return true;
		}
	}

	return false;
}

void ConvexPolygonMesh::Triangle::MakePolygon(ConvexPolygon& polygon, const std::vector<Vector>& pointArray) const
{
	polygon.Clear();
	for (int i = 0; i < 3; i++)
		polygon.vertexArray->push_back(pointArray[this->vertex[i]]);
}

void ConvexPolygonMesh::Triangle::MakePlane(Plane& plane, const std::vector<Vector>& pointArray) const
{
	ConvexPolygon polygon;
	this->MakePolygon(polygon, pointArray);
	polygon.CalcPlane(plane);
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
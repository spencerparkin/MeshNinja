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

ConvexPolygonMesh::ConvexPolygonMesh(const ConvexPolygonMesh& mesh)
{
	this->facetArray = new std::vector<Facet>();
	this->vertexArray = new std::vector<Vector>();

	this->Copy(mesh);
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

void ConvexPolygonMesh::Copy(const ConvexPolygonMesh& mesh)
{
	this->Clear();

	for (const Vector& vertex : *mesh.vertexArray)
		this->vertexArray->push_back(vertex);

	for (const Facet& facet : *mesh.facetArray)
		this->facetArray->push_back(facet);
}

bool ConvexPolygonMesh::IsConvex(double eps /*= MESH_NINJA_EPS*/) const
{
	for (const Facet& facet : *this->facetArray)
	{
		ConvexPolygon polygon;
		facet.MakePolygon(polygon, this);

		Plane plane;
		polygon.CalcPlane(plane, eps);

		for (const Vector& vertex : *this->vertexArray)
			if (plane.WhichSide(vertex, eps) == Plane::Side::FRONT)
				return false;
	}

	return true;
}

bool ConvexPolygonMesh::IsConcave(double eps /*= MESH_NINJA_EPS*/) const
{
	return !this->IsConvex(eps);
}

void ConvexPolygonMesh::ApplyTransform(const Transform& transform)
{
	for (Vector& vertex : *this->vertexArray)
		vertex = transform.TransformPosition(vertex);
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
	std::list<Facet> facetQueue;
	for (Facet& facet : *this->facetArray)
		facetQueue.push_back(facet);

	this->facetArray->clear();

	while (facetQueue.size() > 0)
	{
		std::list<Facet>::iterator iter = facetQueue.begin();
		Facet facetA = *iter;
		facetQueue.erase(iter);

		bool mergeHappened = false;
		for (iter = facetQueue.begin(); iter != facetQueue.end(); iter++)
		{
			Facet& facetB = *iter;
			Facet mergedFacet;
			if (mergedFacet.Merge(facetA, facetB, this))
			{
				facetQueue.erase(iter);
				facetQueue.push_back(mergedFacet);
				mergeHappened = true;
				break;
			}
		}

		if (!mergeHappened)
			this->facetArray->push_back(facetA);
	}
}

void ConvexPolygonMesh::TessellateFaces(double eps /*= MESH_NINJA_EPS*/)
{
	std::list<Facet> facetQueue;
	for (Facet& facet : *this->facetArray)
		facetQueue.push_back(facet);

	this->facetArray->clear();

	while (facetQueue.size() > 0)
	{
		std::list<Facet>::iterator iter = facetQueue.begin();
		Facet facet = *iter;
		facetQueue.erase(iter);

		Facet facetA, facetB;
		if (!facet.Split(facetA, facetB, this))
			this->facetArray->push_back(facet);
		else
		{
			facetQueue.push_back(facetA);
			facetQueue.push_back(facetB);
		}
	}
}

void ConvexPolygonMesh::NormalizeEdges(double eps /*= MESH_NINJA_EPS*/)
{
	for (Facet& facet : *this->facetArray)
	{
		int i = 0;
		while (i < (signed)facet.vertexArray->size())
		{
			int j = (i + 1) % facet.vertexArray->size();

			LineSegment edge((*this->vertexArray)[(*facet.vertexArray)[i]], (*this->vertexArray)[(*facet.vertexArray)[j]]);

			bool foundInteriorPoint = false;
			for (int k = 0; k < (signed)this->vertexArray->size(); k++)
			{
				const Vector& vertex = (*this->vertexArray)[k];
				if (edge.IsInteriorPoint(vertex, eps))
				{
					foundInteriorPoint = true;
					facet.vertexArray->insert(facet.vertexArray->begin() + j, k);
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

			facet.vertexArray->push_back(iter->second);
		}

		this->facetArray->push_back(facet);
	}
}

bool ConvexPolygonMesh::GeneratePolyhedron(Polyhedron polyhedron, double eps /*= MESH_NINJA_EPS*/)
{
	std::vector<Vector> pointArray;

	auto singleCombo = [](std::function<void(double a)> callback)
	{
		for (int i = 0; i < 2; i++)
		{
			double a = (i == 0) ? -1.0 : 1.0;
			callback(a);
		}
	};

	auto doubleCombo = [](std::function<void(double a, double b)> callback)
	{
		for (int i = 0; i < 2; i++)
		{
			double a = (i == 0) ? -1.0 : 1.0;
			for (int j = 0; j < 2; j++)
			{
				double b = (j == 0) ? -1.0 : 1.0;
				callback(a, b);
			}
		}
	};

	auto tripleCombo = [](std::function<void(double a, double b, double c)> callback)
	{
		for (int i = 0; i < 2; i++)
		{
			double a = (i == 0) ? -1.0 : 1.0;
			for (int j = 0; j < 2; j++)
			{
				double b = (j == 0) ? -1.0 : 1.0;
				for (int k = 0; k < 2; k++)
				{
					double c = (k == 0) ? -1.0 : 1.0;
					callback(a, b, c);
				}
			}
		}
	};

	switch (polyhedron)
	{
		case Polyhedron::TETRAHEDRON:
		{
			singleCombo([&pointArray](double a) {
				pointArray.push_back(Vector(a, 0, -1.0 / ::sqrt(2.0)));
				pointArray.push_back(Vector(0, a, 1.0 / ::sqrt(2.0)));
			});
			break;
		}
		case Polyhedron::OCTAHEDRON:
		{
			singleCombo([&pointArray](double a) {
				pointArray.push_back(Vector(a, 0.0, 0.0));
				pointArray.push_back(Vector(0.0, a, 0.0));
				pointArray.push_back(Vector(0.0, 0.0, a));
			});
			break;
		}
		case Polyhedron::HEXADRON:
		{
			tripleCombo([&pointArray](double a, double b, double c) {
				pointArray.push_back(Vector(a, b, c));
			});
			break;
		}
		case Polyhedron::ICOSAHEDRON:
		{
			doubleCombo([&pointArray](double a, double b) {
				pointArray.push_back(Vector(0.0, a, b * MESH_NINJA_PHI));
				pointArray.push_back(Vector(a, b * MESH_NINJA_PHI, 0.0));
				pointArray.push_back(Vector(a * MESH_NINJA_PHI, 0.0, b));
			});
			break;
		}
		case Polyhedron::DODECAHEDRON:
		{
			tripleCombo([&pointArray](double a, double b, double c) {
				pointArray.push_back(Vector(a, b, c));
			});
			doubleCombo([&pointArray](double a, double b) {
				pointArray.push_back(Vector(0.0, a * MESH_NINJA_PHI, b / MESH_NINJA_PHI));
				pointArray.push_back(Vector(a / MESH_NINJA_PHI, 0.0, b * MESH_NINJA_PHI));
				pointArray.push_back(Vector(a * MESH_NINJA_PHI, b / MESH_NINJA_PHI, 0.0));
			});
			break;
		}
		case Polyhedron::ICOSIDODECAHEDRON:
		{
			singleCombo([&pointArray](double a) {
				pointArray.push_back(Vector(a * MESH_NINJA_PHI, 0.0, 0.0));
				pointArray.push_back(Vector(0.0, a * MESH_NINJA_PHI, 0.0));
				pointArray.push_back(Vector(0.0, 0.0, a * MESH_NINJA_PHI));
			});
			tripleCombo([&pointArray](double a, double b, double c) {
				pointArray.push_back(Vector(a / 2.0, b * MESH_NINJA_PHI / 2.0, c * MESH_NINJA_PHI * MESH_NINJA_PHI / 2.0));
				pointArray.push_back(Vector(a * MESH_NINJA_PHI / 2.0, b * MESH_NINJA_PHI * MESH_NINJA_PHI / 2.0, c / 2.0));
				pointArray.push_back(Vector(a * MESH_NINJA_PHI * MESH_NINJA_PHI / 2.0, b / 2.0, c * MESH_NINJA_PHI / 2.0));
			});
			break;
		}
		case Polyhedron::CUBOCTAHEDRON:
		{
			doubleCombo([&pointArray](double a, double b) {
				pointArray.push_back(Vector(a, b, 0.0));
				pointArray.push_back(Vector(a, 0.0, b));
				pointArray.push_back(Vector(0.0, a, b));
			});
			break;
		}
		case Polyhedron::RHOMBICOSIDODECAHEDRON:
		{
			tripleCombo([&pointArray](double a, double b, double c) {
				pointArray.push_back(Vector(a, b, c * MESH_NINJA_PHI * MESH_NINJA_PHI * MESH_NINJA_PHI));
				pointArray.push_back(Vector(a, b * MESH_NINJA_PHI * MESH_NINJA_PHI * MESH_NINJA_PHI, c));
				pointArray.push_back(Vector(a * MESH_NINJA_PHI * MESH_NINJA_PHI * MESH_NINJA_PHI, b, c));
				pointArray.push_back(Vector(a * MESH_NINJA_PHI * MESH_NINJA_PHI, b * MESH_NINJA_PHI, 2.0 * c * MESH_NINJA_PHI));
				pointArray.push_back(Vector(a * MESH_NINJA_PHI, 2.0 * b * MESH_NINJA_PHI, c * MESH_NINJA_PHI * MESH_NINJA_PHI));
				pointArray.push_back(Vector(2.0 * a * MESH_NINJA_PHI, b * MESH_NINJA_PHI * MESH_NINJA_PHI, c * MESH_NINJA_PHI));
			});
			doubleCombo([&pointArray](double a, double b) {
				pointArray.push_back(Vector(a * (2.0 + MESH_NINJA_PHI), 0.0, b * MESH_NINJA_PHI * MESH_NINJA_PHI));
				pointArray.push_back(Vector(0.0, a * MESH_NINJA_PHI * MESH_NINJA_PHI, b * (2.0 + MESH_NINJA_PHI)));
				pointArray.push_back(Vector(a * MESH_NINJA_PHI * MESH_NINJA_PHI, b * (2.0 + MESH_NINJA_PHI), 0.0));
			});
			break;
		}
	}

	return this->GenerateConvexHull(pointArray);
}

// Interestingly, this algorithm is also a proof that any polyhedron is the sum of tetrahedrons.
bool ConvexPolygonMesh::GenerateConvexHull(const std::vector<Vector>& pointArray, double eps /*= MESH_NINJA_EPS*/)
{
	this->Clear();

	std::list<Triangle> triangleList;

	// Hmmmm...Is there a better/faster way to find the initial tetrahedron?
	auto findInitialTetrahedron = [&pointArray, &triangleList]() -> bool
	{
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

	auto convexHullContainsPoint = [&pointArray, &triangleList, eps](const Vector& point) -> bool
	{
		for (const Triangle& triangle : triangleList)
		{
			Plane plane;
			triangle.MakePlane(plane, pointArray);
			if (plane.WhichSide(point, eps) == Plane::Side::FRONT)
				return false;
		}

		return true;
	};

	auto cancelExistingTriangle = [&triangleList](const Triangle& newTriangle) -> bool
	{
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

// Note that we can do better than O(N) here if we used a point-cloud.
// In that case, we would get O(log N), I believe.
int ConvexPolygonMesh::FindClosestPointTo(const Vector& point, double* smallestDistance /*= nullptr*/) const
{
	double smallestDistanceStorage = 0.0;
	if (!smallestDistance)
		smallestDistance = &smallestDistanceStorage;

	*smallestDistance = DBL_MAX;
	int j = -1;
	for (int i = 0; i < (signed)this->vertexArray->size(); i++)
	{
		double distance = ((*this->vertexArray)[i] - point).Length();
		if (distance < *smallestDistance)
		{
			*smallestDistance = distance;
			j = i;
		}
	}

	return j;
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
	this->vertexArray = new std::vector<int>();
}

ConvexPolygonMesh::Facet::Facet(const Facet& facet)
{
	this->vertexArray = new std::vector<int>();
	for (int i : *facet.vertexArray)
		this->vertexArray->push_back(i);
}

/*virtual*/ ConvexPolygonMesh::Facet::~Facet()
{
	delete this->vertexArray;
}

void ConvexPolygonMesh::Facet::MakePolygon(ConvexPolygon& polygon, const ConvexPolygonMesh* mesh) const
{
	polygon.vertexArray->clear();
	for (int i : *this->vertexArray)
		polygon.vertexArray->push_back((*mesh->vertexArray)[i]);
}

// Note that the resulting facet might be concave!
bool ConvexPolygonMesh::Facet::Merge(const Facet& facetA, const Facet& facetB, const ConvexPolygonMesh* mesh)
{
	ConvexPolygon polygonA, polygonB;
	facetA.MakePolygon(polygonA, mesh);
	facetB.MakePolygon(polygonB, mesh);

	Plane planeA, planeB;
	polygonA.CalcPlane(planeA);
	polygonB.CalcPlane(planeB);

	if (!planeA.IsEqualTo(planeB))
		return false;

	struct Edge
	{
		int i, j;
	};

	std::list<Edge> edgeList;
	int cancelationCount = 0;

	auto integrateEdge = [&edgeList, &cancelationCount](Edge newEdge)
	{
		// The new edge might just cancel an existing edge.
		for (std::list<Edge>::iterator iter = edgeList.begin(); iter != edgeList.end(); iter++)
		{
			const Edge& existingEdge = *iter;
			if (existingEdge.i == newEdge.j && existingEdge.j == newEdge.i)
			{
				edgeList.erase(iter);
				cancelationCount++;
				return;
			}
		}

		// The new edge might truncate an existing edge.  One or two parts of the existing edge may survive.  One or two parts of the new edge may survive.
#if 0				// This is a case we really should handle, but I'm going to punt on it for now as it is a bit complex.
		for (std::list<Edge>::iterator iter = edgeList.begin(); iter != edgeList.end(); iter++)
		{
			const Edge& existingEdge = *iter;
			//...
		}
#endif

		// At this point we just add the new edge to the list.
		edgeList.push_back(newEdge);
	};

	for (int i = 0; i < (signed)facetA.vertexArray->size(); i++)
	{
		int j = (i + 1) % facetA.vertexArray->size();
		integrateEdge(Edge{ (*facetA.vertexArray)[i], (*facetA.vertexArray)[j] });
	}

	for (int i = 0; i < (signed)facetB.vertexArray->size(); i++)
	{
		int j = (i + 1) % facetB.vertexArray->size();
		integrateEdge(Edge{ (*facetB.vertexArray)[i], (*facetB.vertexArray)[j] });
	}

	// Reject the merge if no cancelation occurred, which indicates that the two facets don't share at least one edge.
	if (cancelationCount == 0)
		return false;

	this->vertexArray->clear();

	if (edgeList.size() > 0)
	{
		this->vertexArray->push_back((*edgeList.begin()).i);
		bool polygonComplete = false;
		while (!polygonComplete)
		{
			int i = (*this->vertexArray)[this->vertexArray->size() - 1];
			for (const Edge& edge : edgeList)
			{
				if (edge.i == i)
				{
					if (edge.j != (*this->vertexArray)[0])
						this->vertexArray->push_back(edge.j);
					else
						polygonComplete = true;
					break;
				}
			}

			// Something has gone wrong in this case.
			if (this->vertexArray->size() > edgeList.size())
				return false;
		}
	}

	return true;
}

bool ConvexPolygonMesh::Facet::Split(Facet& facetA, Facet& facetB, const ConvexPolygonMesh* mesh) const
{
	if (this->vertexArray->size() <= 3)
		return false;

	struct SplitCase
	{
		int i, j;
	};

	auto formulateSplit = [this, &facetA, &facetB](const SplitCase& splitCase)
	{
		facetA.vertexArray->clear();
		facetB.vertexArray->clear();

		int k = splitCase.i;

		while (true)
		{
			facetA.vertexArray->push_back((*this->vertexArray)[k]);
			if (k == splitCase.j)
				break;
			k = (k + 1) % this->vertexArray->size();
		}

		while (true)
		{
			facetB.vertexArray->push_back((*this->vertexArray)[k]);
			if (k == splitCase.i)
				break;
			k = (k + 1) % this->vertexArray->size();
		}
	};

	double largestSmallestInteriorAngle = -DBL_MAX;
	SplitCase bestSplitCase{ -1, -1 };

	for (int i = 0; i < (signed)this->vertexArray->size(); i++)
	{
		for (int j = 0; j < (signed)this->vertexArray->size(); j++)
		{
			if (i < j && i != (j + 1) % this->vertexArray->size() && j != (i + 1) % this->vertexArray->size())
			{
				SplitCase splitCase{ i, j };
				formulateSplit(splitCase);

				AngleStats angleStatsA, angleStatsB;

				facetA.CalcInteriorAngleStats(angleStatsA, mesh);
				facetB.CalcInteriorAngleStats(angleStatsB, mesh);

				double smallestInteriorAngle = MESH_NINJA_MIN(angleStatsA.smallestInteriorAngle, angleStatsB.smallestInteriorAngle);
				if (smallestInteriorAngle > largestSmallestInteriorAngle)
				{
					largestSmallestInteriorAngle = smallestInteriorAngle;
					bestSplitCase = splitCase;
				}
			}
		}
	}

	if (bestSplitCase.i < 0 || bestSplitCase.j < 0)
		return false;

	formulateSplit(bestSplitCase);
	return true;
}

bool ConvexPolygonMesh::Facet::CalcInteriorAngleStats(AngleStats& angleStats, const ConvexPolygonMesh* mesh) const
{
	if (this->vertexArray->size() < 3)
		return false;

	angleStats.smallestInteriorAngle = DBL_MAX;
	angleStats.largestInteriorAngle = -DBL_MAX;

	for (int i = 0; i < (signed)this->vertexArray->size(); i++)
	{
		int j = (i + 1) % this->vertexArray->size();
		int k = (i + 2) % this->vertexArray->size();

		const Vector& edgeA = (*mesh->vertexArray)[i] - (*mesh->vertexArray)[j];
		const Vector& edgeB = (*mesh->vertexArray)[k] - (*mesh->vertexArray)[j];

		double angle = edgeA.AngleBetweenThisAnd(edgeB);

		angleStats.smallestInteriorAngle = MESH_NINJA_MIN(angleStats.smallestInteriorAngle, angle);
		angleStats.largestInteriorAngle = MESH_NINJA_MAX(angleStats.largestInteriorAngle, angle);
	}

	return true;
}

bool ConvexPolygonMesh::Facet::HasVertex(int i) const
{
	for (int j : *this->vertexArray)
		if (i == j)
			return true;

	return false;
}
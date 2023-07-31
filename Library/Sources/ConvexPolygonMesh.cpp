#include "ConvexPolygonMesh.h"
#include "LineSegment.h"
#include "Plane.h"
#include "SpaceCurve.h"

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

ConvexPolygonMesh::ConvexPolygonMesh(const std::vector<ConvexPolygon>& polygonArray)
{
	this->facetArray = new std::vector<Facet>();
	this->vertexArray = new std::vector<Vector>();

	this->FromConvexPolygonArray(polygonArray);
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

bool ConvexPolygonMesh::AllFacetsValid(double eps /*= MESH_NINJA_EPS*/) const
{
	for (const Facet& facet : *this->facetArray)
	{
		ConvexPolygon polygon;
		facet.MakePolygon(polygon, this);

		if (!polygon.VerticesAreCoplanar(eps))
			return false;

		if (!polygon.IsConvex(eps))
			return false;
	}

	return true;
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
					if (j == 0)
						facet.vertexArray->push_back(k);
					else
						facet.vertexArray->insert(facet.vertexArray->begin() + j, k);
					break;
				}
			}

			if (!foundInteriorPoint)
				i++;
		}
	}
}

void ConvexPolygonMesh::ToConvexPolygonArray(std::vector<ConvexPolygon>& convexPolygonArray, bool concatinate /*= true*/) const
{
	if (!concatinate)
		convexPolygonArray.clear();

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
						if (determinant > MESH_NINJA_EPS)
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

void ConvexPolygonMesh::ReverseAllPolygons()
{
	for (Facet& facet : *this->facetArray)
		facet.Reverse();
}

// Sometimes it makes sense to add vertices to a mesh, even though doing so does not make any visual change to the mesh when rendered.
// If the given vertex is not on this mesh, we return false.
// If it is one of our vertices, nothing changes, and we return true.
// If it is on one of our edges, then the edge is split for all polygons using it, and the vertex is added, and we return true.
// If it is on a face, then for now, we just return false, but it would make sense to tessellate in this case.
bool ConvexPolygonMesh::AddRedundantVertex(const Vector& vertex, double eps /*= MESH_NINJA_EPS*/)
{
	double smallestDistance = DBL_MAX;
	int i = this->FindClosestPointTo(vertex, &smallestDistance);
	if (i >= 0 && smallestDistance < eps)
		return true;

	bool addVertex = false;

	for (Facet& facet : *this->facetArray)
	{
		for (int i = 0; i < (signed)facet.vertexArray->size(); i++)
		{
			int j = (i + 1) % facet.vertexArray->size();

			LineSegment edgeSegment((*this->vertexArray)[facet[i]], (*this->vertexArray)[facet[j]]);

			if (edgeSegment.ContainsPoint(vertex, eps))
			{
				facet.vertexArray->insert(facet.vertexArray->begin() + j, this->vertexArray->size());
				addVertex = true;
				break;
			}
		}
	}

	if (addVertex)
		this->vertexArray->push_back(vertex);

	return addVertex;
}

bool ConvexPolygonMesh::GenerateSphere(double radius, int segments, int slices)
{
	if (segments < 3 || slices < 3)
		return false;

	std::vector<ConvexPolygon> polygonArray;

	Vector** pointMatrix = new Vector*[segments + 1];
	for (int i = 0; i <= segments; i++)
		pointMatrix[i] = new Vector[slices];

	for (int i = 0; i <= segments; i++)
	{
		double phi = MESH_NINJA_PI * double(i) / double(segments);

		double cosPhi = ::cos(phi);
		double sinPhi = ::sin(phi);

		for (int j = 0; j < slices; j++)
		{
			double theta = 2.0 * MESH_NINJA_PI * double(j) / double(slices);

			double cosTheta = ::cos(theta);
			double sinTheta = ::sin(theta);

			Vector vertex;

			double segmentRadius = radius * sinPhi;

			vertex.x = segmentRadius * cosTheta;
			vertex.y = segmentRadius * sinTheta;
			vertex.z = radius * cosPhi;

			pointMatrix[i][j] = vertex;
		}
	}

	for (int j = 0; j < slices; j++)
	{
		ConvexPolygon polygon;

		polygon.vertexArray->push_back(pointMatrix[0][0]);
		polygon.vertexArray->push_back(pointMatrix[1][j]);
		polygon.vertexArray->push_back(pointMatrix[1][(j + 1) % slices]);

		polygonArray.push_back(polygon);

		polygon.Clear();

		polygon.vertexArray->push_back(pointMatrix[segments][0]);
		polygon.vertexArray->push_back(pointMatrix[segments - 1][(j + 1) % slices]);
		polygon.vertexArray->push_back(pointMatrix[segments - 1][j]);

		polygonArray.push_back(polygon);
	}

	for (int i = 1; i < segments - 1; i++)
	{
		for (int j = 0; j < slices; j++)
		{
			ConvexPolygon polygon;

			polygon.vertexArray->push_back(pointMatrix[i][j]);
			polygon.vertexArray->push_back(pointMatrix[i + 1][j]);
			polygon.vertexArray->push_back(pointMatrix[i + 1][(j + 1) % slices]);
			polygon.vertexArray->push_back(pointMatrix[i][(j + 1) % slices]);

			polygonArray.push_back(polygon);
		}
	}

	for (int i = 0; i <= segments; i++)
		delete[] pointMatrix[i];
	delete[] pointMatrix;

	this->FromConvexPolygonArray(polygonArray);
	return true;
}

bool ConvexPolygonMesh::GenerateCylinder(double length, double radius, int segments, int slices)
{
	if (segments < 3 || slices < 3)
		return false;

	std::vector<ConvexPolygon> polygonArray;

	Vector** pointMatrix = new Vector*[segments + 1];
	for (int i = 0; i <= segments; i++)
		pointMatrix[i] = new Vector[slices];

	for (int i = 0; i <= segments; i++)
	{
		for (int j = 0; j < slices; j++)
		{
			double theta = 2.0 * MESH_NINJA_PI * double(j) / double(slices);

			double cosTheta = ::cos(theta);
			double sinTheta = ::sin(theta);

			Vector vertex;

			vertex.x = radius * cosTheta;
			vertex.y = radius * sinTheta;
			vertex.z = -length / 2.0 + length * double(i) / double(segments);

			pointMatrix[i][j] = vertex;
		}
	}

	ConvexPolygon polygonCapA;
	for (int j = slices - 1; j >= 0; j--)
		polygonCapA.vertexArray->push_back(pointMatrix[0][j]);
	polygonArray.push_back(polygonCapA);

	ConvexPolygon polygonCapB;
	for (int j = 0; j < slices; j++)
		polygonCapB.vertexArray->push_back(pointMatrix[segments][j]);
	polygonArray.push_back(polygonCapB);

	for (int i = 0; i < segments; i++)
	{
		for (int j = 0; j < slices; j++)
		{
			ConvexPolygon polygon;

			polygon.vertexArray->push_back(pointMatrix[i][(j + 1) % slices]);
			polygon.vertexArray->push_back(pointMatrix[i + 1][(j + 1) % slices]);
			polygon.vertexArray->push_back(pointMatrix[i + 1][j]);
			polygon.vertexArray->push_back(pointMatrix[i][j]);

			polygonArray.push_back(polygon);
		}
	}

	for (int i = 0; i <= segments; i++)
		delete[] pointMatrix[i];
	delete[] pointMatrix;

	this->FromConvexPolygonArray(polygonArray);
	return true;
}

bool ConvexPolygonMesh::GenerateTorus(double innerRadius, double outerRadius, int segments, int slices)
{
	if (segments < 3 || slices < 3)
		return false;

	std::vector<ConvexPolygon> polygonArray;

	double majorRadius = (outerRadius + innerRadius) / 2.0;
	double minorRadius = (outerRadius - innerRadius) / 2.0;

	Vector** pointMatrix = new Vector*[slices];
	for (int i = 0; i < slices; i++)
		pointMatrix[i] = new Vector[segments];

	for (int i = 0; i < slices; i++)
	{
		double phi = 2.0 * MESH_NINJA_PI * double(i) / double(slices);

		Vector majorVector;

		majorVector.x = majorRadius * ::cos(phi);
		majorVector.y = majorRadius * ::sin(phi);
		majorVector.z = 0.0;

		for (int j = 0; j < segments; j++)
		{
			double theta = 2.0 * MESH_NINJA_PI * double(j) / double(segments);

			Vector xAxis = majorVector.Normalized();
			Vector yAxis(0.0, 0.0, 1.0);

			Vector minorVector = xAxis * minorRadius * ::cos(theta) + yAxis * minorRadius * ::sin(theta);

			pointMatrix[i][j] = majorVector + minorVector;
		}
	}

	for (int i = 0; i < slices; i++)
	{
		for (int j = 0; j < segments; j++)
		{
			ConvexPolygon polygon;

			polygon.vertexArray->push_back(pointMatrix[i][j]);
			polygon.vertexArray->push_back(pointMatrix[(i + 1) % slices][j]);
			polygon.vertexArray->push_back(pointMatrix[(i + 1) % slices][(j + 1) % segments]);
			polygon.vertexArray->push_back(pointMatrix[i][(j + 1) % segments]);

			polygonArray.push_back(polygon);
		}
	}

	for (int i = 0; i < slices; i++)
		delete[] pointMatrix[i];
	delete[] pointMatrix;

	this->FromConvexPolygonArray(polygonArray);
	return true;
}

bool ConvexPolygonMesh::GenerateMobiusStrip(double width, double radius, int segments)
{
	if (segments < 3)
		return false;

	std::vector<ConvexPolygon> polygonArray;

	Vector** pointMatrix = new Vector*[segments];
	for (int i = 0; i < segments; i++)
		pointMatrix[i] = new Vector[2];

	for (int i = 0; i < segments; i++)
	{
		double theta = 2.0 * MESH_NINJA_PI * double(i) / double(segments);

		Vector majorVector;

		majorVector.x = radius * ::cos(theta);
		majorVector.y = radius * ::sin(theta);
		majorVector.z = 0.0;

		double phi = MESH_NINJA_PI * double(i) / double(segments);

		Vector xAxis = majorVector.Normalized();
		Vector yAxis(0.0, 0.0, 1.0);

		Vector minorVector = xAxis * width * ::cos(phi) + yAxis * width * ::sin(phi);

		pointMatrix[i][0] = majorVector + minorVector;
		pointMatrix[i][1] = majorVector - minorVector;
	}

	for (int i = 0; i < segments; i++)
	{
		ConvexPolygon polygon;

		if (i < segments - 1)
		{
			polygon.vertexArray->push_back(pointMatrix[i][0]);
			polygon.vertexArray->push_back(pointMatrix[i + 1][0]);
			polygon.vertexArray->push_back(pointMatrix[i + 1][1]);
			polygon.vertexArray->push_back(pointMatrix[i][1]);
		}
		else
		{
			polygon.vertexArray->push_back(pointMatrix[i][0]);
			polygon.vertexArray->push_back(pointMatrix[0][1]);
			polygon.vertexArray->push_back(pointMatrix[0][0]);
			polygon.vertexArray->push_back(pointMatrix[i][1]);
		}

		polygonArray.push_back(polygon);
	}

	for (int i = 0; i < segments; i++)
		delete[] pointMatrix[i];
	delete[] pointMatrix;

	this->FromConvexPolygonArray(polygonArray);
	return true;
}

bool ConvexPolygonMesh::GenerateKleinBottle(int segments)
{
	CompositeBezierCurve curve;

	curve.controlPointArray->push_back(CompositeBezierCurve::ControlPoint{ Vector(0.0, 16.0, 0.0), Vector(0.0, -2.0, 0.0) });
	curve.controlPointArray->push_back(CompositeBezierCurve::ControlPoint{ Vector(-1.0, 0.0, 0.0), Vector(0.0, -2.0, 0.0) });
	curve.controlPointArray->push_back(CompositeBezierCurve::ControlPoint{ Vector(3.0, -4.0, 0.0), Vector(2.0, 0.0, 0.0) });
	curve.controlPointArray->push_back(CompositeBezierCurve::ControlPoint{ Vector(7.0, 0.0, 0.0), Vector(0.0, 3.0, 0.0) });
	curve.controlPointArray->push_back(CompositeBezierCurve::ControlPoint{ Vector(0.0, 16.0, 0.0), Vector(0.0, 8.0, 0.0) });

	curve.GenerateTubeMesh(*this, 1.0, 16, [](double t) -> double
		{
			if (t < 0.2)
			{
				return 3.0 - 2.0 * MESH_NINJA_SQUARED(t / 0.2) + 4.0 * ::sin((t / 0.2) * MESH_NINJA_PI);
			}

			if (t > 0.8)
			{
				return 1.0 + MESH_NINJA_SQUARED((t - 0.8) / 0.2) * 2.0;
			}

			return 1.0;
		});

	return true;
}

Vector ConvexPolygonMesh::CalcCenter() const
{
	Vector center(0.0, 0.0, 0.0);

	if (this->vertexArray->size() > 0)
	{
		for (const Vector& vertex : *this->vertexArray)
			center += vertex;

		center /= double(this->vertexArray->size());
	}

	return center;
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

void ConvexPolygonMesh::Facet::Reverse()
{
	std::vector<int> reverseVertexArray;
	for (int i = (signed)this->vertexArray->size(); i >= 0; i--)
		reverseVertexArray.push_back((*this)[i]);

	this->vertexArray->clear();
	for (int i : reverseVertexArray)
		this->vertexArray->push_back(i);
}

void ConvexPolygonMesh::CenterAndScale(double radius)
{
	Vector center = this->CalcCenter();
	for (Vector& vertex : *this->vertexArray)
		vertex -= center;

	double maxLength = DBL_MIN;
	for (Vector& vertex : *this->vertexArray)
	{
		double length = vertex.Length();
		if (length > maxLength)
			maxLength = length;
	}

	if (maxLength > 0.0)
	{
		double scale = radius / maxLength;
		for (Vector& vertex : *this->vertexArray)
			vertex *= scale;
	}
}
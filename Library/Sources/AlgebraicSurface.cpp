#include "AlgebraicSurface.h"
#include "ConvexPolygonMesh.h"
#include "Ray.h"
#include "Plane.h"
#include "AxisAlignedBoundingBox.h"
#if defined MESH_NINJA_DEBUG
#	include "MeshFileFormat.h"
#endif //MESH_NINJA_DEBUG

using namespace MeshNinja;

//--------------------------------- AlgebraicSurface ---------------------------------

AlgebraicSurface::AlgebraicSurface()
{
}

/*virtual*/ AlgebraicSurface::~AlgebraicSurface()
{
}

/*virtual*/ Vector AlgebraicSurface::EvaluateGradient(const Vector& point) const
{
	return Vector(0.0, 0.0, 0.0);	// TODO: Approximate gradient here using central differencing?
}

/*virtual*/ double AlgebraicSurface::EvaluateDirectionalDerivative(const Vector& point, const Vector& unitDirection) const
{
	return this->EvaluateGradient(point).Dot(unitDirection);
}

/*virtual*/ double AlgebraicSurface::ApproximateDirectionalDerivative(const Vector& point, const Vector& unitDirection, double delta) const
{
	return (this->Evaluate(point + unitDirection * delta) - this->Evaluate(point)) / delta;
}

bool AlgebraicSurface::GenerateMesh(ConvexPolygonMesh& mesh, const Ray& initialContactRay, const AxisAlignedBoundingBox& aabb, double approximateEdgeLength) const
{
	mesh.Clear();

	double alpha = 0.0;
	if (!initialContactRay.CastAgainst(*this, alpha, MESH_NINJA_EPS, 1000, 1.0, false))
		return false;

	Vector surfacePointA = initialContactRay.Lerp(alpha);
	if (!aabb.ContainsPoint(surfacePointA))
		return false;

	Vector normalDirection = this->EvaluateGradient(surfacePointA).Normalized();
	Vector tangentDirection;
	if (!tangentDirection.MakeOrthogonalTo(normalDirection))
		return false;

	tangentDirection.Normalize();
	Ray ray(surfacePointA + tangentDirection * approximateEdgeLength, normalDirection);
	if (!ray.CastAgainst(*this, alpha, MESH_NINJA_EPS, 1000, 1.0, true))
		return false;

	Vector surfacePointB = ray.Lerp(alpha);

	mesh.vertexArray->push_back(surfacePointA);
	mesh.vertexArray->push_back(surfacePointB);

	struct Edge
	{
		int i, j;
	};

	std::list<Edge> edgeQueue;
	edgeQueue.push_back(Edge{ 0, 1 });
	edgeQueue.push_back(Edge{ 1, 0 });

	while (edgeQueue.size() > 0)
	{
		std::list<Edge>::iterator iter = edgeQueue.begin();
		Edge edge = *iter;
		edgeQueue.erase(iter);

		if (!aabb.ContainsPoint((*mesh.vertexArray)[edge.i]) || !aabb.ContainsPoint((*mesh.vertexArray)[edge.j]))
			continue;

		std::vector<Edge> newEdgeArray;
		ConvexPolygonMesh::Facet newFacet;

		Vector normalVector = this->EvaluateGradient((*mesh.vertexArray)[edge.i]);
		Vector edgeVector = (*mesh.vertexArray)[edge.j] - (*mesh.vertexArray)[edge.i];
		Vector tangentVector = normalVector.Cross(edgeVector);
		Plane edgePlane((*mesh.vertexArray)[edge.i], tangentVector);

		// Merge two edges together first if we can.
		for (iter = edgeQueue.begin(); iter != edgeQueue.end(); iter++)
		{
			const Edge& adjacentEdge = *iter;
			if (adjacentEdge.j == edge.i && adjacentEdge.i != edge.j && edgePlane.WhichSide((*mesh.vertexArray)[adjacentEdge.i]) == Plane::Side::FRONT)
			{
				Vector vectorA = (*mesh.vertexArray)[adjacentEdge.i] - (*mesh.vertexArray)[adjacentEdge.j];
				Vector vectorB = (*mesh.vertexArray)[edge.j] - (*mesh.vertexArray)[edge.i];
				double angle = vectorA.AngleBetweenThisAnd(vectorB);
				if (angle < MESH_NINJA_PI / 2.0)
				{
					newEdgeArray.push_back(Edge{ adjacentEdge.i, edge.j });
					newFacet.vertexArray.push_back(adjacentEdge.i);
					newFacet.vertexArray.push_back(edge.i);
					newFacet.vertexArray.push_back(edge.j);
					edgeQueue.erase(iter);
					break;
				}
			}
			else if (edge.j == adjacentEdge.i && edge.i != adjacentEdge.j && edgePlane.WhichSide((*mesh.vertexArray)[adjacentEdge.j]) == Plane::Side::FRONT)
			{
				Vector vectorA = (*mesh.vertexArray)[adjacentEdge.j] - (*mesh.vertexArray)[adjacentEdge.i];
				Vector vectorB = (*mesh.vertexArray)[edge.i] - (*mesh.vertexArray)[edge.j];
				double angle = vectorA.AngleBetweenThisAnd(vectorB);
				if (angle < MESH_NINJA_PI / 2.0)
				{
					newEdgeArray.push_back(Edge{ edge.i, adjacentEdge.j });
					newFacet.vertexArray.push_back(adjacentEdge.j);
					newFacet.vertexArray.push_back(edge.i);
					newFacet.vertexArray.push_back(edge.j);
					edgeQueue.erase(iter);
					break;
				}
			}
		}

		// Okay, we should have enough room to create a new flap.
		if (newFacet.vertexArray.size() == 0)
		{
			
			double length = approximateEdgeLength * ::sqrt(3.0) / 2.0;
			Vector point = (*mesh.vertexArray)[edge.i] + (edgeVector / 2.0) + (tangentVector.Normalized() * length);
			ray = Ray(point, normalVector);
			if (!ray.CastAgainst(*this, alpha, MESH_NINJA_EPS, 1000, 1.0, true))
				return false;

			Vector surfacePointC = ray.Lerp(alpha);
			double smallestDistance = 0.0;
			int i = mesh.FindClosestPointTo(surfacePointC, &smallestDistance);
			if (i < 0 || smallestDistance > approximateEdgeLength / 2.0)
			{
				i = mesh.vertexArray->size();
				mesh.vertexArray->push_back(surfacePointC);
			}

			newFacet.vertexArray.push_back(edge.i);
			newFacet.vertexArray.push_back(edge.j);
			newFacet.vertexArray.push_back(i);

			newEdgeArray.push_back(Edge{ i, edge.j });
			newEdgeArray.push_back(Edge{ edge.i, i });
		}

		mesh.facetArray->push_back(newFacet);

		for (const Edge& newEdge : newEdgeArray)
		{
			bool edgeCanceled = false;
			for (iter = edgeQueue.begin(); iter != edgeQueue.end(); iter++)
			{
				const Edge& queuedEdge = *iter;
				if (queuedEdge.i == newEdge.j && queuedEdge.j == newEdge.i)
				{
					edgeCanceled = true;
					edgeQueue.erase(iter);
					break;
				}
			}

			if (!edgeCanceled)
				edgeQueue.push_back(newEdge);
		}
	}

	return true;
}

//--------------------------------- QuadraticSurface ---------------------------------

QuadraticSurface::QuadraticSurface()
{
	this->a = 0.0;
	this->b = 0.0;
	this->c = 0.0;
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 0.0;
	this->j = 0.0;
}

/*virtual*/ QuadraticSurface::~QuadraticSurface()
{
}

/*virtual*/ double QuadraticSurface::Evaluate(const Vector& point) const
{
	double x = point.x;
	double y = point.y;
	double z = point.z;
	double xx = x * x;
	double xy = x * y;
	double xz = x * z;
	double yy = y * y;
	double yz = y * z;
	double zz = z * z;

	double result =
		this->a * xx + this->b * yy + this->c * zz +
		this->d * xy + this->e * yz + this->f * xz +
		this->g * x + this->h * y + this->i * z +
		this->j;

	return result;
}

/*virtual*/ Vector QuadraticSurface::EvaluateGradient(const Vector& point) const
{
	double x = point.x;
	double y = point.y;
	double z = point.z;

	return Vector(
		2.0 * this->a * x + this->d * y + this->f * z + this->g,
		2.0 * this->b * y + this->d * x + this->e * z + this->h,
		2.0 * this->c * z + this->e * y + this->f * x + this->i);
}

void QuadraticSurface::MakeEllipsoid(double A, double B, double C)
{
	this->a = 1.0 / (A * A);
	this->b = 1.0 / (B * B);
	this->c = 1.0 / (C * C);
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 0.0;
	this->j = -1.0;
}

void QuadraticSurface::MakeEllipticCone(double A, double B, double C)
{
	this->a = 1.0 / (A * A);
	this->b = 1.0 / (B * B);
	this->c = -1.0 / (C * C);
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 0.0;
	this->j = 0.0;
}

void QuadraticSurface::MakeEllipticCylinder(double A, double B)
{
	this->a = 1.0 / (A * A);
	this->b = 1.0 / (B * B);
	this->c = 0.0;
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 0.0;
	this->j = -1.0;
}

void QuadraticSurface::MakeEllipticParaboloid(double A, double B)
{
	this->a = 1.0 / (A * A);
	this->b = 1.0 / (B * B);
	this->c = 0.0;
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 1.0;
	this->j = 0.0;
}

void QuadraticSurface::MakeHyperbolicCylinder(double A, double B)
{
	this->a = 1.0 / (A * A);
	this->b = -1.0 / (B * B);
	this->c = 0.0;
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 0.0;
	this->j = 1.0;
}

void QuadraticSurface::MakeHyperbolicParaboloid(double A, double B)
{
	this->a = -1.0 / (A * A);
	this->b = 1.0 / (B * B);
	this->c = 0.0;
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = -1.0;
	this->j = 0.0;
}

void QuadraticSurface::MakeHyperboloidOfOneSheet(double A, double B, double C)
{
	this->a = 1.0 / (A * A);
	this->b = 1.0 / (B * B);
	this->c = -1.0 / (C * C);
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 0.0;
	this->j = 1.0;
}

void QuadraticSurface::MakeHyperboloidOfTwoSheets(double A, double B, double C)
{
	this->a = 1.0 / (A * A);
	this->b = 1.0 / (B * B);
	this->c = -1.0 / (C * C);
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 0.0;
	this->j = -1.0;
}

void QuadraticSurface::MakeIntersectingPlanes(double A, double B)
{
	this->a = 1.0 / (A * A);
	this->b = -1.0 / (B * B);
	this->c = 0.0;
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 0.0;
	this->j = 0.0;
}

void QuadraticSurface::MakeParabolicSylinder(double R)
{
	this->a = 1.0;
	this->b = 0.0;
	this->c = 0.0;
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 2.0 * R;
	this->j = 0.0;
}

void QuadraticSurface::MakeParallelPlanes(double A)
{
	this->a = 1.0;
	this->b = 0.0;
	this->c = 0.0;
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 0.0;
	this->j = -A * A;
}
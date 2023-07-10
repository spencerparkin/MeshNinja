#include "AlgebraicSurface.h"
#include "ConvexPolygonMesh.h"
#include "Ray.h"
#include "AxisAlignedBoundingBox.h"

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
	if (!initialContactRay.CastAgainst(*this, alpha, MESH_NINJA_EPS, 100, 1.0, false))
		return false;

	Vector surfacePointA = initialContactRay.Lerp(alpha);
	if (!aabb.ContainsPoint(surfacePointA))
		return false;

	Vector normalDirection = this->EvaluateGradient(surfacePointA);
	Vector tangentDirection;
	if (!tangentDirection.MakeOrthogonalTo(normalDirection))
		return false;

	Ray ray(surfacePointA + tangentDirection.Normalized() * approximateEdgeLength, normalDirection);
	if (!ray.CastAgainst(*this, alpha, MESH_NINJA_EPS, 100, 1.0, true))
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

		Vector edgeVector = (*mesh.vertexArray)[edge.j] - (*mesh.vertexArray)[edge.i];
		Vector normalVector = this->EvaluateGradient((*mesh.vertexArray)[edge.i]);
		Vector tangentVector = normalVector.Cross(edgeVector);
		double length = approximateEdgeLength * ::sqrt(3.0) / 2.0;
		Vector point = (*mesh.vertexArray)[edge.i] + (edgeVector / 2.0) + (tangentVector.Normalized() * length);
		ray = Ray(point, normalVector);
		if (!ray.CastAgainst(*this, alpha, MESH_NINJA_EPS, 100, 1.0, true))
			return false;

		Vector surfacePointC = ray.Lerp(alpha);
		double smallestDistance = 0.0;
		int i = mesh.FindClosestPointTo(surfacePointC, &smallestDistance);
		if (i < 0 || smallestDistance > approximateEdgeLength / 2.0)
		{
			i = mesh.vertexArray->size();
			mesh.vertexArray->push_back(surfacePointC);
		}

		Edge newEdge[2] = { Edge{edge.j, i}, Edge{i, edge.i} };
		for (int j = 0; j < 2; j++)
		{
			bool edgeCanceled = false;
			for (const Edge& queuedEdge : edgeQueue)
			{
				if (queuedEdge.i == newEdge[j].j && queuedEdge.j == newEdge[j].i)
				{
					edgeCanceled = true;
					break;
				}
			}

			if (!edgeCanceled)
				edgeQueue.push_back(newEdge[j]);
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
		a * xx + b * yy + c * zz +
		d * xy + e * yz + f * xz +
		g * x + h * y + i + z * z;

	return result;
}

/*virtual*/ Vector QuadraticSurface::EvaluateGradient(const Vector& point) const
{
	double x = point.x;
	double y = point.y;
	double z = point.z;

	return Vector(
		2.0 * a * x + d * y + f * z,
		2.0 * b * y + d * x + e * z,
		2.0 * c * z + e * y + f * x);
}
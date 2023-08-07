#include "MeshFitter.h"
#include "ConvexPolygonMesh.h"
#include "AxisAlignedBoundingBox.h"
#include "Ray.h"
#include "AlgebraicSurface.h"
#include "Plane.h"
#if defined MESH_NINJA_DEBUG
#	include "MeshFileFormat.h"
#endif //MESH_NINJA_DEBUG

using namespace MeshNinja;

//----------------------------- MeshFitter::FittableObject -----------------------------

MeshFitter::MeshFitter()
{
}

/*virtual*/ MeshFitter::~MeshFitter()
{
}

bool MeshFitter::GenerateMesh(ConvexPolygonMesh& mesh, const AxisAlignedBoundingBox& aabb, double approximateEdgeLength, const FittableObject& object)
{
	mesh.Clear();

	double alpha = 0.0;
	Ray ray = object.CalcInitialContactRay();
	if (!object.RayCast(ray, alpha))
		return false;

	Vector3 surfacePointA = ray.Lerp(alpha);
	if (!aabb.ContainsPoint(surfacePointA))
		return false;

	Vector3 normalDirection = object.CalculateSurfaceNormalAt(surfacePointA).Normalized();
	Vector3 tangentDirection;
	if (!tangentDirection.MakeOrthogonalTo(normalDirection))
		return false;

	tangentDirection.Normalize();
	ray = Ray(surfacePointA + tangentDirection * approximateEdgeLength, normalDirection);
	if (!object.RayCast(ray, alpha))
		return false;

	Vector3 surfacePointB = ray.Lerp(alpha);

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

		Vector3 normalVector = object.CalculateSurfaceNormalAt((*mesh.vertexArray)[edge.i]);
		Vector3 edgeVector = (*mesh.vertexArray)[edge.j] - (*mesh.vertexArray)[edge.i];
		Vector3 tangentVector = normalVector.Cross(edgeVector);
		Plane edgePlane((*mesh.vertexArray)[edge.i], tangentVector);

		// Merge two edges together first if we can.
		for (iter = edgeQueue.begin(); iter != edgeQueue.end(); iter++)
		{
			const Edge& adjacentEdge = *iter;
			if (adjacentEdge.j == edge.i && adjacentEdge.i != edge.j && edgePlane.WhichSide((*mesh.vertexArray)[adjacentEdge.i]) == Plane::Side::FRONT)
			{
				Vector3 vectorA = (*mesh.vertexArray)[adjacentEdge.i] - (*mesh.vertexArray)[adjacentEdge.j];
				Vector3 vectorB = (*mesh.vertexArray)[edge.j] - (*mesh.vertexArray)[edge.i];
				double angle = vectorA.AngleBetweenThisAnd(vectorB);
				if (angle < MESH_NINJA_PI / 2.0)
				{
					newEdgeArray.push_back(Edge{ adjacentEdge.i, edge.j });
					newFacet.vertexArray->push_back(adjacentEdge.i);
					newFacet.vertexArray->push_back(edge.i);
					newFacet.vertexArray->push_back(edge.j);
					edgeQueue.erase(iter);
					break;
				}
			}
			else if (edge.j == adjacentEdge.i && edge.i != adjacentEdge.j && edgePlane.WhichSide((*mesh.vertexArray)[adjacentEdge.j]) == Plane::Side::FRONT)
			{
				Vector3 vectorA = (*mesh.vertexArray)[adjacentEdge.j] - (*mesh.vertexArray)[adjacentEdge.i];
				Vector3 vectorB = (*mesh.vertexArray)[edge.i] - (*mesh.vertexArray)[edge.j];
				double angle = vectorA.AngleBetweenThisAnd(vectorB);
				if (angle < MESH_NINJA_PI / 2.0)
				{
					newEdgeArray.push_back(Edge{ edge.i, adjacentEdge.j });
					newFacet.vertexArray->push_back(adjacentEdge.j);
					newFacet.vertexArray->push_back(edge.i);
					newFacet.vertexArray->push_back(edge.j);
					edgeQueue.erase(iter);
					break;
				}
			}
		}

		// Okay, we should have enough room to create a new flap.
		if (newFacet.vertexArray->size() == 0)
		{
			double length = approximateEdgeLength * ::sqrt(3.0) / 2.0;
			Vector3 point = (*mesh.vertexArray)[edge.i] + (edgeVector / 2.0) + (tangentVector.Normalized() * length);
			ray = Ray(point, normalVector);
			if (!object.RayCast(ray, alpha))
				return false;

			Vector3 surfacePointC = ray.Lerp(alpha);
			double smallestDistance = 0.0;
			int i = mesh.FindClosestPointTo(surfacePointC, &smallestDistance);
			if (i < 0 || smallestDistance > approximateEdgeLength / 2.0)
			{
				i = mesh.vertexArray->size();
				mesh.vertexArray->push_back(surfacePointC);
			}

			newFacet.vertexArray->push_back(edge.i);
			newFacet.vertexArray->push_back(edge.j);
			newFacet.vertexArray->push_back(i);

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

//----------------------------- MeshFitter::FittableObject -----------------------------

MeshFitter::FittableObject::FittableObject()
{
}

/*virtual*/ MeshFitter::FittableObject::~FittableObject()
{
}

//----------------------------- MeshFitter::FittableAlgebraicSurface -----------------------------

MeshFitter::FittableAlgebraicSurface::FittableAlgebraicSurface(const AlgebraicSurface* surface)
{
	this->surface = surface;
}

/*virtual*/ MeshFitter::FittableAlgebraicSurface::~FittableAlgebraicSurface()
{
}

/*virtual*/ bool MeshFitter::FittableAlgebraicSurface::RayCast(const Ray& ray, double& alpha, double eps /*= MESH_NINJA_EPS*/) const
{
	return ray.CastAgainst(*this->surface, alpha, eps);
}

/*virtual*/ Vector3 MeshFitter::FittableAlgebraicSurface::CalculateSurfaceNormalAt(const Vector3& surfacePoint) const
{
	return this->surface->EvaluateGradient(surfacePoint);
}

/*virtual*/ Ray MeshFitter::FittableAlgebraicSurface::CalcInitialContactRay() const
{
	// TODO: This is just a guess.  Do better.
	return Ray(Vector3(0.0, 0.0, 100.0), Vector3(0.0, 0.0, -1.0));
}

//----------------------------- MeshFitter::FittableMesh -----------------------------

MeshFitter::FittableMesh::FittableMesh(const ConvexPolygonMesh* mesh)
{
	this->mesh = mesh;
}

/*virtual*/ MeshFitter::FittableMesh::~FittableMesh()
{
}

/*virtual*/ bool MeshFitter::FittableMesh::RayCast(const Ray& ray, double& alpha, double eps /*= MESH_NINJA_EPS*/) const
{
	// TODO: We should cast against a bounding-box-tree here instead for efficiency purposes.
	return ray.CastAgainst(*this->mesh, alpha, eps);
}

/*virtual*/ Vector3 MeshFitter::FittableMesh::CalculateSurfaceNormalAt(const Vector3& surfacePoint) const
{
	// TODO: Write this.  Using bounding-box-tree to be more efficient.
	return Vector3(0.0, 0.0, 0.0);
}

/*virtual*/ Ray MeshFitter::FittableMesh::CalcInitialContactRay() const
{
	// TODO: Write this.  Just shoot a ray at the biggest triangle or something like that?
	return Ray(Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0));
}
#pragma once

#include "Common.h"
#include "Math/Vector3.h"

namespace MeshNinja
{
	class Vector3;
	class Plane;
	class ConvexPolygon;
	class LineSegment;
	class AlgebraicSurface;
	class ConvexPolygonMesh;
	class AxisAlignedBoundingBox;

	class MESH_NINJA_API Ray
	{
	public:
		Ray();
		Ray(const Ray& ray);
		Ray(const Vector3& origin, const Vector3& direction);
		virtual ~Ray();

		bool CastAgainst(const Plane& plane, double& alpha, double eps = MESH_NINJA_EPS) const;
		bool CastAgainst(const ConvexPolygon& polygon, double& alpha, double eps = MESH_NINJA_EPS) const;
		bool CastAgainst(const LineSegment& lineSegment, double& alpha, double eps = MESH_NINJA_EPS) const;
		bool CastAgainst(const AlgebraicSurface& algebraicSurface, double& alpha, double eps = MESH_NINJA_EPS, int maxIterations = 100, double initialStepSize = 1.0, bool forwardOrBackward = false) const;
		bool CastAgainst(const ConvexPolygonMesh& mesh, double& alpha, double eps = MESH_NINJA_EPS) const;
		bool CastAgainst(const AxisAlignedBoundingBox& aabb, double& alpha, double eps = MESH_NINJA_EPS) const;
		Vector3 Lerp(double alpha) const;
		double LerpInverse(const Vector3& point) const;

		static bool Intersect(const Ray& rayA, const Ray& rayB, double& alpha, double& beta, double eps = MESH_NINJA_EPS);

		Vector3 origin;
		Vector3 direction;
	};
}
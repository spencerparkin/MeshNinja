#pragma once

#include "Common.h"
#include "Vector.h"

namespace MeshNinja
{
	class Vector;
	class Plane;
	class ConvexPolygon;
	class LineSegment;
	class AlgebraicSurface;

	class MESH_NINJA_API Ray
	{
	public:
		Ray();
		Ray(const Ray& ray);
		Ray(const Vector& origin, const Vector& direction);
		virtual ~Ray();

		bool CastAgainst(const Plane& plane, double& alpha, double eps = MESH_NINJA_EPS) const;
		bool CastAgainst(const ConvexPolygon& polygon, double& alpha, double eps = MESH_NINJA_EPS) const;
		bool CastAgainst(const LineSegment& lineSegment, double& alpha, double eps = MESH_NINJA_EPS) const;
		bool CastAgainst(const AlgebraicSurface& algebraicSurface, double& alpha, double eps = MESH_NINJA_EPS, int maxIterations = 100, double initialStepSize = 1.0, bool forwardOrBackward = false) const;
		Vector Lerp(double alpha) const;
		double LerpInverse(const Vector& point) const;

		static bool Intersect(const Ray& rayA, const Ray& rayB, double& alpha, double& beta, double eps = MESH_NINJA_EPS);

		Vector origin;
		Vector direction;
	};
}
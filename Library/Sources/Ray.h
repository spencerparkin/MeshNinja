#pragma once

#include "Common.h"
#include "Vector.h"

namespace MeshNinja
{
	class Vector;
	class Plane;
	class ConvexPolygon;
	class LineSegment;

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
		Vector Lerp(double alpha) const;

		Vector origin;
		Vector direction;
	};
}
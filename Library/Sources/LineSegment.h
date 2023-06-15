#pragma once

#include "Vector.h"

namespace MeshNinja
{
	class MESH_NINJA_API LineSegment
	{
	public:
		LineSegment();
		LineSegment(const LineSegment& lineSegment);
		LineSegment(const Vector& vertexA, const Vector& vertexB);
		virtual ~LineSegment();

		double DistanceToPoint(const Vector& point) const;
		bool ContainsPoint(const Vector& point, double eps = MESH_NINJA_EPS) const;

		Vector vertexA, vertexB;
	};
}
#pragma once

#include "Math/Vector3.h"

namespace MeshNinja
{
	class MESH_NINJA_API LineSegment
	{
	public:
		LineSegment();
		LineSegment(const LineSegment& lineSegment);
		LineSegment(const Vector3& vertexA, const Vector3& vertexB);
		virtual ~LineSegment();

		double DistanceToPoint(const Vector3& point) const;
		bool ContainsPoint(const Vector3& point, double eps = MESH_NINJA_EPS) const;
		bool IsEndPoint(const Vector3& point, double eps = MESH_NINJA_EPS) const;
		bool IsInteriorPoint(const Vector3& point, double eps = MESH_NINJA_EPS) const;
		bool IsEqualTo(const LineSegment& lineSegment, double eps = MESH_NINJA_EPS) const;
		Vector3 CalcMidpoint() const;

		Vector3 vertexA, vertexB;
	};
}
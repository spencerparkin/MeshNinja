#pragma once

#include "Common.h"
#include "Math/Vector3.h"

namespace MeshNinja
{
	class Vector3;
	class Line;

	class MESH_NINJA_API Plane
	{
	public:
		Plane();
		Plane(const Plane& plane);
		Plane(const Vector3& point, const Vector3& normal);
		virtual ~Plane();

		double SignedDistanceToPoint(const Vector3& point) const;
		Vector3 CalcCenter() const;
		bool IsEqualTo(const Plane& plane, double eps = MESH_NINJA_EPS) const;

		enum class Side
		{
			BACK,
			FRONT,
			NEITHER
		};

		Side WhichSide(const Vector3& point, double eps = MESH_NINJA_EPS) const;

		bool AllPointsOnSide(const std::vector<Vector3>& pointArray, Side side) const;
		bool AllPointsNotOnSide(const std::vector<Vector3>& pointArray, Side side) const;

		Vector3 normal;
		double D;
	};
}
#pragma once

#include "Common.h"
#include "Math/Vector.h"

namespace MeshNinja
{
	class Vector;
	class Line;

	class MESH_NINJA_API Plane
	{
	public:
		Plane();
		Plane(const Plane& plane);
		Plane(const Vector& point, const Vector& normal);
		virtual ~Plane();

		double SignedDistanceToPoint(const Vector& point) const;
		Vector CalcCenter() const;
		bool IsEqualTo(const Plane& plane, double eps = MESH_NINJA_EPS) const;

		enum class Side
		{
			BACK,
			FRONT,
			NEITHER
		};

		Side WhichSide(const Vector& point, double eps = MESH_NINJA_EPS) const;

		bool AllPointsOnSide(const std::vector<Vector>& pointArray, Side side) const;
		bool AllPointsNotOnSide(const std::vector<Vector>& pointArray, Side side) const;

		Vector normal;
		double D;
	};
}
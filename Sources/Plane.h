#pragma once

#include "Common.h"
#include "Vector.h"

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

		Vector normal;
		double signedDistanceToOrigin;
	};
}
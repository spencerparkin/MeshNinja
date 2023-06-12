#pragma once

#include "Common.h"
#include "Vector.h"

namespace MeshNinja
{
	class Vector;
	class Plane;

	class MESH_NINJA_API Ray
	{
	public:
		Ray();
		Ray(const Ray& ray);
		Ray(const Vector& origin, const Vector& direction);
		virtual ~Ray();

		bool CastAgainst(const Plane& plane, double& alpha) const;
		Vector Lerp(double alpha) const;

		Vector origin;
		Vector direction;
	};
}
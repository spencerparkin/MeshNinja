#pragma once

#include "Common.h"
#include "Vector.h"

namespace MeshNinja
{
	class Vector;
	class Plane;

	class MESH_NINJA_API Line
	{
	public:
		Line();
		Line(const Line& line);
		Line(const Vector& point, const Vector& normal);
		virtual ~Line();

		double DistanceToPoint(const Vector& point) const;
		Vector Lerp(double alpha) const;
		bool Intersect(const Plane& planeA, const Plane& planeB) const;

		Vector center;
		Vector normal;
	};
}
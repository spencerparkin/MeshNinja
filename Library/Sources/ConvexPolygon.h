#pragma once

#include "Common.h"
#include "Vector.h"

namespace MeshNinja
{
	// We assume here that the list of points are coplanar and form a
	// convex polygon.  If this is not the case, then the result of any
	// algorithm is left undefined.  Polygons of 0, 1, or 2 vertices
	// are allowed.  Doing so, the intersection of any two polygons can
	// be said to be a polygon.
	class MESH_NINJA_API ConvexPolygon
	{
	public:
		ConvexPolygon();
		ConvexPolygon(const ConvexPolygon& polygon);
		virtual ~ConvexPolygon();

		void Clear();
		void Compress(double eps = MESH_NINJA_EPS);
		bool IsConvex(double eps = MESH_NINJA_EPS) const;
		bool IsConcave(double eps = MESH_NINJA_EPS) const;
		bool CalcPlane(Plane& plane, double eps = MESH_NINJA_EPS) const;
		Vector CalcCenter() const;
		bool Intersect(const ConvexPolygon& polygonA, const ConvexPolygon& polygonB, double eps = MESH_NINJA_EPS);
		bool IntersectWithLineSegment(const Vector& pointA, const Vector& pointB, Vector& intersectionPoint, double eps = MESH_NINJA_EPS) const;
		bool ContainsPoint(const Vector& point, bool* isInteriorPoint = nullptr, double eps = MESH_NINJA_EPS) const;
		bool ContainsPointOnBoundary(const Vector& point, double eps = MESH_NINJA_EPS) const;
		bool SplitAgainst(const Plane& cuttingPlane, ConvexPolygon& polygonA, ConvexPolygon& polygonB, double eps = MESH_NINJA_EPS) const;
		void MakeReverseOf(const ConvexPolygon& polygon);

		std::vector<Vector>* vertexArray;
	};
}
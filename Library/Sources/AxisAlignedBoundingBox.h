#pragma once

#include "Vector.h"

namespace MeshNinja
{
	class MESH_NINJA_API AxisAlignedBoundingBox
	{
	public:
		AxisAlignedBoundingBox();
		AxisAlignedBoundingBox(const Vector& point);
		AxisAlignedBoundingBox(const Vector& min, const Vector& max);
		AxisAlignedBoundingBox(const AxisAlignedBoundingBox& aabb);
		virtual ~AxisAlignedBoundingBox();

		bool IsValid() const;

		double Width() const;
		double Height() const;
		double Depth() const;
		double Volume() const;
		Vector Center() const;

		bool ContainsPoint(const Vector& point, double eps = MESH_NINJA_EPS) const;
		bool ContainsInteriorPoint(const Vector& point, double eps = MESH_NINJA_EPS) const;
		bool ContainsPointOnBoundary(const Vector& point, double eps = MESH_NINJA_EPS) const;

		void ExpandToIncludePoint(const Vector& point);

		bool Intersect(const AxisAlignedBoundingBox& aabbA, const AxisAlignedBoundingBox& aabbB);
		bool Merge(const AxisAlignedBoundingBox& aabbA, const AxisAlignedBoundingBox& aabbB);
		bool OverlapsWith(const AxisAlignedBoundingBox& aabb) const;

		void SplitReasonably(AxisAlignedBoundingBox& aabbA, AxisAlignedBoundingBox& aabbB) const;

		Vector min, max;
	};
}
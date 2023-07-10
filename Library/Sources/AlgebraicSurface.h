#pragma once

#include "Common.h"
#include "Vector.h"

namespace MeshNinja
{
	class ConvexPolygonMesh;
	class AxisAlignedBoundingBox;
	class Ray;

	class MESH_NINJA_API AlgebraicSurface
	{
	public:
		AlgebraicSurface();
		virtual ~AlgebraicSurface();

		virtual double Evaluate(const Vector& point) const = 0;
		virtual Vector EvaluateGradient(const Vector& point) const;
		virtual double EvaluateDirectionalDerivative(const Vector& point, const Vector& unitDirection) const;
		virtual double ApproximateDirectionalDerivative(const Vector& point, const Vector& unitDirection, double delta) const;

		bool GenerateMesh(ConvexPolygonMesh& mesh, const Ray& initialContactRay, const AxisAlignedBoundingBox& aabb, double approximateEdgeLength) const;
	};

	class MESH_NINJA_API QuadraticSurface : public AlgebraicSurface
	{
	public:
		QuadraticSurface();
		virtual ~QuadraticSurface();

		virtual double Evaluate(const Vector& point) const override;
		virtual Vector EvaluateGradient(const Vector& point) const override;

		double a, b, c, d, e, f, g, h, i;
	};
}
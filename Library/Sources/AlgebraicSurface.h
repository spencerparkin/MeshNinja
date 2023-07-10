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

		void MakeEllipsoid(double A, double B, double C);
		void MakeEllipticCone(double A, double B, double C);
		void MakeEllipticCylinder(double A, double B);
		void MakeEllipticParaboloid(double A, double B);
		void MakeHyperbolicCylinder(double A, double B);
		void MakeHyperbolicParaboloid(double A, double B);
		void MakeHyperboloidOfOneSheet(double A, double B, double C);
		void MakeHyperboloidOfTwoSheets(double A, double B, double C);
		void MakeIntersectingPlanes(double A, double B);
		void MakeParabolicSylinder(double R);
		void MakeParallelPlanes(double A);

		double a, b, c, d, e, f, g, h, i, j;
	};
}
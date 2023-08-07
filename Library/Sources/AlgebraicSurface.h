#pragma once

#include "Common.h"
#include "Math/Vector3.h"

namespace MeshNinja
{
	class MESH_NINJA_API AlgebraicSurface
	{
	public:
		AlgebraicSurface();
		virtual ~AlgebraicSurface();

		virtual double Evaluate(const Vector3& point) const = 0;
		virtual Vector3 EvaluateGradient(const Vector3& point) const;
		virtual double EvaluateDirectionalDerivative(const Vector3& point, const Vector3& unitDirection) const;
		virtual double ApproximateDirectionalDerivative(const Vector3& point, const Vector3& unitDirection, double delta) const;
	};

	class MESH_NINJA_API QuadraticSurface : public AlgebraicSurface
	{
	public:
		QuadraticSurface();
		virtual ~QuadraticSurface();

		virtual double Evaluate(const Vector3& point) const override;
		virtual Vector3 EvaluateGradient(const Vector3& point) const override;

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
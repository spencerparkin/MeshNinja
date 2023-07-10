#pragma once

#include "Common.h"
#include "Vector.h"

namespace MeshNinja
{
	// TODO: Write an algorithm that can generate a mesh that approximates a given algebraic surface.
	class MESH_NINJA_API AlgebraicSurface
	{
	public:
		AlgebraicSurface();
		virtual ~AlgebraicSurface();

		virtual double Evaluate(const Vector& point) const = 0;
		virtual Vector EvaluateGradient(const Vector& point) const;
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
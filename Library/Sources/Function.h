#pragma once

#include "Common.h"

namespace MeshNinja
{
	class MESH_NINJA_API Function
	{
	public:
		Function();
		virtual ~Function();

		virtual double Evaluate(double x) const = 0;
		virtual double EvaluateDerivative(double x) const;

		bool FindRoot(double initialX, double& finalX, int maxIterations = 30, double eps = MESH_NINJA_EPS) const;
	};

	class MESH_NINJA_API Derivative : public Function
	{
	public:
		Derivative(const Function* function);
		virtual ~Derivative();

		virtual double Evaluate(double x) const override;

		const Function* function;
	};
}
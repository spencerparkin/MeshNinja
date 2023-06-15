#include "Function.h"

using namespace MeshNinja;

//----------------------------------- Function -----------------------------------

Function::Function()
{
}

/*virtual*/ Function::~Function()
{
}

/*virtual*/ double Function::EvaluateDerivative(double x) const
{
	double delta = 1e-3;
	return (this->Evaluate(x + delta) - this->Evaluate(x - delta)) / (2.0 * delta);
}

bool Function::FindRoot(double initialX, double& finalX, int maxIterations /*= 30*/, double eps /*= MESH_NINJA_EPS*/) const
{
	double x = initialX;
	double y = 0.0;

	for (int i = 0; i < maxIterations; i++)
	{
		y = this->Evaluate(x);

		if (::fabs(y) <= eps)
		{
			finalX = x;
			return true;
		}

		x -= this->Evaluate(x) / this->EvaluateDerivative(x);
	}

	return false;
}

//----------------------------------- Derivative -----------------------------------

Derivative::Derivative(const Function* function)
{
	this->function = function;
}

/*virtual*/ Derivative::~Derivative()
{
}

/*virtual*/ double Derivative::Evaluate(double x) const
{
	return function->EvaluateDerivative(x);
}
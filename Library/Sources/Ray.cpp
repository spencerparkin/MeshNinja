#include "Ray.h"
#include "Plane.h"
#include "ConvexPolygon.h"
#include "LineSegment.h"
#include "AlgebraicSurface.h"

using namespace MeshNinja;

Ray::Ray()
{
}

Ray::Ray(const Ray& ray)
{
	this->origin = ray.origin;
	this->direction = ray.direction;
}

Ray::Ray(const Vector& origin, const Vector& direction)
{
	this->origin = origin;
	this->direction = direction;
}

/*virtual*/ Ray::~Ray()
{
}

bool Ray::CastAgainst(const Plane& plane, double& alpha, double eps /* = MESH_NINJA_EPS*/) const
{
	if (plane.WhichSide(this->origin, eps) == Plane::Side::NEITHER)
	{
		alpha = 0.0;
		return true;
	}

	double dotDenominator = this->direction.Dot(plane.normal);
	if (fabs(dotDenominator) < eps)
		return false;

	double dotNumerator = (plane.CalcCenter() - this->origin).Dot(plane.normal);
	alpha = dotNumerator / dotDenominator;
	if (::isnan(alpha) || ::isinf(alpha))
		return false;

	return alpha >= 0.0;
}

bool Ray::CastAgainst(const LineSegment& lineSegment, double& alpha, double eps /*= MESH_NINJA_EPS*/) const
{
	Ray ray(lineSegment.vertexA, lineSegment.vertexB - lineSegment.vertexA);

	double beta = 0.0;
	if (!Ray::Intersect(*this, ray, alpha, beta, eps))
		return false;

	Vector point = this->Lerp(alpha);
	return lineSegment.ContainsPoint(point, eps);
}

bool Ray::CastAgainst(const ConvexPolygon& polygon, double& alpha, double eps /*= MESH_NINJA_EPS*/) const
{
	if (polygon.ContainsPoint(this->origin, nullptr, eps))
	{
		alpha = 0.0;
		return true;
	}

	Plane plane;
	polygon.CalcPlane(plane, eps);
	if (this->direction.Dot(plane.normal) < eps && plane.WhichSide(this->origin) == Plane::Side::NEITHER)
	{
		double smallestAlpha = DBL_MAX;
		for (int i = 0; i < (signed)polygon.vertexArray->size(); i++)
		{
			int j = (i + 1) % polygon.vertexArray->size();
			const Vector& vertexA = (*polygon.vertexArray)[i];
			const Vector& vertexB = (*polygon.vertexArray)[j];
			LineSegment line(vertexA, vertexB);
			if (this->CastAgainst(line, alpha, eps) && alpha < smallestAlpha)
				smallestAlpha = alpha;
		}

		if (smallestAlpha != DBL_MAX)
		{
			alpha = smallestAlpha;
			return true;
		}
	}
	
	if (!this->CastAgainst(plane, alpha))
		return false;

	Vector hitPoint = this->Lerp(alpha);
	return polygon.ContainsPoint(hitPoint, nullptr, eps);
}

bool Ray::CastAgainst(const AlgebraicSurface& algebraicSurface, double& alpha,
								double eps /*= MESH_NINJA_EPS*/,
								int maxIterations /*= 100*/,
								double initialStepSize /*= 1.0*/,
								bool forwardOrBackward /*= false*/) const
{
	Vector unitDirection(this->direction);
	if (!unitDirection.Normalize())
		return false;

	int iterationCount = 0;
	Vector point(this->origin);
	alpha = 0.0;
	double value = algebraicSurface.Evaluate(point);
	double stepSize = initialStepSize;
	while (stepSize >= eps / 2.0 && iterationCount++ < maxIterations)
	{
		if (::fabs(value) < eps)
		{
			alpha = this->LerpInverse(point);
			return forwardOrBackward || alpha >= 0.0;
		}

		double derivativeValue = algebraicSurface.EvaluateDirectionalDerivative(point, unitDirection);
		double delta = -MESH_NINJA_SIGN(derivativeValue) * stepSize;

		point += unitDirection * delta;

		double nextValue = algebraicSurface.Evaluate(point);
		if (MESH_NINJA_SIGN(value) != MESH_NINJA_SIGN(nextValue))
			stepSize /= 2.0;

		value = nextValue;
	}

	return false;
}

Vector Ray::Lerp(double alpha) const
{
	return this->origin + this->direction * alpha;
}

double Ray::LerpInverse(const Vector& point) const
{
	return (point - this->origin).Dot(this->direction) / this->direction.Dot(this->direction);
}

/*static*/ bool Ray::Intersect(const Ray& rayA, const Ray& rayB, double& alpha, double& beta, double eps /*= MESH_NINJA_EPS*/)
{
	Vector normal = rayA.direction.Normalized().Cross(rayB.direction.Normalized());
	double length = normal.Length();
	if (length < eps || !normal.Normalize())
	{
		// The rays are parallel.
		if (rayA.origin.IsEqualTo(rayB.origin, eps))
		{
			// The rays are the same ray.
			alpha = 0.0;
			beta = 0.0;
			return true;
		}

		return false;
	}
	
	Plane plane(rayA.origin, normal);
	if (plane.WhichSide(rayB.origin, eps) != Plane::Side::NEITHER)
	{
		// The rays are not in the same plane.
		return false;
	}

	Plane planeA(rayA.origin, rayA.direction.Cross(normal));
	if (!rayB.CastAgainst(planeA, beta, eps))
		return false;

	Plane planeB(rayB.origin, rayB.direction.Cross(normal));
	if (!rayA.CastAgainst(planeB, alpha, eps))
		return false;

	// Perform sanity check.
	//assert(rayA.Lerp(alpha).IsEqualTo(rayB.Lerp(beta), eps));

	return true;
}
#include "Ray.h"
#include "Plane.h"
#include "ConvexPolygon.h"
#include "LineSegment.h"

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
	double dotDenominator = this->direction.Dot(plane.normal);
	if (dotDenominator < eps)
		return false;

	double dotNumerator = (plane.CalcCenter() - this->origin).Dot(plane.normal);
	alpha = dotNumerator / dotDenominator;
	if (::isnan(alpha) || ::isinf(alpha))
		return false;

	return true;
}

// There is a way to solve this analytically using matrix algebra, but I'm going to be lazy here.
bool Ray::CastAgainst(const LineSegment& lineSegment, double& alpha, double eps /*= MESH_NINJA_EPS*/) const
{
	alpha = 0.0;
	double delta = this->direction.Length();
	double smallestDistance = DBL_MAX;
	while (smallestDistance >= eps && delta >= eps)
	{
		Vector point = this->Lerp(alpha);
		double distance = lineSegment.DistanceToPoint(point);
		if (distance < smallestDistance)
			smallestDistance = distance;
		else
		{
			alpha -= delta;
			delta /= -2.0;
		}
		alpha += delta;
	}

	return smallestDistance < eps;
}

bool Ray::CastAgainst(const ConvexPolygon& polygon, double& alpha, double eps /*= MESH_NINJA_EPS*/) const
{
	if (polygon.ContainsPoint(this->origin, eps))
	{
		alpha = 0.0;
		return true;
	}

	Plane plane = polygon.CalcPlane();
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
	return polygon.ContainsPoint(hitPoint, eps);
}

Vector Ray::Lerp(double alpha) const
{
	return this->origin + this->direction * alpha;
}
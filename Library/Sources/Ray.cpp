#include "Ray.h"
#include "Plane.h"

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

bool Ray::CastAgainst(const Plane& plane, double& alpha) const
{
	double dotDenominator = this->direction.Dot(plane.normal);
	if (dotDenominator == 0.0)
		return false;

	double dotNumerator = (plane.CalcCenter() - this->origin).Dot(plane.normal);
	alpha = dotNumerator / dotDenominator;
	if (::isnan(alpha) || ::isinf(alpha))
		return false;

	return true;
}

Vector Ray::Lerp(double alpha) const
{
	return this->origin + this->direction * alpha;
}
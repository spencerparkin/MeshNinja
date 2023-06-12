#include "Plane.h"

using namespace MeshNinja;

Plane::Plane()
{
	this->normal = Vector(0.0, 0.0, 0.0);
	this->signedDistanceToOrigin = 0.0;
}

Plane::Plane(const Plane& plane)
{
	this->normal = plane.normal;
	this->signedDistanceToOrigin = plane.signedDistanceToOrigin;
}

Plane::Plane(const Vector& point, const Vector& normal)
{
	this->normal = normal;
	this->normal.Normalize();
	this->signedDistanceToOrigin = point.Dot(this->normal);
}

/*virtual*/ Plane::~Plane()
{
}

double Plane::SignedDistanceToPoint(const Vector& point) const
{
	return point.Dot(this->normal) - this->signedDistanceToOrigin;
}
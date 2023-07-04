#include "Plane.h"

using namespace MeshNinja;

Plane::Plane()
{
	this->normal = Vector(0.0, 0.0, 0.0);
	this->D = 0.0;
}

Plane::Plane(const Plane& plane)
{
	this->normal = plane.normal;
	this->D = plane.D;
}

Plane::Plane(const Vector& point, const Vector& normal)
{
	this->normal = normal;
	this->normal.Normalize();
	this->D = point.Dot(this->normal);
}

/*virtual*/ Plane::~Plane()
{
}

double Plane::SignedDistanceToPoint(const Vector& point) const
{
	return point.Dot(this->normal) - this->D;
}

bool Plane::IsEqualTo(const Plane& plane, double eps /*= MESH_NINJA_EPS*/) const
{
	return (this->D - plane.D) < eps && this->normal.IsEqualTo(plane.normal, eps);
}

Vector Plane::CalcCenter() const
{
	return this->normal * this->D;
}

Plane::Side Plane::WhichSide(const Vector& point, double eps /*= MESH_NINJA_EPS*/) const
{
	double signedDistance = this->SignedDistanceToPoint(point);
	
	if (signedDistance < -eps / 2.0)
		return Side::BACK;

	if (signedDistance > eps / 2.0)
		return Side::FRONT;

	return Side::NEITHER;
}

bool Plane::AllPointsOnSide(const std::vector<Vector>& pointArray, Side side) const
{
	for (const Vector& point : pointArray)
		if (this->WhichSide(point) != side)
			return false;

	return true;
}

bool Plane::AllPointsNotOnSide(const std::vector<Vector>& pointArray, Side side) const
{
	for (const Vector& point : pointArray)
		if (this->WhichSide(point) == side)
			return false;

	return true;
}
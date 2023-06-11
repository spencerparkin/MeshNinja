#include "Line.h"

using namespace MeshNinja;

Line::Line()
{
}

Line::Line(const Line& line)
{
	this->center = line.center;
	this->normal = line.normal;
}

Line::Line(const Vector& point, const Vector& normal)
{
	this->normal = normal;
	this->normal.Normalize();
	this->center = point;
	this->center.RejectFrom(this->normal);
}

/*virtual*/ Line::~Line()
{
}

double Line::DistanceToPoint(const Vector& point) const
{
	return (point - this->center - this->normal * point.Dot(this->normal)).Length();
}

Vector Line::Lerp(double alpha) const
{
	return this->center + this->normal * alpha;
}

bool Line::Intersect(const Plane& planeA, const Plane& planeB) const
{
	return false;
}
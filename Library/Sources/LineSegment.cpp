#include "LineSegment.h"

using namespace MeshNinja;

LineSegment::LineSegment()
{
}

LineSegment::LineSegment(const LineSegment& lineSegment)
{
	this->vertexA = lineSegment.vertexA;
	this->vertexB = lineSegment.vertexB;
}

LineSegment::LineSegment(const Vector& vertexA, const Vector& vertexB)
{
	this->vertexA = vertexA;
	this->vertexB = vertexB;
}

/*virtual*/ LineSegment::~LineSegment()
{
}

double LineSegment::DistanceToPoint(const Vector& point) const
{
	Vector axis = this->vertexB - this->vertexA;
	double length = axis.Length();
	if (length == 0.0)
		return (point - this->vertexA).Length();
	
	axis /= length;

	double dot = (point - this->vertexA).Dot(axis);
	if (dot <= 0.0)
		return (point - this->vertexA).Length();
	else if (dot >= length)
		return (point - this->vertexB).Length();

	double hypotenuse = (point - this->vertexA).Length();
	double distance = ::sqrt(hypotenuse * hypotenuse - dot * dot);
	return distance;
}

bool LineSegment::ContainsPoint(const Vector& point, double eps /*= MESH_NINJA_EPS*/) const
{
	return this->DistanceToPoint(point) < eps;
}
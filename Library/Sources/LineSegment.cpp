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

	double hypotenuseSquared = (point - this->vertexA).Dot(point - this->vertexA);
	double distance = ::sqrt(::fabs(hypotenuseSquared - dot * dot));
	return distance;
}

bool LineSegment::ContainsPoint(const Vector& point, double eps /*= MESH_NINJA_EPS*/) const
{
	return this->DistanceToPoint(point) < eps;
}

bool LineSegment::IsEndPoint(const Vector& point, double eps /*= MESH_NINJA_EPS*/) const
{
	return point.IsEqualTo(this->vertexA, eps) || point.IsEqualTo(this->vertexB, eps);
}

bool LineSegment::IsInteriorPoint(const Vector& point, double eps /*= MESH_NINJA_EPS*/) const
{
	return !this->IsEndPoint(point, eps) && this->ContainsPoint(point, eps);
}

bool LineSegment::IsEqualTo(const LineSegment& lineSegment, double eps /*= MESH_NINJA_EPS*/) const
{
	if (this->vertexA.IsEqualTo(lineSegment.vertexA, eps) && this->vertexB.IsEqualTo(lineSegment.vertexB, eps))
		return true;

	if (this->vertexA.IsEqualTo(lineSegment.vertexB, eps) && this->vertexB.IsEqualTo(lineSegment.vertexA, eps))
		return true;

	return false;
}

Vector LineSegment::CalcMidpoint() const
{
	return (this->vertexA + this->vertexB) / 2.0;
}
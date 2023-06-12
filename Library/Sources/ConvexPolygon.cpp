#include "ConvexPolygon.h"
#include "Plane.h"
#include "Ray.h"

using namespace MeshNinja;

ConvexPolygon::ConvexPolygon()
{
	this->vertexArray = new std::vector<Vector>();
}

ConvexPolygon::ConvexPolygon(const ConvexPolygon& polygon)
{
	this->vertexArray = new std::vector<Vector>();
	*this->vertexArray = *polygon.vertexArray;
}

/*virtual*/ ConvexPolygon::~ConvexPolygon()
{
	delete this->vertexArray;
}

void ConvexPolygon::Clear()
{
	this->vertexArray->clear();
}

void ConvexPolygon::Compress(double eps /*= MESH_NINJA_EPS*/)
{
	bool makeAnotherPass = true;
	while (makeAnotherPass)
	{
		makeAnotherPass = false;

		for (int i = 0; i < (signed)this->vertexArray->size(); i++)
		{
			const Vector& vertexA = (*this->vertexArray)[i];
			const Vector& vertexB = (*this->vertexArray)[(i + 1) % this->vertexArray->size()];
			const Vector& vertexC = (*this->vertexArray)[(i + 2) % this->vertexArray->size()];

			double triangleArea = (vertexB - vertexA).Cross(vertexC - vertexA).Length() / 2.0;

			if (triangleArea < eps)
			{
				this->vertexArray->erase(this->vertexArray->begin() + ((i + 1) % this->vertexArray->size()));
				makeAnotherPass = true;
				break;
			}
		}
	}
}

Plane ConvexPolygon::CalcPlane() const
{
	Plane plane;

	if (this->vertexArray->size() >= 3)
	{
		const Vector& vertexA = (*this->vertexArray)[0];
		const Vector& vertexB = (*this->vertexArray)[1];
		const Vector& vertexC = (*this->vertexArray)[2];

		plane = Plane(vertexA, (vertexB - vertexA).Cross(vertexC - vertexA));
	}

	return plane;
}

bool ConvexPolygon::IntersectWithLineSegment(const Vector& pointA, const Vector& pointB, Vector& intersectionPoint, double eps /*= MESH_NINJA_EPS*/) const
{
	Plane plane = this->CalcPlane();
	double alpha = 0.0;
	Ray ray(pointA, pointB - pointA);
	if (!ray.CastAgainst(plane, alpha))
		return false;

	if (-eps < alpha && alpha < 1.0 + eps)
	{
		intersectionPoint = ray.Lerp(alpha);
		return true;
	}

	return false;
}

bool ConvexPolygon::ContainsPoint(const Vector& point, double eps /*= MESH_NINJA_EPS*/) const
{
	Plane plane = this->CalcPlane();
	if (plane.WhichSide(point, eps) != Plane::Side::NEITHER)
		return false;

	for (int i = 0; i < (signed)this->vertexArray->size(); i++)
	{
		int j = (i + 1) % this->vertexArray->size();
		const Vector& vertexA = (*this->vertexArray)[i];
		const Vector& vertexB = (*this->vertexArray)[j];
		double dot = (vertexA - point).Cross(vertexB - point).Dot(plane.normal);
		if (dot < 0.0)
			return false;
	}

	return true;
}

bool ConvexPolygon::Intersect(const ConvexPolygon& polygonA, const ConvexPolygon& polygonB, double eps /*= MESH_NINJA_EPS*/)
{
	//...

	return false;
}
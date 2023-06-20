#include "ConvexPolygon.h"
#include "Plane.h"
#include "Ray.h"
#include "LineSegment.h"

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
	// Is the given point in the same plane of the polygon?
	Plane plane = this->CalcPlane();
	if (plane.WhichSide(point, eps) != Plane::Side::NEITHER)
		return false;

	// Is the given point on the boundary of the polygon?
	for (int i = 0; i < (signed)this->vertexArray->size(); i++)
	{
		int j = (i + 1) % this->vertexArray->size();
		const Vector& vertexA = (*this->vertexArray)[i];
		const Vector& vertexB = (*this->vertexArray)[j];
		LineSegment line(vertexA, vertexB);
		if (line.ContainsPoint(point, eps))
			return true;
	}

	// Is the given point in the interior of the polygon?
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
	this->Clear();

	const ConvexPolygon* polygonArray[2] = { &polygonA, &polygonB };
	const ConvexPolygon* chosenPolygon = nullptr;
	int chosenVertex = -1;

	for (int i = 0; i < 2 && !chosenPolygon; i++)
	{
		const ConvexPolygon* polygon = polygonArray[i];
		const ConvexPolygon* otherPolygon = polygonArray[1 - i];

		for (int j = 0; j < (signed)polygon->vertexArray->size(); j++)
		{
			const Vector& vertex = (*polygon->vertexArray)[j];
			if (!otherPolygon->ContainsPoint(vertex))
			{
				chosenPolygon = polygon;
				chosenVertex = j;
				break;
			}
		}
	}

	if (!chosenPolygon)
	{
		// Polygons A and B must be the same polygon.
		*this = polygonA;
		return true;
	}

	Plane planeA, planeB;

	planeA = polygonA.CalcPlane();
	planeB = polygonB.CalcPlane();

	Vector centerA = planeA.CalcCenter();
	Vector centerB = planeB.CalcCenter();

	const ConvexPolygon* otherPolygon = (chosenPolygon == &polygonA) ? &polygonB : &polygonA;

	if ((planeB.normal - planeA.normal).Length() < eps && (centerB - centerA).Length() < eps)
	{
		//...
	}
	else
	{
		for (int i = 0; i < (signed)chosenPolygon->vertexArray->size(); i++)
		{
			int j = (chosenVertex + i) % chosenPolygon->vertexArray->size();
			int k = (j + 1) % chosenPolygon->vertexArray->size();
			const Vector& vertexA = (*chosenPolygon->vertexArray)[j];
			const Vector& vertexB = (*chosenPolygon->vertexArray)[k];

			Ray ray(vertexA, vertexB - vertexA);
			double alpha = 0.0;
			if (ray.CastAgainst(*otherPolygon, alpha))
			{
				Vector hitPoint = ray.Lerp(alpha);
				this->vertexArray->push_back(hitPoint);
			}
		}
	}

	return this->vertexArray->size() > 0;
}
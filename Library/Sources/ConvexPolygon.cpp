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

			if (vertexA.IsEqualTo(vertexB, eps))
			{
				this->vertexArray->erase(vertexArray->begin() + i);
				makeAnotherPass = true;
				break;
			}
		}
	}

	makeAnotherPass = true;
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

bool ConvexPolygon::ContainsPoint(const Vector& point, bool* isInteriorPoint /*= nullptr*/, double eps /*= MESH_NINJA_EPS*/) const
{
	if (isInteriorPoint)
		*isInteriorPoint = false;

	// Is the given point in the same plane of the polygon?
	Plane plane = this->CalcPlane();
	if (plane.WhichSide(point, eps) != Plane::Side::NEITHER)
		return false;

	// Is the given point on the boundary of the polygon?
	if (this->ContainsPointOnBoundary(point, eps))
		return true;

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

	if (isInteriorPoint)
		*isInteriorPoint = true;

	return true;
}

bool ConvexPolygon::ContainsPointOnBoundary(const Vector& point, double eps /*= MESH_NINJA_EPS*/) const
{
	for (int i = 0; i < (signed)this->vertexArray->size(); i++)
	{
		int j = (i + 1) % this->vertexArray->size();
		const Vector& vertexA = (*this->vertexArray)[i];
		const Vector& vertexB = (*this->vertexArray)[j];
		LineSegment line(vertexA, vertexB);
		if (line.ContainsPoint(point, eps))
			return true;
	}

	return false;
}

bool ConvexPolygon::Intersect(const ConvexPolygon& polygonA, const ConvexPolygon& polygonB, double eps /*= MESH_NINJA_EPS*/)
{
	this->Clear();

	const ConvexPolygon* polygonArray[2] = { &polygonA, &polygonB };
	bool samePolygon = true;

	for (int i = 0; i < 2 && samePolygon; i++)
	{
		const ConvexPolygon* polygon = polygonArray[i];
		const ConvexPolygon* otherPolygon = polygonArray[1 - i];

		for (int j = 0; j < (signed)polygon->vertexArray->size(); j++)
		{
			const Vector& vertex = (*polygon->vertexArray)[j];
			if (!otherPolygon->ContainsPoint(vertex))
			{
				samePolygon = false;
				break;
			}
		}
	}

	if (samePolygon)
	{
		// Polygons A and B must be the same polygon.
		*this = polygonA;
		return true;
	}

	std::vector<Vector> hitPointArray[2];

	for (int i = 0; i < 2; i++)
	{
		const ConvexPolygon* polygon = polygonArray[i];
		const ConvexPolygon* otherPolygon = polygonArray[1 - i];

		for (int j = 0; j < (signed)polygon->vertexArray->size(); j++)
		{
			int k = (j + 1) % polygon->vertexArray->size();

			const Vector& vertexA = (*polygon->vertexArray)[j];
			const Vector& vertexB = (*polygon->vertexArray)[k];

			Ray ray(vertexA, vertexB - vertexA);
			double alpha = 0.0;
			if (ray.CastAgainst(*otherPolygon, alpha) && alpha <= 1.0)
			{
				Vector hitPoint = ray.Lerp(alpha);
				if (!hitPoint.IsEqualTo(vertexB))
					hitPointArray[i].push_back(hitPoint);
			}
		}
	}

	if (hitPointArray[0].size() <= 2 && hitPointArray[1].size() <= 2)
	{
		std::vector<Vector> combinedHitPointArray;
		for (int i = 0; i < 2; i++)
			for (Vector& hitPoint : hitPointArray[i])
				combinedHitPointArray.push_back(hitPoint);

		for (Vector& hitPoint : combinedHitPointArray)
		{
			bool pointFound = false;
			for (Vector& vertex : *this->vertexArray)
			{
				if ((vertex - hitPoint).Length() < eps)
				{
					pointFound = true;
					break;
				}
			}

			if (!pointFound)
				this->vertexArray->push_back(hitPoint);
		}
	}
	else
	{
		// This is a valid case and can happen if the two polygons are
		// co-planar.  However, it's not a case that I need to handle
		// yet, so here I'm just going to ignore it.  For the record,
		// however, this is where we might return a polygon with three
		// or more vertices.
	}

	return this->vertexArray->size() > 0;
}

bool ConvexPolygon::SplitAgainst(const Plane& cuttingPlane, ConvexPolygon& polygonA, ConvexPolygon& polygonB, double eps /*= MESH_NINJA_EPS*/) const
{
	polygonA.Clear();
	polygonB.Clear();

	for (int i = 0; i < (signed)this->vertexArray->size(); i++)
	{
		int j = (i + 1) % this->vertexArray->size();

		const Vector& vertexA = (*this->vertexArray)[i];
		const Vector& vertexB = (*this->vertexArray)[j];

		Plane::Side vertexASide = cuttingPlane.WhichSide(vertexA, eps);
		Plane::Side vertexBSide = cuttingPlane.WhichSide(vertexB, eps);

		if (vertexASide == Plane::Side::FRONT)
		{
			polygonA.vertexArray->push_back(vertexA);

			if (vertexBSide == Plane::Side::BACK)
			{
				Ray ray(vertexA, vertexB - vertexA);
				double alpha = 0.0;
				ray.CastAgainst(cuttingPlane, alpha, eps);
				Vector hitPoint = ray.Lerp(alpha);
				polygonA.vertexArray->push_back(hitPoint);
				polygonB.vertexArray->push_back(hitPoint);
			}
		}
		else if (vertexASide == Plane::Side::BACK)
		{
			polygonB.vertexArray->push_back(vertexA);

			if (vertexBSide == Plane::Side::FRONT)
			{
				Ray ray(vertexA, vertexB - vertexA);
				double alpha = 0.0;
				ray.CastAgainst(cuttingPlane, alpha, eps);
				Vector hitPoint = ray.Lerp(alpha);
				polygonA.vertexArray->push_back(hitPoint);
				polygonB.vertexArray->push_back(hitPoint);
			}
		}
		else if (vertexASide == Plane::Side::NEITHER)
		{
			polygonA.vertexArray->push_back(vertexA);
			polygonB.vertexArray->push_back(vertexB);
		}
	}

	return polygonA.vertexArray->size() > 0 && polygonB.vertexArray->size() > 0;
}

void ConvexPolygon::MakeReverseOf(const ConvexPolygon& polygon)
{
	this->vertexArray->clear();
	for (int i = (signed)polygon.vertexArray->size() - 1; i >= 0; i--)
		this->vertexArray->push_back((*polygon.vertexArray)[i]);
}
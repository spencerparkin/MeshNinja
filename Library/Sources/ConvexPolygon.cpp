#include "ConvexPolygon.h"
#include "Plane.h"
#include "Ray.h"
#include "LineSegment.h"
#include "AxisAlignedBoundingBox.h"

using namespace MeshNinja;

ConvexPolygon::ConvexPolygon()
{
	this->vertexArray = new std::vector<Vector3>();
}

ConvexPolygon::ConvexPolygon(const ConvexPolygon& polygon)
{
	this->vertexArray = new std::vector<Vector3>();
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

bool ConvexPolygon::CalcBox(AxisAlignedBoundingBox& box) const
{
	if (this->vertexArray->size() == 0)
		return false;

	box.min = (*this->vertexArray)[0];
	box.max = (*this->vertexArray)[0];

	for (const Vector3& vertex : *this->vertexArray)
		box.ExpandToIncludePoint(vertex);

	return true;
}

bool ConvexPolygon::VerticesAreCoplanar(double eps /*= MESH_NINJA_EPS*/) const
{
	Plane plane;
	if (!this->CalcPlane(plane, eps))
		return false;

	for (const Vector3& vertex : *this->vertexArray)
		if (plane.WhichSide(vertex, eps) != Plane::Side::NEITHER)
			return false;

	return true;
}

bool ConvexPolygon::IsConvex(double eps /*= MESH_NINJA_EPS*/) const
{
	Plane plane;
	if (!this->CalcPlane(plane, eps))
		return false;

	for (int i = 0; i < (signed)this->vertexArray->size(); i++)
	{
		int j = (i + 1) % this->vertexArray->size();
		Vector3 edgeVector = (*this->vertexArray)[j] - (*this->vertexArray)[i];
		Vector3 edgeNormal = edgeVector.Cross(plane.normal);
		Plane edgePlane((*this->vertexArray)[i], edgeNormal);

		for (j = 0; j < (signed)this->vertexArray->size(); j++)
		{
			const Vector3& vertex = (*this->vertexArray)[j];
			if (edgePlane.WhichSide(vertex, eps) == Plane::Side::FRONT)
				return false;
		}
	}

	return true;
}

bool ConvexPolygon::IsConcave(double eps /*= MESH_NINJA_EPS*/) const
{
	return !this->IsConvex(eps);
}

void ConvexPolygon::Compress(double eps /*= MESH_NINJA_EPS*/)
{
	bool makeAnotherPass = true;
	while (makeAnotherPass)
	{
		makeAnotherPass = false;

		for (int i = 0; i < (signed)this->vertexArray->size(); i++)
		{
			const Vector3& vertexA = (*this->vertexArray)[i];
			const Vector3& vertexB = (*this->vertexArray)[(i + 1) % this->vertexArray->size()];

			if (vertexA.IsEqualTo(vertexB, eps))
			{
				this->vertexArray->erase(this->vertexArray->begin() + i);
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
			const Vector3& vertexA = (*this->vertexArray)[i];
			const Vector3& vertexB = (*this->vertexArray)[(i + 1) % this->vertexArray->size()];
			const Vector3& vertexC = (*this->vertexArray)[(i + 2) % this->vertexArray->size()];

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

bool ConvexPolygon::CalcPlane(Plane& plane, double eps /*= MESH_NINJA_EPS*/) const
{
	if (this->vertexArray->size() < 3)
		return false;
	
	double largestLength = DBL_MIN;
	Vector3 bestNormal;

	for(int i = 0; i < (signed)vertexArray->size(); i++)
	{
		const Vector3& vertexA = (*this->vertexArray)[i];
		const Vector3& vertexB = (*this->vertexArray)[(i + 1) % vertexArray->size()];
		const Vector3& vertexC = (*this->vertexArray)[(i + 2) % vertexArray->size()];

		Vector3 normal = (vertexC - vertexB).Cross(vertexA - vertexB);
		double length = normal.Length();
		if (length > largestLength)
		{
			largestLength = length;
			bestNormal = normal;
		}
	}

	if (largestLength < eps)
		return false;

	plane = Plane((*this->vertexArray)[0], bestNormal);
	return true;
}

Vector3 ConvexPolygon::CalcCenter() const
{
	Vector3 center(0.0, 0.0, 0.0);

	for (const Vector3& vertex : *this->vertexArray)
		center += vertex;

	if (this->vertexArray->size() > 0)
		center *= 1.0 / double(this->vertexArray->size());

	return center;
}

double ConvexPolygon::CalcArea() const
{
	Vector3 center = this->CalcCenter();
	double area = 0.0;

	for (int i = 0; i < (signed)this->vertexArray->size(); i++)
	{
		int j = (i + 1) % this->vertexArray->size();

		const Vector3& vertexA = (*this->vertexArray)[i];
		const Vector3& vertexB = (*this->vertexArray)[j];

		area += (vertexA - center).Cross(vertexB - center).Length() / 2.0;
	}

	return area;
}

bool ConvexPolygon::IntersectWithLineSegment(const Vector3& pointA, const Vector3& pointB, Vector3& intersectionPoint, double eps /*= MESH_NINJA_EPS*/) const
{
	Plane plane;
	if (!this->CalcPlane(plane, eps))
		return false;

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

bool ConvexPolygon::ContainsPoint(const Vector3& point, bool* isInteriorPoint /*= nullptr*/, double eps /*= MESH_NINJA_EPS*/) const
{
	if (isInteriorPoint)
		*isInteriorPoint = false;

	// Is the given point in the same plane of the polygon?
	Plane plane;
	this->CalcPlane(plane, eps);
	if (plane.WhichSide(point, eps) != Plane::Side::NEITHER)
		return false;

	// Is the given point on the boundary of the polygon?
	if (this->ContainsPointOnBoundary(point, eps))
		return true;

	// Is the given point in the interior of the polygon?
	for (int i = 0; i < (signed)this->vertexArray->size(); i++)
	{
		int j = (i + 1) % this->vertexArray->size();
		const Vector3& vertexA = (*this->vertexArray)[i];
		const Vector3& vertexB = (*this->vertexArray)[j];
		double dot = (vertexA - point).Cross(vertexB - point).Dot(plane.normal);
		if (dot < 0.0)
			return false;
	}

	if (isInteriorPoint)
		*isInteriorPoint = true;

	return true;
}

bool ConvexPolygon::ContainsPointOnBoundary(const Vector3& point, double eps /*= MESH_NINJA_EPS*/) const
{
	for (int i = 0; i < (signed)this->vertexArray->size(); i++)
	{
		int j = (i + 1) % this->vertexArray->size();
		const Vector3& vertexA = (*this->vertexArray)[i];
		const Vector3& vertexB = (*this->vertexArray)[j];
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
			const Vector3& vertex = (*polygon->vertexArray)[j];
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

	std::vector<Vector3> hitPointArray[2];

	for (int i = 0; i < 2; i++)
	{
		const ConvexPolygon* polygon = polygonArray[i];
		const ConvexPolygon* otherPolygon = polygonArray[1 - i];

		for (int j = 0; j < (signed)polygon->vertexArray->size(); j++)
		{
			int k = (j + 1) % polygon->vertexArray->size();

			const Vector3& vertexA = (*polygon->vertexArray)[j];
			const Vector3& vertexB = (*polygon->vertexArray)[k];

			Ray ray(vertexA, vertexB - vertexA);
			double alpha = 0.0;
			if (ray.CastAgainst(*otherPolygon, alpha) && alpha <= 1.0)
			{
				Vector3 hitPoint = ray.Lerp(alpha);
				if (!hitPoint.IsEqualTo(vertexB))
					hitPointArray[i].push_back(hitPoint);
			}
		}
	}

	if (hitPointArray[0].size() <= 2 && hitPointArray[1].size() <= 2)
	{
		std::vector<Vector3> combinedHitPointArray;
		for (int i = 0; i < 2; i++)
			for (Vector3& hitPoint : hitPointArray[i])
				combinedHitPointArray.push_back(hitPoint);

		for (Vector3& hitPoint : combinedHitPointArray)
		{
			bool pointFound = false;
			for (Vector3& vertex : *this->vertexArray)
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

		const Vector3& vertexA = (*this->vertexArray)[i];
		const Vector3& vertexB = (*this->vertexArray)[j];

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
				Vector3 hitPoint = ray.Lerp(alpha);
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
				Vector3 hitPoint = ray.Lerp(alpha);
				polygonA.vertexArray->push_back(hitPoint);
				polygonB.vertexArray->push_back(hitPoint);
			}
		}
		else if (vertexASide == Plane::Side::NEITHER)
		{
			polygonA.vertexArray->push_back(vertexA);
			polygonB.vertexArray->push_back(vertexA);
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
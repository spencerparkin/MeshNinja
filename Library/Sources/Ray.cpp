#include "Ray.h"
#include "Plane.h"
#include "ConvexPolygon.h"
#include "LineSegment.h"
#include "AlgebraicSurface.h"
#include "ConvexPolygonMesh.h"
#include "AxisAlignedBoundingBox.h"

using namespace MeshNinja;

Ray::Ray()
{
}

Ray::Ray(const Ray& ray)
{
	this->origin = ray.origin;
	this->direction = ray.direction;
}

Ray::Ray(const Vector3& origin, const Vector3& direction)
{
	this->origin = origin;
	this->direction = direction;
}

/*virtual*/ Ray::~Ray()
{
}

bool Ray::CastAgainst(const Plane& plane, double& alpha, double eps /* = MESH_NINJA_EPS*/) const
{
	if (plane.WhichSide(this->origin, eps) == Plane::Side::NEITHER)
	{
		alpha = 0.0;
		return true;
	}

	double dotDenominator = this->direction.Dot(plane.normal);
	if (fabs(dotDenominator) < eps)
		return false;

	double dotNumerator = (plane.CalcCenter() - this->origin).Dot(plane.normal);
	alpha = dotNumerator / dotDenominator;
	if (::isnan(alpha) || ::isinf(alpha))
		return false;

	return alpha >= 0.0;
}

bool Ray::CastAgainst(const LineSegment& lineSegment, double& alpha, double eps /*= MESH_NINJA_EPS*/) const
{
	Ray ray(lineSegment.vertexA, lineSegment.vertexB - lineSegment.vertexA);

	double beta = 0.0;
	if (!Ray::Intersect(*this, ray, alpha, beta, eps))
		return false;

	Vector3 point = this->Lerp(alpha);
	return lineSegment.ContainsPoint(point, eps);
}

bool Ray::CastAgainst(const ConvexPolygon& polygon, double& alpha, double eps /*= MESH_NINJA_EPS*/) const
{
	if (polygon.ContainsPoint(this->origin, nullptr, eps))
	{
		alpha = 0.0;
		return true;
	}

	Plane plane;
	polygon.CalcPlane(plane, eps);
	if (this->direction.Dot(plane.normal) < eps && plane.WhichSide(this->origin) == Plane::Side::NEITHER)
	{
		double smallestAlpha = DBL_MAX;
		for (int i = 0; i < (signed)polygon.vertexArray->size(); i++)
		{
			int j = (i + 1) % polygon.vertexArray->size();
			const Vector3& vertexA = (*polygon.vertexArray)[i];
			const Vector3& vertexB = (*polygon.vertexArray)[j];
			LineSegment line(vertexA, vertexB);
			if (this->CastAgainst(line, alpha, eps) && alpha < smallestAlpha)
				smallestAlpha = alpha;
		}

		if (smallestAlpha != DBL_MAX)
		{
			alpha = smallestAlpha;
			return true;
		}
	}
	
	if (!this->CastAgainst(plane, alpha))
		return false;

	Vector3 hitPoint = this->Lerp(alpha);
	return polygon.ContainsPoint(hitPoint, nullptr, eps);
}

bool Ray::CastAgainst(const AlgebraicSurface& algebraicSurface, double& alpha,
								double eps /*= MESH_NINJA_EPS*/,
								int maxIterations /*= 100*/,
								double initialStepSize /*= 1.0*/,
								bool forwardOrBackward /*= false*/) const
{
	Vector3 unitDirection = this->direction.Normalized();
	double signedStepSize = initialStepSize;
	int iterationCount = 0;
	Vector3 currentPoint(this->origin);

	while (iterationCount++ < maxIterations)
	{
		double currentValue = algebraicSurface.Evaluate(currentPoint);
		if (::fabs(currentValue) < eps)
		{
			alpha = this->LerpInverse(currentPoint);
			return forwardOrBackward || alpha >= 0.0;
		}

		double currentDerivativeValue = algebraicSurface.EvaluateDirectionalDerivative(currentPoint, unitDirection * MESH_NINJA_SIGN(signedStepSize));
		if (MESH_NINJA_SIGN(currentValue) == MESH_NINJA_SIGN(currentDerivativeValue))
		{
			signedStepSize = -signedStepSize / 2.0;
		}

		currentPoint += unitDirection * signedStepSize;
	}

	return false;
}

// Of course, if the polygons of a mesh were thrown into a spacial sorting data-structure first,
// and then we cast against that, then we would be more efficient.  This is fine for now.
bool Ray::CastAgainst(const ConvexPolygonMesh& mesh, double& alpha, double eps /*= MESH_NINJA_EPS*/) const
{
	std::vector<ConvexPolygon> polygonArray;
	mesh.ToConvexPolygonArray(polygonArray);

	alpha = DBL_MAX;
	for (const ConvexPolygon& polygon : polygonArray)
	{
		double beta = 0.0;
		if (this->CastAgainst(polygon, beta, eps))
		{
			if (beta < alpha)
				alpha = beta;
		}
	}

	return alpha != DBL_MAX;
}

bool Ray::CastAgainst(const AxisAlignedBoundingBox& aabb, double& alpha, double eps /*= MESH_NINJA_EPS*/) const
{
	Plane plane;

	for (int i = 0; i < 6; i++)
	{
		switch (i)
		{
			case 0: plane = Plane(aabb.min, Vector3(-1.0, 0.0, 0.0)); break;
			case 1: plane = Plane(aabb.min, Vector3(0.0, -1.0, 0.0)); break;
			case 2: plane = Plane(aabb.min, Vector3(0.0, 0.0, -1.0)); break;
			case 3: plane = Plane(aabb.max, Vector3(+1.0, 0.0, 0.0)); break;
			case 4: plane = Plane(aabb.max, Vector3(0.0, +1.0, 0.0)); break;
			case 5: plane = Plane(aabb.max, Vector3(0.0, 0.0, +1.0)); break;
		}

		if (this->CastAgainst(plane, alpha, eps))
		{
			if (aabb.ContainsPoint(this->Lerp(alpha), eps))
				return true;
		}
	}

	return false;
}

Vector3 Ray::Lerp(double alpha) const
{
	return this->origin + this->direction * alpha;
}

double Ray::LerpInverse(const Vector3& point) const
{
	return (point - this->origin).Dot(this->direction) / this->direction.Dot(this->direction);
}

/*static*/ bool Ray::Intersect(const Ray& rayA, const Ray& rayB, double& alpha, double& beta, double eps /*= MESH_NINJA_EPS*/)
{
	Vector3 normal = rayA.direction.Normalized().Cross(rayB.direction.Normalized());
	double length = normal.Length();
	if (length < eps || !normal.Normalize())
	{
		// The rays are parallel.
		if (rayA.origin.IsEqualTo(rayB.origin, eps))
		{
			// The rays are the same ray.
			alpha = 0.0;
			beta = 0.0;
			return true;
		}

		return false;
	}
	
	Plane plane(rayA.origin, normal);
	if (plane.WhichSide(rayB.origin, eps) != Plane::Side::NEITHER)
	{
		// The rays are not in the same plane.
		return false;
	}

	Plane planeA(rayA.origin, rayA.direction.Cross(normal));
	if (!rayB.CastAgainst(planeA, beta, eps))
		return false;

	Plane planeB(rayB.origin, rayB.direction.Cross(normal));
	if (!rayA.CastAgainst(planeB, alpha, eps))
		return false;

	// Perform sanity check.
	//assert(rayA.Lerp(alpha).IsEqualTo(rayB.Lerp(beta), eps));

	return true;
}
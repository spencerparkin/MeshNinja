#include "AlgebraicSurface.h"
#include "Ray.h"

using namespace MeshNinja;

//--------------------------------- AlgebraicSurface ---------------------------------

AlgebraicSurface::AlgebraicSurface()
{
}

/*virtual*/ AlgebraicSurface::~AlgebraicSurface()
{
}

/*virtual*/ Vector AlgebraicSurface::EvaluateGradient(const Vector& point) const
{
	return Vector(0.0, 0.0, 0.0);	// TODO: Approximate gradient here using central differencing?
}

/*virtual*/ double AlgebraicSurface::EvaluateDirectionalDerivative(const Vector& point, const Vector& unitDirection) const
{
	return this->EvaluateGradient(point).Dot(unitDirection);
}

/*virtual*/ double AlgebraicSurface::ApproximateDirectionalDerivative(const Vector& point, const Vector& unitDirection, double delta) const
{
	return (this->Evaluate(point + unitDirection * delta) - this->Evaluate(point)) / delta;
}

//--------------------------------- QuadraticSurface ---------------------------------

QuadraticSurface::QuadraticSurface()
{
	this->a = 0.0;
	this->b = 0.0;
	this->c = 0.0;
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 0.0;
	this->j = 0.0;
}

/*virtual*/ QuadraticSurface::~QuadraticSurface()
{
}

/*virtual*/ double QuadraticSurface::Evaluate(const Vector& point) const
{
	double x = point.x;
	double y = point.y;
	double z = point.z;
	double xx = x * x;
	double xy = x * y;
	double xz = x * z;
	double yy = y * y;
	double yz = y * z;
	double zz = z * z;

	double result =
		this->a * xx + this->b * yy + this->c * zz +
		this->d * xy + this->e * yz + this->f * xz +
		this->g * x + this->h * y + this->i * z +
		this->j;

	return result;
}

/*virtual*/ Vector QuadraticSurface::EvaluateGradient(const Vector& point) const
{
	double x = point.x;
	double y = point.y;
	double z = point.z;

	return Vector(
		2.0 * this->a * x + this->d * y + this->f * z + this->g,
		2.0 * this->b * y + this->d * x + this->e * z + this->h,
		2.0 * this->c * z + this->e * y + this->f * x + this->i);
}

void QuadraticSurface::MakeEllipsoid(double A, double B, double C)
{
	this->a = 1.0 / (A * A);
	this->b = 1.0 / (B * B);
	this->c = 1.0 / (C * C);
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 0.0;
	this->j = -1.0;
}

void QuadraticSurface::MakeEllipticCone(double A, double B, double C)
{
	this->a = 1.0 / (A * A);
	this->b = 1.0 / (B * B);
	this->c = -1.0 / (C * C);
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 0.0;
	this->j = 0.0;
}

void QuadraticSurface::MakeEllipticCylinder(double A, double B)
{
	this->a = 1.0 / (A * A);
	this->b = 1.0 / (B * B);
	this->c = 0.0;
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 0.0;
	this->j = -1.0;
}

void QuadraticSurface::MakeEllipticParaboloid(double A, double B)
{
	this->a = 1.0 / (A * A);
	this->b = 1.0 / (B * B);
	this->c = 0.0;
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 1.0;
	this->j = 0.0;
}

void QuadraticSurface::MakeHyperbolicCylinder(double A, double B)
{
	this->a = 1.0 / (A * A);
	this->b = -1.0 / (B * B);
	this->c = 0.0;
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 0.0;
	this->j = 1.0;
}

void QuadraticSurface::MakeHyperbolicParaboloid(double A, double B)
{
	this->a = -1.0 / (A * A);
	this->b = 1.0 / (B * B);
	this->c = 0.0;
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = -1.0;
	this->j = 0.0;
}

void QuadraticSurface::MakeHyperboloidOfOneSheet(double A, double B, double C)
{
	this->a = 1.0 / (A * A);
	this->b = 1.0 / (B * B);
	this->c = -1.0 / (C * C);
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 0.0;
	this->j = 1.0;
}

void QuadraticSurface::MakeHyperboloidOfTwoSheets(double A, double B, double C)
{
	this->a = 1.0 / (A * A);
	this->b = 1.0 / (B * B);
	this->c = -1.0 / (C * C);
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 0.0;
	this->j = -1.0;
}

void QuadraticSurface::MakeIntersectingPlanes(double A, double B)
{
	this->a = 1.0 / (A * A);
	this->b = -1.0 / (B * B);
	this->c = 0.0;
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 0.0;
	this->j = 0.0;
}

void QuadraticSurface::MakeParabolicSylinder(double R)
{
	this->a = 1.0;
	this->b = 0.0;
	this->c = 0.0;
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 2.0 * R;
	this->j = 0.0;
}

void QuadraticSurface::MakeParallelPlanes(double A)
{
	this->a = 1.0;
	this->b = 0.0;
	this->c = 0.0;
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 0.0;
	this->j = -A * A;
}
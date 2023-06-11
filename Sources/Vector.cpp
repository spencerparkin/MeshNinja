#include "Vector.h"

using namespace MeshNinja;

Vector::Vector()
{
	this->x = 0.0;
	this->y = 0.0;
	this->z = 0.0;
}

Vector::Vector(double x, double y, double z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

Vector::Vector(const Vector& vector)
{
	this->x = vector.x;
	this->y = vector.y;
	this->z = vector.z;
}

/*virtual*/ Vector::~Vector()
{
}

bool Vector::operator==(const Vector& vector)
{
	return this->x == vector.x && this->y == vector.y && this->z == vector.z;
}

void Vector::operator=(const Vector& vector)
{
	this->x = vector.x;
	this->y = vector.y;
	this->z = vector.z;
}

void Vector::operator+=(const Vector& vector)
{
	this->x += vector.x;
	this->y += vector.y;
	this->z += vector.z;
}

void Vector::operator-=(const Vector& vector)
{
	this->x -= vector.x;
	this->y -= vector.y;
	this->z -= vector.z;
}

void Vector::operator*=(double scale)
{
	this->x *= scale;
	this->y *= scale;
	this->z *= scale;
}

Vector Vector::operator*(double scale)
{
	return Vector(
		this->x * scale,
		this->y * scale,
		this->z * scale);
}

double Vector::Length() const
{
	return ::sqrt(this->Dot(*this));
}

bool Vector::Normalize(double* returnedLength /*= nullptr*/)
{
	double length = this->Length();
	if (length == 0.0)
		return false;

	double scale = 1.0 / length;
	if (::isnan(scale) || ::isinf(scale))
		return false;

	*this *= scale;
	if (returnedLength)
		*returnedLength = length;

	return true;
}

double Vector::Dot(const Vector& vector) const
{
	double dot =
		this->x * vector.x +
		this->y * vector.y +
		this->z * vector.z;

	return dot;
}

Vector Vector::Cross(const Vector& vector) const
{
	return Vector(
		this->y * vector.z - this->z * vector.y,
		this->z * vector.x - this->x * vector.z,
		this->x * vector.y - this->y * vector.x);
}

bool Vector::ProjectOnto(const Vector& vector)
{
	Vector normal(vector);
	if (!normal.Normalize())
		return false;

	*this = normal * this->Dot(normal);
	return true;
}

bool Vector::RejectFrom(const Vector& vector)
{
	Vector projection(*this);
	if (!projection.ProjectOnto(vector))
		return false;

	*this -= projection;
	return true;
}

bool Vector::RotateAbout(const Vector& vector, double angle)
{
	Vector axis(vector);
	if (!axis.Normalize())
		return false;

	Vector projection = axis * this->Dot(axis);
	Vector rejection = *this - projection;

	Vector xAxis(rejection);
	double length = 0.0;
	if (xAxis.Normalize(&length))
	{
		Vector yAxis = axis.Cross(xAxis);
		rejection = (xAxis * ::cos(angle) + yAxis * ::sin(angle)) * length;
		*this = rejection + projection;
	}
	
	return true;
}

namespace MeshNinja
{
	Vector operator+(const Vector& vectorA, const Vector& vectorB)
	{
		return Vector(
			vectorA.x + vectorB.x,
			vectorA.y + vectorB.y,
			vectorA.z + vectorB.z);
	}

	Vector operator-(const Vector& vectorA, const Vector& vectorB)
	{
		return Vector(
			vectorA.x - vectorB.x,
			vectorA.y - vectorB.y,
			vectorA.z - vectorB.z);
	}

	Vector operator*(const Vector& vector, double scale)
	{
		return Vector(
			vector.x * scale,
			vector.y * scale,
			vector.z * scale);
	}

	Vector operator*(double scale, const Vector& vector)
	{
		return Vector(
			vector.x * scale,
			vector.y * scale,
			vector.z * scale);
	}
}
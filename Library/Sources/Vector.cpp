#include "Vector.h"
#include "Quaternion.h"

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

Vector::Vector(const Quaternion& quat)
{
	this->x = quat.x;
	this->y = quat.y;
	this->z = quat.z;
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

void Vector::operator/=(double scale)
{
	this->x /= scale;
	this->y /= scale;
	this->z /= scale;
}

Vector Vector::operator*(double scale)
{
	return Vector(
		this->x * scale,
		this->y * scale,
		this->z * scale);
}

Vector Vector::operator-() const
{
	return Vector(
		-this->x,
		-this->y,
		-this->z);
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

Vector Vector::Normalized() const
{
	Vector unitVector(*this);
	unitVector.Normalize();
	return unitVector;
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

bool Vector::Intersect(const Line& line, const Plane& plane)
{
	return false;
}

bool Vector::ToString(std::string& str) const
{
	std::stringstream stringStream;
	stringStream << this->x << ", " << this->y << ", " << this->z;
	str = stringStream.str();
	return true;
}

bool Vector::FromString(const std::string& str)
{
	// TODO: Write this.
	return false;
}

bool Vector::MakeOrthogonalTo(const Vector& vector)
{
	double ax = fabs(vector.x);
	double ay = fabs(vector.y);
	double az = fabs(vector.z);

	if (ax > ay && ax > az)
	{
		this->z = vector.x;
		this->x = -vector.z;
		this->y = 0.0;
		return true;
	}
	else if (ay > ax && ay > az)
	{
		this->x = vector.y;
		this->y = -vector.x;
		this->z = 0.0;
		return true;
	}
	else if (az > ax && az > ay)
	{
		this->y = vector.z;
		this->z = -vector.y;
		this->x = 0.0;
		return true;
	}

	return false;
}

bool Vector::IsEqualTo(const Vector& vector, double eps /*= MESH_NINJA_EPS*/) const
{
	return (*this - vector).Length() < eps;
}

double Vector::AngleBetweenThisAnd(const Vector& vector) const
{
	Vector unitNormalA(*this);
	Vector unitNormalB(vector);

	if (!unitNormalA.Normalize() || !unitNormalB.Normalize())
		return 0.0;

	double dot = unitNormalA.Dot(unitNormalB);
	dot = MESH_NINJA_CLAMP(dot, -1.0, 1.0);
	return ::acos(dot);
}

void Vector::Max(const Vector& vectorA, const Vector& vectorB)
{
	this->x = MESH_NINJA_MAX(vectorA.x, vectorB.x);
	this->y = MESH_NINJA_MAX(vectorA.y, vectorB.y);
	this->z = MESH_NINJA_MAX(vectorA.z, vectorB.z);
}

void Vector::Min(const Vector& vectorA, const Vector& vectorB)
{
	this->x = MESH_NINJA_MIN(vectorA.x, vectorB.x);
	this->y = MESH_NINJA_MIN(vectorA.y, vectorB.y);
	this->z = MESH_NINJA_MIN(vectorA.z, vectorB.z);
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

	Vector operator/(const Vector& vector, double scale)
	{
		return Vector(
			vector.x / scale,
			vector.y / scale,
			vector.z / scale);
	}

	bool operator<(const Vector& vectorA, const Vector& vectorB)
	{
		std::string strA, strB;
		vectorA.ToString(strA);
		vectorB.ToString(strB);
		return ::strcmp(strA.c_str(), strB.c_str()) < 0;
	}
}
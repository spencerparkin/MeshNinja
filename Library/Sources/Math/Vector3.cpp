#include "Vector3.h"
#include "Quaternion.h"

using namespace MeshNinja;

Vector3::Vector3()
{
	this->x = 0.0;
	this->y = 0.0;
	this->z = 0.0;
}

Vector3::Vector3(double x, double y, double z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

Vector3::Vector3(const Vector3& vector)
{
	this->x = vector.x;
	this->y = vector.y;
	this->z = vector.z;
}

Vector3::Vector3(const Quaternion& quat)
{
	this->x = quat.x;
	this->y = quat.y;
	this->z = quat.z;
}

/*virtual*/ Vector3::~Vector3()
{
}

bool Vector3::operator==(const Vector3& vector)
{
	return this->x == vector.x && this->y == vector.y && this->z == vector.z;
}

void Vector3::operator=(const Vector3& vector)
{
	this->x = vector.x;
	this->y = vector.y;
	this->z = vector.z;
}

void Vector3::operator+=(const Vector3& vector)
{
	this->x += vector.x;
	this->y += vector.y;
	this->z += vector.z;
}

void Vector3::operator-=(const Vector3& vector)
{
	this->x -= vector.x;
	this->y -= vector.y;
	this->z -= vector.z;
}

void Vector3::operator*=(double scale)
{
	this->x *= scale;
	this->y *= scale;
	this->z *= scale;
}

void Vector3::operator/=(double scale)
{
	this->x /= scale;
	this->y /= scale;
	this->z /= scale;
}

Vector3 Vector3::operator*(double scale)
{
	return Vector3(
		this->x * scale,
		this->y * scale,
		this->z * scale);
}

Vector3 Vector3::operator-() const
{
	return Vector3(
		-this->x,
		-this->y,
		-this->z);
}

void Vector3::SetComponents(double x, double y, double z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

void Vector3::GetComponents(double& x, double& y, double& z) const
{
	x = this->x;
	y = this->y;
	z = this->z;
}

double Vector3::Length() const
{
	return ::sqrt(this->Dot(*this));
}

bool Vector3::Normalize(double* returnedLength /*= nullptr*/)
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

Vector3 Vector3::Normalized() const
{
	Vector3 unitVector(*this);
	unitVector.Normalize();
	return unitVector;
}

double Vector3::Dot(const Vector3& vector) const
{
	double dot =
		this->x * vector.x +
		this->y * vector.y +
		this->z * vector.z;

	return dot;
}

Vector3 Vector3::Cross(const Vector3& vector) const
{
	return Vector3(
		this->y * vector.z - this->z * vector.y,
		this->z * vector.x - this->x * vector.z,
		this->x * vector.y - this->y * vector.x);
}

/*static*/ double Vector3::Dot(const Vector3& vectorA, const Vector3& vectorB)
{
	return vectorA.Dot(vectorB);
}

Vector3& Vector3::Cross(const Vector3& vectorA, const Vector3& vectorB)
{
	*this = vectorA.Cross(vectorB);
	return *this;
}

bool Vector3::ProjectOnto(const Vector3& vector)
{
	Vector3 normal(vector);
	if (!normal.Normalize())
		return false;

	*this = normal * this->Dot(normal);
	return true;
}

bool Vector3::RejectFrom(const Vector3& vector)
{
	Vector3 projection(*this);
	if (!projection.ProjectOnto(vector))
		return false;

	*this -= projection;
	return true;
}

bool Vector3::RotateAbout(const Vector3& vector, double angle)
{
	Vector3 axis(vector);
	if (!axis.Normalize())
		return false;

	Vector3 projection = axis * this->Dot(axis);
	Vector3 rejection = *this - projection;

	Vector3 xAxis(rejection);
	double length = 0.0;
	if (xAxis.Normalize(&length))
	{
		Vector3 yAxis = axis.Cross(xAxis);
		rejection = (xAxis * ::cos(angle) + yAxis * ::sin(angle)) * length;
		*this = rejection + projection;
	}
	
	return true;
}

bool Vector3::Intersect(const Line& line, const Plane& plane)
{
	return false;
}

bool Vector3::ToString(std::string& str) const
{
	std::stringstream stringStream;
	stringStream << this->x << ", " << this->y << ", " << this->z;
	str = stringStream.str();
	return true;
}

bool Vector3::FromString(const std::string& str)
{
	// TODO: Write this.
	return false;
}

bool Vector3::MakeOrthogonalTo(const Vector3& vector)
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

bool Vector3::IsEqualTo(const Vector3& vector, double eps /*= MESH_NINJA_EPS*/) const
{
	return (*this - vector).Length() < eps;
}

double Vector3::AngleBetweenThisAnd(const Vector3& vector) const
{
	Vector3 unitNormalA(*this);
	Vector3 unitNormalB(vector);

	if (!unitNormalA.Normalize() || !unitNormalB.Normalize())
		return 0.0;

	double dot = unitNormalA.Dot(unitNormalB);
	dot = MESH_NINJA_CLAMP(dot, -1.0, 1.0);
	return ::acos(dot);
}

void Vector3::Max(const Vector3& vectorA, const Vector3& vectorB)
{
	this->x = MESH_NINJA_MAX(vectorA.x, vectorB.x);
	this->y = MESH_NINJA_MAX(vectorA.y, vectorB.y);
	this->z = MESH_NINJA_MAX(vectorA.z, vectorB.z);
}

void Vector3::Min(const Vector3& vectorA, const Vector3& vectorB)
{
	this->x = MESH_NINJA_MIN(vectorA.x, vectorB.x);
	this->y = MESH_NINJA_MIN(vectorA.y, vectorB.y);
	this->z = MESH_NINJA_MIN(vectorA.z, vectorB.z);
}

unsigned int Vector3::ToColor() const
{
	unsigned int r = unsigned int(MESH_NINJA_CLAMP(this->x, 0.0, 1.0) * 255.0);
	unsigned int g = unsigned int(MESH_NINJA_CLAMP(this->y, 0.0, 1.0) * 255.0);
	unsigned int b = unsigned int(MESH_NINJA_CLAMP(this->z, 0.0, 1.0) * 255.0);

	unsigned int color = (r << 0) | (g << 8) | (b << 16);
	return color;
}

void Vector3::FromColor(unsigned int color)
{
	unsigned int r = (color >> 0) & 0xFF;
	unsigned int g = (color >> 8) & 0xFF;
	unsigned int b = (color >> 16) & 0xFF;

	this->x = double(r) / 255.0;
	this->y = double(g) / 255.0;
	this->z = double(b) / 255.0;
}

namespace MeshNinja
{
	Vector3 operator+(const Vector3& vectorA, const Vector3& vectorB)
	{
		return Vector3(
			vectorA.x + vectorB.x,
			vectorA.y + vectorB.y,
			vectorA.z + vectorB.z);
	}

	Vector3 operator-(const Vector3& vectorA, const Vector3& vectorB)
	{
		return Vector3(
			vectorA.x - vectorB.x,
			vectorA.y - vectorB.y,
			vectorA.z - vectorB.z);
	}

	Vector3 operator*(const Vector3& vector, double scale)
	{
		return Vector3(
			vector.x * scale,
			vector.y * scale,
			vector.z * scale);
	}

	Vector3 operator*(double scale, const Vector3& vector)
	{
		return Vector3(
			vector.x * scale,
			vector.y * scale,
			vector.z * scale);
	}

	Vector3 operator/(const Vector3& vector, double scale)
	{
		return Vector3(
			vector.x / scale,
			vector.y / scale,
			vector.z / scale);
	}

	bool operator<(const Vector3& vectorA, const Vector3& vectorB)
	{
		std::string strA, strB;
		vectorA.ToString(strA);
		vectorB.ToString(strB);
		return ::strcmp(strA.c_str(), strB.c_str()) < 0;
	}
}
#include "Vector4.h"
#include "Vector3.h"

using namespace MeshNinja;

Vector4::Vector4()
{
	this->x = 0.0;
	this->y = 0.0;
	this->z = 0.0;
	this->w = 0.0;
}

Vector4::Vector4(double x, double y, double z, double w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

Vector4::Vector4(const Vector3& vector, double w)
{
	this->x = vector.x;
	this->y = vector.y;
	this->z = vector.z;
	this->w = w;
}

Vector4::Vector4(const Vector4& vector)
{
	this->x = vector.x;
	this->y = vector.y;
	this->z = vector.z;
	this->w = vector.w;
}

/*virtual*/ Vector4::~Vector4()
{
}

bool Vector4::operator==(const Vector4& vector)
{
	bool equal =
		this->x == vector.x &&
		this->y == vector.y &&
		this->z == vector.z &&
		this->w == vector.w;

	return equal;
}

void Vector4::operator=(const Vector4& vector)
{
	this->x = vector.x;
	this->y = vector.y;
	this->z = vector.z;
	this->w = vector.w;
}

void Vector4::operator+=(const Vector4& vector)
{
	this->x += vector.x;
	this->y += vector.y;
	this->z += vector.z;
	this->w += vector.w;
}

void Vector4::operator-=(const Vector4& vector)
{
	this->x -= vector.x;
	this->y -= vector.y;
	this->z -= vector.z;
	this->w -= vector.w;
}

void Vector4::operator*=(double scale)
{
	this->x *= scale;
	this->y *= scale;
	this->z *= scale;
	this->w *= scale;
}

void Vector4::operator/=(double scale)
{
	this->x /= scale;
	this->y /= scale;
	this->z /= scale;
	this->w /= scale;
}

Vector4 Vector4::operator*(double scale)
{
	return Vector4(
		this->x * scale,
		this->y * scale,
		this->z * scale,
		this->w * scale
	);
}

Vector4 Vector4::operator-() const
{
	return Vector4(
		-this->x,
		-this->y,
		-this->z,
		-this->w
	);
}

void Vector4::SetComponents(double x, double y, double z, double w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

void Vector4::GetComponents(double& x, double& y, double& z, double& w) const
{
	x = this->x;
	y = this->y;
	z = this->z;
	w = this->w;
}

double Vector4::Length() const
{
	return ::sqrt(
		this->x * this->x +
		this->y * this->y +
		this->z * this->z +
		this->w * this->w
	);
}

bool Vector4::Normalize(double* returnedLength /*= nullptr*/)
{
	double length = this->Length();
	if (returnedLength)
		*returnedLength = length;

	if (length == 0.0)
		return false;

	double scale = 1.0 / length;
	if (::isnan(scale) || ::isinf(scale))
		return false;

	*this *= scale;
	return true;
}

Vector4 Vector4::Normalized() const
{
	Vector4 normalized(*this);
	normalized.Normalize();
	return normalized;
}

bool Vector4::Homogenize()
{
	if (this->w == 0.0)
		return false;

	double scale = 1.0 / w;
	if (::isnan(scale) || ::isinf(scale))
		return false;

	*this *= scale;
	return true;
}

Vector4 Vector4::Homogenized() const
{
	Vector4 homogenized(*this);
	homogenized.Homogenize();
	return homogenized;
}

namespace MeshNinja
{
	Vector4 operator+(const Vector4& vectorA, const Vector4& vectorB)
	{
		return Vector4(
			vectorA.x + vectorB.x,
			vectorA.y + vectorB.y,
			vectorA.z + vectorB.z,
			vectorA.w + vectorB.w
		);
	}

	Vector4 operator-(const Vector4& vectorA, const Vector4& vectorB)
	{
		return Vector4(
			vectorA.x - vectorB.x,
			vectorA.y - vectorB.y,
			vectorA.z - vectorB.z,
			vectorA.w - vectorB.w
		);
	}

	Vector4 operator*(const Vector4& vector, double scale)
	{
		return Vector4(
			vector.x * scale,
			vector.y * scale,
			vector.z * scale,
			vector.w * scale
		);
	}

	Vector4 operator*(double scale, const Vector4& vector)
	{
		return Vector4(
			vector.x * scale,
			vector.y * scale,
			vector.z * scale,
			vector.w * scale
		);
	}

	Vector4 operator/(const Vector4& vector, double scale)
	{
		return Vector4(
			vector.x / scale,
			vector.y / scale,
			vector.z / scale,
			vector.w / scale
		);
	}
}
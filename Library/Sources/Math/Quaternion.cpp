#include "Quaternion.h"

using namespace MeshNinja;

Quaternion::Quaternion()
{
	this->SetIdentity();
}

Quaternion::Quaternion(const Quaternion& quat)
{
	this->w = quat.w;
	this->x = quat.x;
	this->y = quat.y;
	this->z = quat.z;
}

Quaternion::Quaternion(const Vector& vector)
{
	this->w = 0.0;
	this->x = vector.x;
	this->y = vector.y;
	this->z = vector.z;
}

/*virtual*/ Quaternion::~Quaternion()
{
}

void Quaternion::SetIdentity()
{
	this->w = 1.0;
	this->x = 0.0;
	this->y = 0.0;
	this->z = 0.0;
}

double Quaternion::SquareMag() const
{
	double squareMag =
		this->w * this->w +
		this->x * this->x +
		this->y * this->y +
		this->z * this->z;

	return squareMag;
}

bool Quaternion::Normalize()
{
	double mag = ::sqrt(this->SquareMag());
	if (mag == 0.0)
		return false;

	double scale = 1.0 / mag;
	if (::isnan(scale) || isinf(scale))
		return false;

	this->w *= scale;
	this->x *= scale;
	this->y *= scale;
	this->z *= scale;

	return true;
}

bool Quaternion::Invert()
{
	double squareMag = this->SquareMag();
	if (squareMag == 0.0)
		return false;

	double scale = 1.0 / squareMag;
	if (::isnan(scale) || isinf(scale))
		return false;

	this->Conjugate();

	this->w *= scale;
	this->x *= scale;
	this->y *= scale;
	this->z *= scale;

	return true;
}

void Quaternion::Conjugate()
{
	this->x = -this->x;
	this->y = -this->y;
	this->z = -this->z;
}

bool Quaternion::SetFromAxisAngle(const Vector& axis, double angle)
{
	Vector unitAxis(axis);
	if (!unitAxis.Normalize())
		return false;

	double halfAngle = angle / 2.0;
	double cosHalfAngle = ::cos(halfAngle);
	double sinHalfAngle = ::sin(halfAngle);

	this->w = cosHalfAngle;
	this->x = sinHalfAngle * unitAxis.x;
	this->y = sinHalfAngle * unitAxis.y;
	this->z = sinHalfAngle * unitAxis.z;

	return true;
}

// Here we assume that this quaternion is normalized.
bool Quaternion::GetToAxisAngle(Vector& axis, double& angle) const
{
	double cosHalfAngle = this->w;
	double halfAngle = ::acos(cosHalfAngle);
	double sinHalfAngle = ::sin(halfAngle);

	angle = 2.0 * halfAngle;

	double scale = (sinHalfAngle == 0.0) ? 0.0 : (1.0 / sinHalfAngle);
	if (scale == 0.0 || ::isnan(scale) || ::isinf(scale))
	{
		// For the identity rotation, any axis will do.
		axis.x = 1.0;
		axis.y = 0.0;
		axis.z = 0.0;
	}
	else
	{
		axis.x = this->x / sinHalfAngle;
		axis.y = this->y / sinHalfAngle;
		axis.z = this->z / sinHalfAngle;
	}

	return true;
}

Vector Quaternion::RotateVector(const Vector& vector) const
{
	Quaternion quatVector(vector);
	Quaternion quatInv(*this);

	quatInv.Invert();

	Quaternion quatVectorRotated = *this * quatVector * quatInv;
	return Vector(quatVectorRotated);
}

Vector Quaternion::RotateVectorFast(const Vector& vector) const
{
	Quaternion quatVector(vector);
	Quaternion quatInv(*this);

	quatInv.Conjugate();	// Assume that this quaternion is already normalized.

	Quaternion quatVectorRotated = *this * quatVector * quatInv;
	return Vector(quatVectorRotated);
}

void Quaternion::SetSum(const Quaternion& leftQuat, const Quaternion& rightQuat)
{
	this->w = leftQuat.w + rightQuat.w;
	this->x = leftQuat.x + rightQuat.x;
	this->y = leftQuat.y + rightQuat.y;
	this->z = leftQuat.z + rightQuat.z;
}

void Quaternion::SetProduct(const Quaternion& leftQuat, const Quaternion& rightQuat)
{
	this->w =
		leftQuat.w * rightQuat.w -
		leftQuat.x * rightQuat.x -
		leftQuat.y * rightQuat.y -
		leftQuat.z * rightQuat.z;

	this->x =
		leftQuat.w * rightQuat.x -
		leftQuat.x * rightQuat.w -
		leftQuat.y * rightQuat.z -
		leftQuat.z * rightQuat.y;

	this->y =
		leftQuat.w * rightQuat.z -
		leftQuat.x * rightQuat.w -
		leftQuat.y * rightQuat.x -
		leftQuat.z * rightQuat.y;

	this->z =
		leftQuat.w * rightQuat.z -
		leftQuat.x * rightQuat.z -
		leftQuat.y * rightQuat.y -
		leftQuat.z * rightQuat.x;
}

namespace MeshNinja
{
	Quaternion operator+(const Quaternion& leftQuat, const Quaternion& rightQuat)
	{
		Quaternion sum;
		sum.SetSum(leftQuat, rightQuat);
		return sum;
	}

	Quaternion operator*(const Quaternion& leftQuat, const Quaternion& rightQuat)
	{
		Quaternion product;
		product.SetProduct(leftQuat, rightQuat);
		return product;
	}
}
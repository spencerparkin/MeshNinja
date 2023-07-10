#pragma once

#include "Common.h"
#include "Vector.h"

namespace MeshNinja
{
	class MESH_NINJA_API Quaternion
	{
	public:
		Quaternion();
		Quaternion(const Quaternion& quat);
		Quaternion(const Vector& vector);
		virtual ~Quaternion();

		double SquareMag() const;

		void SetIdentity();

		bool Normalize();
		bool Invert();
		void Conjugate();

		bool SetFromAxisAngle(const Vector& axis, double angle);
		bool GetToAxisAngle(Vector& axis, double& angle) const;

		Vector RotateVector(const Vector& vector) const;
		Vector RotateVectorFast(const Vector& vector) const;

		void SetSum(const Quaternion& leftQuat, const Quaternion& rightQuat);
		void SetProduct(const Quaternion& leftQuat, const Quaternion& rightQuat);

		double x, y, z, w;
	};

	Quaternion operator+(const Quaternion& leftQuat, const Quaternion& rightQuat);
	Quaternion operator*(const Quaternion& leftQuat, const Quaternion& rightQuat);
}
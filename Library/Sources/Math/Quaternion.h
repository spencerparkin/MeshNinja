#pragma once

#include "../Common.h"
#include "Vector3.h"

namespace MeshNinja
{
	class MESH_NINJA_API Quaternion
	{
	public:
		Quaternion();
		Quaternion(const Quaternion& quat);
		Quaternion(const Vector3& vector);
		virtual ~Quaternion();

		double SquareMag() const;

		void SetIdentity();

		bool Normalize();
		bool Invert();
		void Conjugate();

		bool SetFromAxisAngle(const Vector3& axis, double angle);
		bool GetToAxisAngle(Vector3& axis, double& angle) const;

		Vector3 RotateVector(const Vector3& vector) const;
		Vector3 RotateVectorFast(const Vector3& vector) const;

		void SetSum(const Quaternion& leftQuat, const Quaternion& rightQuat);
		void SetProduct(const Quaternion& leftQuat, const Quaternion& rightQuat);

		double x, y, z, w;
	};

	Quaternion MESH_NINJA_API operator+(const Quaternion& leftQuat, const Quaternion& rightQuat);
	Quaternion MESH_NINJA_API operator*(const Quaternion& leftQuat, const Quaternion& rightQuat);
}
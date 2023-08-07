#pragma once

#include "../Common.h"
#include "Vector3.h"
#include "Quaternion.h"

namespace MeshNinja
{
	class MESH_NINJA_API Matrix3x3
	{
	public:

		Matrix3x3();
		virtual ~Matrix3x3();

		void SetIdentity(void);

		void SetRow(int i, const Vector3& vector);
		void SetCol(int j, const Vector3& vector);
		Vector3 GetRow(int i) const;
		Vector3 GetCol(int j) const;

		void Orthonormalize(void);

		void SetCopy(const Matrix3x3& matrix);
		void GetCopy(Matrix3x3& matrix) const;

		void SetFromAxisAngle(const Vector3& axis, double angle);
		bool GetToAxisAngle(Vector3& axis, double& angle) const;

		void SetFromQuat(const Quaternion& quat);
		bool GetToQuat(Quaternion& quat) const; // Must be orthonormal with determinant +1 for this to work.

		bool SetInverse(const Matrix3x3& matrix);
		bool GetInverse(Matrix3x3& matrix) const;

		void SetTranspose(const Matrix3x3& matrix);
		void GetTranspose(Matrix3x3& matrix) const;

		bool SetInverseTranspose(const Matrix3x3& matrix);

		void SetProduct(const Matrix3x3& matrixA, const Matrix3x3& matrixB);

		void MultiplyLeft(const Vector3& inVector, Vector3& outVector) const;
		void MultiplyRight(const Vector3& inVector, Vector3& outVector) const;

		void operator=(const Matrix3x3& matrix);
		void operator*=(const Matrix3x3& matrix);
		void operator/=(const Matrix3x3& matrix);
		Vector3 operator*(const Vector3& vector) const;

		double Determinant(void) const;
		double Trace(void) const;

		void Scale(double scale);

		double ele[3][3];
	};

	Matrix3x3 MESH_NINJA_API operator*(const Matrix3x3& matrixA, const Matrix3x3& matrixB);
}
#pragma once

#include "../Common.h"

namespace MeshNinja
{
	class Vector3;
	class Vector4;
	class Matrix3x3;
	class Quaternion;

	// These are 4x4 matrices geared toward 3D computer graphics applications.
	class MESH_NINJA_API Matrix4x4
	{
	public:
		Matrix4x4();
		Matrix4x4(const Matrix4x4& matrix);
		Matrix4x4(const Matrix3x3& matrix);
		Matrix4x4(const Matrix3x3& matrix, const Vector3& translation);
		virtual ~Matrix4x4();

		void Identity();

		bool SetCol(int col, const Vector3& vector);
		bool GetCol(int col, Vector3& vector) const;

		void GetAxes(Vector3& xAxis, Vector3& yAxis, Vector3& zAxis) const;
		void SetAxes(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis);

		void TransformVector(const Vector3& vector, Vector3& vectorTransformed) const;
		void TransformPoint(const Vector3& point, Vector3& pointTransformed) const;
		void TransformVector(const Vector4& vector, Vector4& vectorTransformed) const;

		void Multiply(const Matrix4x4& leftMatrix, const Matrix4x4& rightMatrix);
		bool Divide(const Matrix4x4& leftMatrix, const Matrix4x4& rightMatrix);

		bool Invert(const Matrix4x4& matrix);
		void Transpose(const Matrix4x4& matrix);

		double Determinant() const;

		void RigidBodyMotion(const Vector3& axis, double angle, const Vector3& delta);
		void RigidBodyMotion(const Quaternion& quat, const Vector3& delta);

		void Projection(double hfovi, double vfovi, double near, double far);

		void SetTranslation(const Vector3& translation);
		void SetScale(const Vector3& scale);
		void SetUniformScale(double scale);
		void SetRotation(const Vector3& axis, double angle);

		bool OrthonormalizeOrientation();

		void operator=(const Matrix4x4& matrix);
		void operator=(const Matrix3x3& matrix);

		double ele[4][4];
	};

	MESH_NINJA_API Matrix4x4 operator*(const Matrix4x4& leftMatrix, const Matrix4x4& rightMatrix);
}
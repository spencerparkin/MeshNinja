#pragma once

#include "../Common.h"

namespace MeshNinja
{
	class Vector;
	class Matrix3x3;
	class Quaternion;

	// These are 4x4 matrices geared toward 3D computer graphics applications.
	class MESH_NINJA_API Matrix4x4
	{
	public:
		Matrix4x4();
		Matrix4x4(const Matrix4x4& matrix);
		Matrix4x4(const Matrix3x3& matrix);
		Matrix4x4(const Matrix3x3& matrix, const Vector& translation);
		virtual ~Matrix4x4();

		void Identity();

		bool SetCol(int col, const Vector& vector);
		bool GetCol(int col, Vector& vector) const;

		void GetAxes(Vector& xAxis, Vector& yAxis, Vector& zAxis) const;
		void SetAxes(const Vector& xAxis, const Vector& yAxis, const Vector& zAxis);

		void TransformVector(const Vector& vector, Vector& vectorTransformed) const;
		void TransformPoint(const Vector& point, Vector& pointTransformed) const;

		void Multiply(const Matrix4x4& leftMatrix, const Matrix4x4& rightMatrix);
		bool Divide(const Matrix4x4& leftMatrix, const Matrix4x4& rightMatrix);

		bool Invert(const Matrix4x4& matrix);
		void Transpose(const Matrix4x4& matrix);

		double Determinant() const;

		void RigidBodyMotion(const Vector& axis, double angle, const Vector& delta);
		void RigidBodyMotion(const Quaternion& quat, const Vector& delta);

		void Projection(double hfovi, double vfovi, double near, double far);

		void SetTranslation(const Vector& translation);
		void SetScale(const Vector& scale);
		void SetUniformScale(double scale);
		void SetRotation(const Vector& axis, double angle);

		bool OrthonormalizeOrientation();

		void operator=(const Matrix4x4& matrix);
		void operator=(const Matrix3x3& matrix);

		double ele[4][4];
	};

	MESH_NINJA_API Matrix4x4 operator*(const Matrix4x4& leftMatrix, const Matrix4x4& rightMatrix);
}
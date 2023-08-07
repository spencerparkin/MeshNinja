#pragma once

#include "Matrix3x3.h"

namespace MeshNinja
{
	class ConvexPolygon;
	class Matrix4x4;

	class MESH_NINJA_API Transform
	{
	public:
		Transform();
		virtual ~Transform();

		void SetIdentity();
		
		bool FromMatrix4x4(const Matrix4x4& matrix);
		void ToMatrix4x4(Matrix4x4& matrix) const;

		Vector3 TransformVector(const Vector3& vector) const;
		Vector3 TransformPosition(const Vector3& position) const;
		void TransformPolygon(ConvexPolygon& polygon) const;
		
		bool SetInverse(const Transform& transform);
		bool GetInverse(Transform& transform) const;

		Transform& Multiply(const Transform& transformA, const Transform& transformB);

		Vector3 operator*(const Vector3& vector);

		Matrix3x3 matrix;
		Vector3 translation;
	};

	Transform MESH_NINJA_API operator*(const Transform& transformA, const Transform& transformB);
}
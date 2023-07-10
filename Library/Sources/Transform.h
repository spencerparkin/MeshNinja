#pragma once

#include "Matrix3x3.h"

namespace MeshNinja
{
	class MESH_NINJA_API Transform
	{
	public:
		Transform();
		virtual ~Transform();

		void SetIdentity();
		
		Vector TransformVector(const Vector& vector) const;
		Vector TransformPosition(const Vector& position) const;
		
		bool SetInverse(const Transform& transform);
		bool GetInverse(Transform& transform) const;

		Transform& Multiply(const Transform& transformA, const Transform& transformB);

		Vector operator*(const Vector& vector);

		Matrix3x3 matrix;
		Vector translation;
	};

	Transform operator*(const Transform& transformA, const Transform& transformB);
}
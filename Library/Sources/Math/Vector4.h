#pragma once

#include "../Common.h"

namespace MeshNinja
{
	class Vector3;

	class MESH_NINJA_API Vector4
	{
	public:
		Vector4();
		Vector4(double x, double y, double z, double w);
		Vector4(const Vector3& vector, double w);
		Vector4(const Vector4& vector);
		virtual ~Vector4();

		bool operator==(const Vector4& vector);
		void operator=(const Vector4& vector);
		void operator+=(const Vector4& vector);
		void operator-=(const Vector4& vector);
		void operator*=(double scale);
		void operator/=(double scale);
		Vector4 operator*(double scale);
		Vector4 operator-() const;

		void SetComponents(double x, double y, double z, double w);
		void GetComponents(double& x, double& y, double& z, double& w) const;
		double Length() const;
		bool Normalize(double* returnedLength = nullptr);
		Vector4 Normalized() const;
		bool Homogenize();
		Vector4 Homogenized() const;

		double x, y, z, w;
	};

	Vector4 MESH_NINJA_API operator+(const Vector4& vectorA, const Vector4& vectorB);
	Vector4 MESH_NINJA_API operator-(const Vector4& vectorA, const Vector4& vectorB);
	Vector4 MESH_NINJA_API operator*(const Vector4& vector, double scale);
	Vector4 MESH_NINJA_API operator*(double scale, const Vector4& vector);
	Vector4 MESH_NINJA_API operator/(const Vector4& vector, double scale);
}
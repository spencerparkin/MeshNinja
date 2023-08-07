#pragma once

#include "../Common.h"

namespace MeshNinja
{
	class Line;
	class Plane;
	class Quaternion;

	class MESH_NINJA_API Vector3
	{
	public:
		Vector3();
		Vector3(double x, double y, double z);
		Vector3(const Vector3& vector);
		Vector3(const Quaternion& quat);
		virtual ~Vector3();

		bool operator==(const Vector3& vector);
		void operator=(const Vector3& vector);
		void operator+=(const Vector3& vector);
		void operator-=(const Vector3& vector);
		void operator*=(double scale);
		void operator/=(double scale);
		Vector3 operator*(double scale);
		Vector3 operator-() const;

		void SetComponents(double x, double y, double z);
		void GetComponents(double& x, double& y, double& z) const;
		double Length() const;
		bool Normalize(double* returnedLength = nullptr);
		Vector3 Normalized() const;
		double Dot(const Vector3& vector) const;
		Vector3 Cross(const Vector3& vector) const;
		static double Dot(const Vector3& vectorA, const Vector3& vectorB);
		Vector3& Cross(const Vector3& vectorA, const Vector3& vectorB);
		bool ProjectOnto(const Vector3& vector);
		bool RejectFrom(const Vector3& vector);
		bool RotateAbout(const Vector3& vector, double angle);
		bool Intersect(const Line& line, const Plane& plane);
		bool ToString(std::string& str) const;
		bool FromString(const std::string& str);
		bool MakeOrthogonalTo(const Vector3& vector);
		bool IsEqualTo(const Vector3& vector, double eps = MESH_NINJA_EPS) const;
		double AngleBetweenThisAnd(const Vector3& vector) const;
		void Max(const Vector3& vectorA, const Vector3& vectorB);
		void Min(const Vector3& vectorA, const Vector3& vectorB);
		unsigned int ToColor() const;
		void FromColor(unsigned int color);

		double x, y, z;
	};

	Vector3 MESH_NINJA_API operator+(const Vector3& vectorA, const Vector3& vectorB);
	Vector3 MESH_NINJA_API operator-(const Vector3& vectorA, const Vector3& vectorB);
	Vector3 MESH_NINJA_API operator*(const Vector3& vector, double scale);
	Vector3 MESH_NINJA_API operator*(double scale, const Vector3& vector);
	Vector3 MESH_NINJA_API operator/(const Vector3& vector, double scale);
	bool MESH_NINJA_API operator<(const Vector3& vectorA, const Vector3& vectorB);
}
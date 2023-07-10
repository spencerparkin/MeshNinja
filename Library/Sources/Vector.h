#pragma once

#include "Common.h"

namespace MeshNinja
{
	class Line;
	class Plane;
	class Quaternion;

	class MESH_NINJA_API Vector
	{
	public:
		Vector();
		Vector(double x, double y, double z);
		Vector(const Vector& vector);
		Vector(const Quaternion& quat);
		virtual ~Vector();

		bool operator==(const Vector& vector);
		void operator=(const Vector& vector);
		void operator+=(const Vector& vector);
		void operator-=(const Vector& vector);
		void operator*=(double scale);
		void operator/=(double scale);
		Vector operator*(double scale);
		Vector operator-() const;

		double Length() const;
		bool Normalize(double* returnedLength = nullptr);
		Vector Normalized() const;
		double Dot(const Vector& vector) const;
		Vector Cross(const Vector& vector) const;
		bool ProjectOnto(const Vector& vector);
		bool RejectFrom(const Vector& vector);
		bool RotateAbout(const Vector& vector, double angle);
		bool Intersect(const Line& line, const Plane& plane);
		bool ToString(std::string& str) const;
		bool FromString(const std::string& str);
		bool MakeOrthogonalTo(const Vector& vector);
		bool IsEqualTo(const Vector& vector, double eps = MESH_NINJA_EPS) const;
		double AngleBetweenThisAnd(const Vector& vector) const;

		double x, y, z;
	};

	Vector operator+(const Vector& vectorA, const Vector& vectorB);
	Vector operator-(const Vector& vectorA, const Vector& vectorB);
	Vector operator*(const Vector& vector, double scale);
	Vector operator*(double scale, const Vector& vector);
	Vector operator/(const Vector& vector, double scale);
	bool operator<(const Vector& vectorA, const Vector& vectorB);
}
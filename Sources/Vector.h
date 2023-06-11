#pragma once

#include "Common.h"

namespace MeshNinja
{
	class Line;
	class Plane;

	class MESH_NINJA_API Vector
	{
	public:
		Vector();
		Vector(double x, double y, double z);
		Vector(const Vector& vector);
		virtual ~Vector();

		bool operator==(const Vector& vector);
		void operator=(const Vector& vector);
		void operator+=(const Vector& vector);
		void operator-=(const Vector& vector);
		void operator*=(double scale);
		Vector operator*(double scale);

		double Length() const;
		bool Normalize(double* returnedLength = nullptr);
		double Dot(const Vector& vector) const;
		Vector Cross(const Vector& vector) const;
		bool ProjectOnto(const Vector& vector);
		bool RejectFrom(const Vector& vector);
		bool RotateAbout(const Vector& vector, double angle);
		bool Intersect(const Line& line, const Plane& plane);

		double x, y, z;
	};

	Vector operator+(const Vector& vectorA, const Vector& vectorB);
	Vector operator-(const Vector& vectorA, const Vector& vectorB);
	Vector operator*(const Vector& vector, double scale);
	Vector operator*(double scale, const Vector& vector);
}
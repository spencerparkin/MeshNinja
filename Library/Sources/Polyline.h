#pragma once

#include "Common.h"
#include "Math/Vector3.h"

namespace MeshNinja
{
	class ConvexPolygonMesh;

	class MESH_NINJA_API Polyline
	{
	public:
		Polyline();
		Polyline(const Polyline& polyline);
		Polyline(const Vector3& vertexA, const Vector3& vertexB);
		virtual ~Polyline();

		const Vector3& GetFirstVertex() const;
		const Vector3& GetLastVertex() const;

		bool IsLineLoop(double eps = MESH_NINJA_EPS) const;
		void ReverseOrder();
		bool Merge(const Polyline& polylineA, const Polyline& polylineB, double eps = MESH_NINJA_EPS);
		void Concatinate(const Polyline& polylineA, const Polyline& polylineB);
		bool GenerateTubeMesh(ConvexPolygonMesh& tubeMesh, double radius, int numSides) const;

		std::vector<Vector3>* vertexArray;
	};
}
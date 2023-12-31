#pragma once

#include "Common.h"
#include "Math/Vector3.h"
#include "Math/Transform.h"
#include "ConvexPolygon.h"

namespace MeshNinja
{
	// Each face is assumed to be a convex polygon.  If this is not the case, then
	// we simply leave the results of all algorithms as undefined.
	class MESH_NINJA_API ConvexPolygonMesh
	{
	public:
		ConvexPolygonMesh();
		ConvexPolygonMesh(const ConvexPolygonMesh& mesh);
		ConvexPolygonMesh(const std::vector<ConvexPolygon>& polygonArray);
		virtual ~ConvexPolygonMesh();

		enum class Polyhedron
		{
			TETRAHEDRON,
			OCTAHEDRON,
			HEXADRON,
			ICOSAHEDRON,
			DODECAHEDRON,
			ICOSIDODECAHEDRON,
			CUBOCTAHEDRON,
			RHOMBICOSIDODECAHEDRON
		};

		void Clear();
		bool AllFacetsValid(double eps = MESH_NINJA_EPS) const;
		bool IsConvex(double eps = MESH_NINJA_EPS) const;
		bool IsConcave(double eps = MESH_NINJA_EPS) const;
		void Compress(double eps = MESH_NINJA_EPS);
		void NormalizeEdges(double eps = MESH_NINJA_EPS);
		void UntessellateFaces(double eps = MESH_NINJA_EPS);
		void TessellateFaces(double eps = MESH_NINJA_EPS);
		void ToConvexPolygonArray(std::vector<ConvexPolygon>& convexPolygonArray, bool concatinate = true) const;
		void FromConvexPolygonArray(const std::vector<ConvexPolygon>& convexPolygonArray);
		bool GenerateConvexHull(const std::vector<Vector3>& pointArray, double eps = MESH_NINJA_EPS);
		bool GeneratePolyhedron(Polyhedron polyhedron, double eps = MESH_NINJA_EPS);
		bool GenerateSphere(double radius, int segments, int slices);
		bool GenerateCylinder(double length, double radius, int segments, int slices);
		bool GenerateTorus(double innerRadius, double outerRadius, int segments, int slices);
		bool GenerateMobiusStrip(double width, double radius, int segments);
		bool GenerateKleinBottle(int segments);
		int FindClosestPointTo(const Vector3& point, double* smallestDistance = nullptr) const;
		void Copy(const ConvexPolygonMesh& mesh);
		Vector3 CalcCenter() const;
		void ApplyTransform(const Transform& transform);
		void ReverseAllPolygons();
		void CenterAndScale(double radius);
		bool AddRedundantVertex(const Vector3& vertex, double eps = MESH_NINJA_EPS);

		struct MESH_NINJA_API Triangle
		{
			int vertex[3];

			bool IsCanceledBy(const Triangle& triangle) const;
			void MakePolygon(ConvexPolygon& polygon, const std::vector<Vector3>& pointArray) const;
			void MakePlane(Plane& plane, const std::vector<Vector3>& pointArray) const;
		};

		class MESH_NINJA_API Facet
		{
		public:
			Facet();
			Facet(const Facet& facet);
			virtual ~Facet();

			struct AngleStats
			{
				double smallestInteriorAngle;
				double largestInteriorAngle;
			};

			void MakePolygon(ConvexPolygon& polygon, const ConvexPolygonMesh* mesh) const;
			bool Merge(const Facet& facetA, const Facet& facetB, const ConvexPolygonMesh* mesh);
			bool Split(Facet& facetA, Facet& facetB, const ConvexPolygonMesh* mesh) const;
			bool CalcInteriorAngleStats(AngleStats& angleStats, const ConvexPolygonMesh* mesh) const;
			bool HasVertex(int i) const;
			void Reverse();

			int operator[](int i) const		// TODO: Edit code to use this to make the code easier to read.
			{
				return (*this->vertexArray)[i];
			}

			std::vector<int>* vertexArray;
		};

		std::vector<Facet>* facetArray;
		std::vector<Vector3>* vertexArray;
	};
}
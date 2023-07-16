#pragma once

#include "Common.h"
#include "Vector.h"
#include "ConvexPolygon.h"

namespace MeshNinja
{
	// Each face is assumed to be a convex polygon.  If this is not the case, then
	// we simply leave the results of all algorithms as undefined.
	class MESH_NINJA_API ConvexPolygonMesh
	{
	public:
		ConvexPolygonMesh();
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
		bool IsConvex(double eps = MESH_NINJA_EPS) const;
		bool IsConcave(double eps = MESH_NINJA_EPS) const;
		void Compress(double eps = MESH_NINJA_EPS);
		void NormalizeEdges(double eps = MESH_NINJA_EPS);
		void UntessellateFaces(double eps = MESH_NINJA_EPS);
		void TessellateFaces(double eps = MESH_NINJA_EPS);
		void ToConvexPolygonArray(std::vector<ConvexPolygon>& convexPolygonArray) const;
		void FromConvexPolygonArray(const std::vector<ConvexPolygon>& convexPolygonArray);
		bool GenerateConvexHull(const std::vector<Vector>& pointArray, double eps = MESH_NINJA_EPS);
		bool GeneratePolyhedron(Polyhedron polyhedron, double eps = MESH_NINJA_EPS);
		int FindClosestPointTo(const Vector& point, double* smallestDistance = nullptr) const;
		void Copy(const ConvexPolygonMesh& mesh);

		struct MESH_NINJA_API Triangle
		{
			int vertex[3];

			bool IsCanceledBy(const Triangle& triangle) const;
			void MakePolygon(ConvexPolygon& polygon, const std::vector<Vector>& pointArray) const;
			void MakePlane(Plane& plane, const std::vector<Vector>& pointArray) const;
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

			int operator[](int i) const		// TODO: Edit code to use this to make the code easier to read.
			{
				return (*this->vertexArray)[i];
			}

			std::vector<int>* vertexArray;
		};

		std::vector<Facet>* facetArray;
		std::vector<Vector>* vertexArray;
	};
}
#pragma once

#include "MeshBinaryOperation.h"
#include "ConvexPolygon.h"
#include "Polyline.h"

namespace MeshNinja
{
	class ConvexPolygonMesh;

	class MESH_NINJA_API MeshSetOperation : public MeshBinaryOperation
	{
	public:
		MeshSetOperation();
		virtual ~MeshSetOperation();

	protected:

		struct PolygonLists
		{
			std::vector<ConvexPolygon> meshA_outsidePolygonList;
			std::vector<ConvexPolygon> meshA_insidePolygonList;
			std::vector<ConvexPolygon> meshB_outsidePolygonList;
			std::vector<ConvexPolygon> meshB_insidePolygonList;
		};

		bool CalculatePolygonLists(const ConvexPolygonMesh& meshA, const ConvexPolygonMesh& meshB, PolygonLists& polygonLists);

		class PolylineCollection
		{
		public:
			PolylineCollection();
			virtual ~PolylineCollection();

			void AddPolyline(const Polyline& givenPolyline);

			std::list<Polyline> polylineList;
		};
	};

	class MESH_NINJA_API MeshUnion : public MeshSetOperation
	{
	public:
		MeshUnion();
		virtual ~MeshUnion();

		virtual bool Perform(const ConvexPolygonMesh& meshA, const ConvexPolygonMesh& meshB, ConvexPolygonMesh& resultingMesh) override;
	};

	class MESH_NINJA_API MeshIntersection : public MeshSetOperation
	{
	public:
		MeshIntersection();
		virtual ~MeshIntersection();

		virtual bool Perform(const ConvexPolygonMesh& meshA, const ConvexPolygonMesh& meshB, ConvexPolygonMesh& resultingMesh) override;
	};

	class MESH_NINJA_API MeshSubtraction : public MeshSetOperation
	{
	public:
		MeshSubtraction();
		virtual ~MeshSubtraction();

		virtual bool Perform(const ConvexPolygonMesh& meshA, const ConvexPolygonMesh& meshB, ConvexPolygonMesh& resultingMesh) override;
	};
}
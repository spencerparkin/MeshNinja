#pragma once

#include "MeshBinaryOperation.h"
#include "ConvexPolygon.h"
#include "Polyline.h"
#include "MeshGraph.h"

namespace MeshNinja
{
	class ConvexPolygonMesh;
	class LineSegment;

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

		void ChopupPolygonArray(std::vector<ConvexPolygon>& polygonArray, const std::vector<LineSegment>& lineSegmentArray);
		bool ChopupPolygon(const ConvexPolygon& polygon, ConvexPolygon& polygonA, ConvexPolygon& polygonB, const std::vector<LineSegment>& lineSegmentArray);

		class Node : public MeshGraph::Node
		{
		public:
			Node();
			virtual ~Node();

			enum class Side
			{
				UNKNOWN,
				INSIDE,
				OUTSIDE
			};

			Side side;
		};

		class Edge : public MeshGraph::Edge
		{
		public:
			Edge();
			virtual ~Edge();

			enum class Type
			{
				UNKNOWN,
				NORMAL,
				CUT_BOUNDARY
			};

			Type type;
		};

		class Graph : public MeshGraph
		{
		public:
			Graph();
			virtual ~Graph();

			virtual Node* CreateNode() override;
			virtual Edge* CreateEdge() override;

			bool ColorEdges(const std::vector<LineSegment>& lineSegmentArray);
			bool ColorNodes(const Graph* otherGraph);

			MeshSetOperation::Node* FindInitialOutsideNode(const Graph* otherGraph);

			void PopulatePolygonLists(std::vector<ConvexPolygon>& insidePolygonList, std::vector<ConvexPolygon>& outsidePolygonList) const;
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
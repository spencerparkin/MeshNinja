#pragma once

#include "Common.h"
#include "ConvexPolygonMesh.h"

namespace MeshNinja
{
	// Mesh graphs make it easier to traverse a given convex polygon mesh using a
	// breadth-first search or depth-first search or whatever.
	class MESH_NINJA_API MeshGraph
	{
	public:
		MeshGraph();
		virtual ~MeshGraph();

		class Node;
		class Edge;

		void Clear();
		bool Generate(const ConvexPolygonMesh& givenMesh);
		bool GenerateDual(ConvexPolygonMesh& mesh) const;

		virtual Node* CreateNode();
		virtual Edge* CreateEdge();

		class Node
		{
		public:
			Node();
			virtual ~Node();

			const ConvexPolygonMesh::Facet* facet;
			std::vector<Edge*> edgeArray;
		};

		struct VertexPair
		{
			int i, j;

			uint64_t CalcKey() const;
		};

		class Edge
		{
		public:
			Edge();
			virtual ~Edge();

			Node* Fallow(Node* origin);

			VertexPair pair;
			Node* node[2];
		};

		std::vector<Node*>* nodeArray;
		std::vector<Edge*>* edgeArray;

		const ConvexPolygonMesh* mesh;
	};

	bool MESH_NINJA_API operator<(const MeshGraph::VertexPair& pairA, const MeshGraph::VertexPair& pairB);
}
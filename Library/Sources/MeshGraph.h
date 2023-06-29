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

		void Clear();
		void Generate(const ConvexPolygonMesh& mesh);
		virtual Node* CreateNode();

		struct Adjacency
		{
			int i;
			Node* node;
		};

		class Node
		{
		public:
			Node();
			virtual ~Node();

			const ConvexPolygonMesh::Facet* facet;
			std::vector<Adjacency> adjacencyArray;
		};

		struct Edge
		{
			int i, j;

			uint64_t CalcKey() const;
		};

	protected:

		std::vector<Node*>* nodeArray;
	};

	bool operator<(const MeshGraph::Edge& edgeA, const MeshGraph::Edge& edgeB);
}
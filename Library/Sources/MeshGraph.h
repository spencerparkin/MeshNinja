#pragma once

#include "Common.h"
#include "ConvexPolygonMesh.h"
#include "DebugDraw.h"

namespace MeshNinja
{
	class Transform;

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
		bool GenerateDual(ConvexPolygonMesh& dualMesh) const;

		virtual void GenerateDebugDrawObjects(DebugDraw& debugDraw) const;
		virtual Node* CreateNode();
		virtual Edge* CreateEdge();

		class Node
		{
		public:
			Node();
			virtual ~Node();

			virtual Vector3 GetDebugColor() const;

			const ConvexPolygonMesh::Facet* facet;
			std::vector<Edge*> edgeArray;
		};

		template<bool ordered>
		struct VertexPair
		{
			int i, j;

			uint64_t CalcKey() const
			{
				if constexpr (!ordered)
				{
					if (this->i < this->j)
						return uint64_t(this->i) | (uint64_t(this->j) << 32);
				}

				return uint64_t(this->j) | (uint64_t(this->i) << 32);
			}
		};

		class Edge
		{
		public:
			Edge();
			virtual ~Edge();

			virtual Vector3 GetDebugColor() const;

			Node* Fallow(Node* origin);

			VertexPair<false> pair;
			Node* node[2];
		};

		void CollectSilhouetteEdges(const Vector3& viewPoint, std::set<VertexPair<false>>& edgeSet, const Transform& transform) const;

		std::vector<Node*>* nodeArray;
		std::vector<Edge*>* edgeArray;

		const ConvexPolygonMesh* mesh;
	};

	template<bool ordered>
	bool operator<(const MeshGraph::VertexPair<ordered>& pairA, const MeshGraph::VertexPair<ordered>& pairB)
	{
		uint64_t keyA = pairA.CalcKey();
		uint64_t keyB = pairB.CalcKey();

		return keyA < keyB;
	}
}
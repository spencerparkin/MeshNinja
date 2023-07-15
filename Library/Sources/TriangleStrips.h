#pragma once

#include "Common.h"
#include "MeshGraph.h"

namespace MeshNinja
{
	class ConvexPolygonMesh;

	class MESH_NINJA_API TriangleStrips
	{
	public:
		TriangleStrips();
		virtual ~TriangleStrips();

		bool Generate(const ConvexPolygonMesh* mesh);

		typedef std::vector<int> TriStripArray;
		std::list<TriStripArray>* stripsList;
		const ConvexPolygonMesh* mesh;

	private:

		class Node : public MeshGraph::Node
		{
		public:
			Node();
			virtual ~Node();

			bool visited;
		};

		class Graph : public MeshGraph
		{
		public:
			Graph();
			virtual ~Graph();

			virtual Node* CreateNode() override;

			TriangleStrips::Node* FindUnvisitedNode();
		};
	};
}
#include "TriangleStrips.h"
#include "ConvexPolygonMesh.h"
#include "MeshGraph.h"

using namespace MeshNinja;

//----------------------------------- TriangleStrips -----------------------------------

TriangleStrips::TriangleStrips()
{
	this->mesh = nullptr;
	this->stripsList = new std::list<TriStripArray>();
}

/*virtual*/ TriangleStrips::~TriangleStrips()
{
	delete this->stripsList;
}

bool TriangleStrips::Generate(const ConvexPolygonMesh* mesh)
{
	this->stripsList->clear();

	// Cache the mesh pointer since it's part of the definition of a set of triangle strips.
	this->mesh = mesh;

	// Verify that the given mesh is a triangle mesh.
	for (const ConvexPolygonMesh::Facet& facet : *mesh->facetArray)
		if (facet.vertexArray->size() != 3)
			return false;

	// Generate a graph of the mesh that we can traverse to build the strips.
	Graph graph;
	if (!graph.Generate(*mesh))
		return false;

	// Note that this algorithm will work, but it won't necessarily find a minimal set of
	// triangle strips for the given mesh.  That is an interesting problem, and not immediately
	// obvious to me.
	while (true)
	{
		// We're done if we've visited all triangles.
		Node* node = graph.FindUnvisitedNode();
		if (!node)
			break;

		// Generate a sequence of adjacent triangles from the found node.
		// Always making left turns if possible is our stratagy for minimizing the
		// number of strips we'll ultimately generate.
		std::vector<const ConvexPolygonMesh::Facet*> facetSequence;

		struct Entry
		{
			Node* curNode;
			Node* prevNode;
		};

		auto searchForAdjacency = [](Node* node, std::function<bool(Node*, int)> predicate) -> Node*
		{
			for (int i = 0; i < (signed)node->edgeArray.size(); i++)
			{
				Node* adjacentNode = (Node*)node->edgeArray[i]->Fallow(node);
				if (predicate(adjacentNode, i))
					return adjacentNode;
			}
			return nullptr;
		};

		// Traverse the mesh, always making left turns, if possible.
		std::list<Entry> queue;
		queue.push_back(Entry{ node, nullptr });
		while (queue.size() > 0)
		{
			std::list<Entry>::iterator iter = queue.begin();
			Entry& entry = *iter;
			queue.erase(iter);

			entry.curNode->visited = true;
			facetSequence.push_back(entry.curNode->facet);

			if (entry.curNode->edgeArray.size() > 0)
			{
				Entry newEntry{ nullptr, entry.curNode };

				Graph::VertexPair traveledEdge{ -1, -1 };
				if (entry.prevNode)
				{
					for (int i = 0; i < (signed)entry.curNode->edgeArray.size(); i++)
					{
						if (entry.curNode->edgeArray[i]->Fallow(entry.curNode) == entry.prevNode)
						{
							traveledEdge = entry.curNode->edgeArray[i]->pair;
							break;
						}
					}

					assert(traveledEdge.i != -1 && traveledEdge.j != -1);
				}

				if (!entry.prevNode)
				{
					// Go any direction.  It doesn't matter.
					newEntry.curNode = searchForAdjacency(entry.curNode, [](Node* adjacentNode, int i) -> bool
						{
							return !adjacentNode->visited;
						});
				}
				else
				{
					// Try to turn left.
					newEntry.curNode = searchForAdjacency(entry.curNode, [&traveledEdge](Node* adjacentNode, int i) -> bool
						{
							return !adjacentNode->visited && adjacentNode->edgeArray[i]->pair.j == traveledEdge.i;
						});

					// If we can't turn left, then try to turn right.
					if (!newEntry.curNode)
					{
						newEntry.curNode = searchForAdjacency(entry.curNode, [&traveledEdge](Node* adjacentNode, int i) -> bool
							{
								return !adjacentNode->visited && adjacentNode->edgeArray[i]->pair.i == traveledEdge.j;
							});
					}
				}

				if (newEntry.curNode)
					queue.push_back(newEntry);
			}
		}

		// Now that we have a sequence of adjacent nodes, we're ready to build a tri-strip.
		this->stripsList->push_back(TriStripArray());
		TriStripArray& triStripArray = this->stripsList->back();
		assert(facetSequence.size() > 0);
		if (facetSequence.size() == 1)
		{
			const ConvexPolygonMesh::Facet* facet = facetSequence[0];
			triStripArray.push_back((*facet->vertexArray)[0]);
			triStripArray.push_back((*facet->vertexArray)[1]);
			triStripArray.push_back((*facet->vertexArray)[2]);
		}
		else
		{
			const ConvexPolygonMesh::Facet* facetA = facetSequence[0];
			const ConvexPolygonMesh::Facet* facetB = facetSequence[1];

			for (int i = 0; i < 3; i++)
			{
				if (!facetB->HasVertex((*facetA->vertexArray)[i]))
				{
					for (int j = 0; j < 3; j++)
						triStripArray.push_back((*facetA->vertexArray)[(i + j) % 3]);
					break;
				}
			}

			for (int i = 0; i < (signed)facetSequence.size() - 1; i++)
			{
				facetA = facetSequence[i];
				facetB = facetSequence[i + 1];

				for (int j = 0; j < 3; j++)
				{
					if (!facetA->HasVertex((*facetB->vertexArray)[j]))
					{
						triStripArray.push_back((*facetB->vertexArray)[j]);
						break;
					}
				}
			}
		}
	}

	return true;
}

//----------------------------------- TriangleStrips::Node -----------------------------------

TriangleStrips::Node::Node()
{
	this->visited = false;
}

/*virtual*/ TriangleStrips::Node::~Node()
{
}

//----------------------------------- TriangleStrips::Graph -----------------------------------

TriangleStrips::Graph::Graph()
{
}

/*virtual*/ TriangleStrips::Graph::~Graph()
{
}

/*virtual*/ MeshGraph::Node* TriangleStrips::Graph::CreateNode()
{
	return new TriangleStrips::Node();
}

TriangleStrips::Node* TriangleStrips::Graph::FindUnvisitedNode()
{
	for (MeshGraph::Node* node : *this->nodeArray)
		if (!((TriangleStrips::Node*)node)->visited)
			return (TriangleStrips::Node*)node;
	
	return nullptr;
}
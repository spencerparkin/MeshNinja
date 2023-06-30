#include "MeshGraph.h"

using namespace MeshNinja;

//----------------------------------- MeshGraph -----------------------------------

MeshGraph::MeshGraph()
{
	this->nodeArray = new std::vector<Node*>();
	this->mesh = nullptr;
}

/*virtual*/ MeshGraph::~MeshGraph()
{
	this->Clear();

	delete this->nodeArray;
}

void MeshGraph::Clear()
{
	for (Node* node : *this->nodeArray)
		delete node;

	this->nodeArray->clear();
}

void MeshGraph::Generate(const ConvexPolygonMesh& givenMesh)
{
	this->Clear();

	this->mesh = &givenMesh;

	for (const ConvexPolygonMesh::Facet& facet : *this->mesh->facetArray)
	{
		Node* node = this->CreateNode();
		node->facet = &facet;
		this->nodeArray->push_back(node);
	}

	std::map<Edge, Node*> nodeMap;

	// We're assuming here that an edge is shared by at most two polygons.
	for (Node* nodeA : *this->nodeArray)
	{
		for (int i = 0; i < (signed)nodeA->facet->vertexArray.size(); i++)
		{
			int j = (i + 1) % nodeA->facet->vertexArray.size();
			Edge edge{ nodeA->facet->vertexArray[i], nodeA->facet->vertexArray[j] };

			std::map<Edge, Node*>::iterator iter = nodeMap.find(edge);
			if (iter == nodeMap.end())
				nodeMap.insert(std::pair<Edge, Node*>(edge, nodeA));
			else
			{
				Node* nodeB = iter->second;
				
				Adjacency adjacencyA{ i, nodeB };
				nodeA->adjacencyArray.push_back(adjacencyA);

				int k;
				for (k = 0; k < (signed)nodeB->facet->vertexArray.size(); k++)
					if (nodeB->facet->vertexArray[k] == nodeA->facet->vertexArray[j])
						break;

				assert(k < (signed)nodeB->facet->vertexArray.size());

				Adjacency adjacencyB{ k, nodeA };
				nodeB->adjacencyArray.push_back(adjacencyB);
			}
		}
	}
}

/*virtual*/ MeshGraph::Node* MeshGraph::CreateNode()
{
	return new Node();
}

//----------------------------------- MeshGraph::Edge -----------------------------------

uint64_t MeshGraph::Edge::CalcKey() const
{
	if (this->i < this->j)
		return uint64_t(this->i) | (uint64_t(this->j) << 32);

	return uint64_t(this->j) | (uint64_t(this->i) << 32);
}

//----------------------------------- MeshGraph::Node -----------------------------------

MeshGraph::Node::Node()
{
	this->facet = nullptr;
}

/*virtual*/ MeshGraph::Node::~Node()
{
}

namespace MeshNinja
{
	bool operator<(const MeshGraph::Edge& edgeA, const MeshGraph::Edge& edgeB)
	{
		uint64_t keyA = edgeA.CalcKey();
		uint64_t keyB = edgeB.CalcKey();

		return keyA < keyB;
	}
}
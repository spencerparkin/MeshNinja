#include "MeshGraph.h"

using namespace MeshNinja;

//----------------------------------- MeshGraph -----------------------------------

MeshGraph::MeshGraph()
{
	this->nodeArray = new std::vector<Node*>();
	this->edgeArray = new std::vector<Edge*>();

	this->mesh = nullptr;
}

/*virtual*/ MeshGraph::~MeshGraph()
{
	this->Clear();

	delete this->nodeArray;
	delete this->edgeArray;
}

void MeshGraph::Clear()
{
	for (Node* node : *this->nodeArray)
		delete node;

	for (Edge* edge : *this->edgeArray)
		delete edge;

	this->nodeArray->clear();
	this->edgeArray->clear();
}

bool MeshGraph::Generate(const ConvexPolygonMesh& givenMesh)
{
	this->Clear();

	this->mesh = &givenMesh;

	for (const ConvexPolygonMesh::Facet& facet : *this->mesh->facetArray)
	{
		Node* node = this->CreateNode();
		node->facet = &facet;
		this->nodeArray->push_back(node);
	}

	std::map<VertexPair, Node*> nodeMap;

	// We're assuming here that an edge is shared by at most two polygons.
	for (Node* nodeA : *this->nodeArray)
	{
		for (int i = 0; i < (signed)nodeA->facet->vertexArray->size(); i++)
		{
			int j = (i + 1) % nodeA->facet->vertexArray->size();

			VertexPair pair;

			pair.i = (*nodeA->facet->vertexArray)[i];
			pair.j = (*nodeA->facet->vertexArray)[j];

			std::map<VertexPair, Node*>::iterator iter = nodeMap.find(pair);
			if (iter == nodeMap.end())
				nodeMap.insert(std::pair<VertexPair, Node*>(pair, nodeA));
			else
			{
				Node* nodeB = iter->second;

				Edge* edge = this->CreateEdge();
				this->edgeArray->push_back(edge);

				edge->pair = pair;

				nodeA->edgeArray.push_back(edge);
				nodeB->edgeArray.push_back(edge);

				edge->node[0] = nodeA;
				edge->node[1] = nodeB;
			}
		}
	}

	return true;
}

/*virtual*/ MeshGraph::Node* MeshGraph::CreateNode()
{
	return new Node();
}

/*virtual*/ MeshGraph::Edge* MeshGraph::CreateEdge()
{
	return new Edge();
}

//----------------------------------- MeshGraph::VertexPair -----------------------------------

uint64_t MeshGraph::VertexPair::CalcKey() const
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

//----------------------------------- MeshGraph::Edge -----------------------------------

MeshGraph::Edge::Edge()
{
	this->pair = VertexPair{ -1, -1 };
	this->node[0] = nullptr;
	this->node[1] = nullptr;
}

/*virtual*/ MeshGraph::Edge::~Edge()
{
}

MeshGraph::Node* MeshGraph::Edge::Fallow(Node* origin)
{
	return (origin == this->node[0]) ? this->node[1] : this->node[0];
}

namespace MeshNinja
{
	bool operator<(const MeshGraph::VertexPair& pairA, const MeshGraph::VertexPair& pairB)
	{
		uint64_t keyA = pairA.CalcKey();
		uint64_t keyB = pairB.CalcKey();

		return keyA < keyB;
	}
}
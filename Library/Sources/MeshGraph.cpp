#include "MeshGraph.h"
#include "Plane.h"
#include "Math/Matrix4x4.h"

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

	std::map<VertexPair<false>, Node*> nodeMap;

	// We're assuming here that an edge is shared by at most two polygons.
	for (Node* nodeA : *this->nodeArray)
	{
		for (int i = 0; i < (signed)nodeA->facet->vertexArray->size(); i++)
		{
			int j = (i + 1) % nodeA->facet->vertexArray->size();

			VertexPair<false> pair;

			pair.i = (*nodeA->facet->vertexArray)[i];
			pair.j = (*nodeA->facet->vertexArray)[j];

			std::map<VertexPair<false>, Node*>::iterator iter = nodeMap.find(pair);
			if (iter == nodeMap.end())
				nodeMap.insert(std::pair<VertexPair<false>, Node*>(pair, nodeA));
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

void MeshGraph::CollectSilhouetteEdges(const Vector3& viewDirection, std::set<VertexPair<false>>& edgeSet, const Matrix4x4& matrix) const
{
	edgeSet.clear();

	for (const Edge* edge : *this->edgeArray)
	{
		Node* nodeA = edge->node[0];
		Node* nodeB = edge->node[1];

		ConvexPolygon polygonA, polygonB;

		nodeA->facet->MakePolygon(polygonA, this->mesh);
		nodeB->facet->MakePolygon(polygonB, this->mesh);

		matrix.TransformPolygon(polygonA);
		matrix.TransformPolygon(polygonB);

		Plane planeA, planeB;

		polygonA.CalcPlane(planeA);
		polygonB.CalcPlane(planeB);

		double dotA = viewDirection.Dot(planeA.normal);
		double dotB = viewDirection.Dot(planeB.normal);

		double signA = MESH_NINJA_SIGN(dotA);
		double signB = MESH_NINJA_SIGN(dotB);

		if (signA != signB)
			edgeSet.insert(edge->pair);
	}
}

bool MeshGraph::GenerateDual(ConvexPolygonMesh& dualMesh) const
{
	dualMesh.Clear();

	std::map<const ConvexPolygonMesh::Facet*, int> vertexMap;

	for (const Node* node : *this->nodeArray)
	{
		ConvexPolygon polygon;
		node->facet->MakePolygon(polygon, this->mesh);
		vertexMap.insert(std::pair<const ConvexPolygonMesh::Facet*, int>(node->facet, dualMesh.vertexArray->size()));
		dualMesh.vertexArray->push_back(polygon.CalcCenter());
	}

	std::list<VertexPair<true>> directedEdgeQueue;

	for (const Edge* edge : *this->edgeArray)
	{
		int i = vertexMap.find(edge->node[0]->facet)->second;
		int j = vertexMap.find(edge->node[1]->facet)->second;
		
		directedEdgeQueue.push_back(VertexPair<true>{ i, j });
		directedEdgeQueue.push_back(VertexPair<true>{ j, i });
	}

	while (directedEdgeQueue.size() > 0)
	{
		std::list<VertexPair<true>>::iterator iter = directedEdgeQueue.begin();
		VertexPair<true> pairA = *iter;
		directedEdgeQueue.erase(iter);

		ConvexPolygonMesh::Facet facet;

		while (true)
		{
			facet.vertexArray->push_back(pairA.i);
			if (facet.HasVertex(pairA.j))
				break;

			double smallestTurnAngle = DBL_MAX;
			std::list<VertexPair<true>>::iterator chosenIter = directedEdgeQueue.end();
			for (iter = directedEdgeQueue.begin(); iter != directedEdgeQueue.end(); iter++)
			{
				VertexPair<true> pairB = *iter;
				if (pairA.j == pairB.i && pairA.i != pairB.j)
				{
					const Vector3& pointA = (*dualMesh.vertexArray)[pairA.i];
					const Vector3& pointB = (*dualMesh.vertexArray)[pairA.j];
					const Vector3& pointC = (*dualMesh.vertexArray)[pairB.j];

					Vector3 vectorAB, vectorBC;

					vectorAB = pointB - pointA;
					vectorBC = pointC - pointB;

					double turnAngle = vectorAB.AngleBetweenThisAnd(vectorBC);

					if (vectorAB.Cross(vectorBC).Dot(pointB) < 0.0)
						turnAngle = MESH_NINJA_PI + turnAngle;
					else
						turnAngle = MESH_NINJA_PI - turnAngle;

					if (turnAngle < smallestTurnAngle)
					{
						smallestTurnAngle = turnAngle;
						chosenIter = iter;
					}
				}
			}

			if (chosenIter == directedEdgeQueue.end())
				return false;

			pairA = *chosenIter;
			directedEdgeQueue.erase(chosenIter);
		}

		dualMesh.facetArray->push_back(facet);
	}

	if (!dualMesh.AllFacetsValid())
		return false;

	return true;
}

/*virtual*/ void MeshGraph::GenerateDebugDrawObjects(DebugDraw& debugDraw) const
{
	for (const Node* node : *this->nodeArray)
	{
		ConvexPolygon polygon;
		node->facet->MakePolygon(polygon, this->mesh);

		DebugDraw::Point* point = debugDraw.New<DebugDraw::Point>();
		point->vertex = polygon.CalcCenter();
		point->color = node->GetDebugColor();
		debugDraw.AddObject(point);

		for (const Edge* edge : node->edgeArray)
		{
			const Vector3& pointA = (*this->mesh->vertexArray)[edge->pair.i];
			const Vector3& pointB = (*this->mesh->vertexArray)[edge->pair.j];

			DebugDraw::Line* line = debugDraw.New<DebugDraw::Line>();
			line->vertex[0] = point->vertex;
			line->vertex[1] = (pointA + pointB) / 2.0;
			line->color = edge->GetDebugColor();
			debugDraw.AddObject(line);
		}
	}
}

//----------------------------------- MeshGraph::Node -----------------------------------

MeshGraph::Node::Node()
{
	this->facet = nullptr;
}

/*virtual*/ MeshGraph::Node::~Node()
{
}

/*virtual*/ Vector3 MeshGraph::Node::GetDebugColor() const
{
	return Vector3(1.0, 1.0, 1.0);
}

//----------------------------------- MeshGraph::Edge -----------------------------------

MeshGraph::Edge::Edge()
{
	this->pair = VertexPair<false>{ -1, -1 };
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

/*virtual*/ Vector3 MeshGraph::Edge::GetDebugColor() const
{
	return Vector3(0.5, 0.5, 0.5);
}
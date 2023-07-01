#include "MeshSetOperation.h"
#include "ConvexPolygonMesh.h"
#include "LineSegment.h"
#include "Plane.h"
#include "Ray.h"
#if defined MESH_NINJA_DEBUG_MESH_SET_OPERATION
#	include "MeshFileFormat.h"
#endif

using namespace MeshNinja;

//----------------------------------- MeshSetOperation -----------------------------------

MeshSetOperation::MeshSetOperation()
{
}

/*virtual*/ MeshSetOperation::~MeshSetOperation()
{
}

bool MeshSetOperation::CalculatePolygonLists(const ConvexPolygonMesh& meshA, const ConvexPolygonMesh& meshB, PolygonLists& polygonLists)
{
#if defined MESH_NINJA_DEBUG_MESH_SET_OPERATION
	ObjFileFormat objFileFormat;
#endif //MESH_NINJA_DEBUG_MESH_SET_OPERATION

	std::vector<ConvexPolygon> polygonArrayA, polygonArrayB;

	meshA.ToConvexPolygonArray(polygonArrayA);
	meshB.ToConvexPolygonArray(polygonArrayB);

	PolylineCollection polylineCollection;

	// Note that for large meshes, this O(N^2) algorithm could be reduced to
	// O(N log N) if here we used a spatial partitioning data-structure.

	for (const ConvexPolygon& polygonA : polygonArrayA)
	{
		for (const ConvexPolygon& polygonB : polygonArrayB)
		{
			ConvexPolygon intersection;
			if (intersection.Intersect(polygonA, polygonB))
			{
				if (intersection.vertexArray->size() > 2)
				{
					// It is certainly possible to define the intersection, union and subtraction of meshes in
					// the case that there exists a non-trivial intersection, but it's not a case that I'm going
					// to try to support with this algorithm, and I don't think that failing to do so results in
					// any major limitation; at least, for the applications I can think of.
					*this->error = "Non-trivial intersection between two polygons encountered.";

#if defined MESH_NINJA_DEBUG_MESH_SET_OPERATION
					ConvexPolygonMesh debugMesh;
					std::vector<ConvexPolygon> debugPolygonArray;
					debugPolygonArray.push_back(polygonA);
					debugMesh.FromConvexPolygonArray(debugPolygonArray);
					objFileFormat.SaveMesh("Meshes/DebugMeshA.obj", debugMesh);
					debugPolygonArray.clear();
					debugPolygonArray.push_back(polygonB);
					debugMesh.FromConvexPolygonArray(debugPolygonArray);
					objFileFormat.SaveMesh("Meshes/DebugMeshB.obj", debugMesh);
#endif //MESH_NINJA_DEBUG_MESH_SET_OPERATION

					return false;
				}
				else if (intersection.vertexArray->size() == 2)
				{
					Polyline polyline((*intersection.vertexArray)[0], (*intersection.vertexArray)[1]);
					polylineCollection.AddPolyline(polyline);
				}
			}
		}
	}

	for (const Polyline& polyline : polylineCollection.polylineList)
	{
		if (!polyline.IsLineLoop())
		{
			*this->error = "Did not find all line-loop cuts between the two given meshes.";
			return false;
		}
	}

#if defined MESH_NINJA_DEBUG_MESH_SET_OPERATION
	int fileCount = 0;
	for (const Polyline& polyline : polylineCollection.polylineList)
	{
		ConvexPolygonMesh tubeMesh;
		if (polyline.GenerateTubeMesh(tubeMesh, 0.05, 10))
			objFileFormat.SaveMesh(std::format("Meshes/polyline{}.obj", fileCount++), tubeMesh);
	}
#endif //MESH_NINJA_DEBUG_MESH_SET_OPERATION

	std::vector<LineSegment> lineSegmentArray;
	for (const Polyline& polyline : polylineCollection.polylineList)
	{
		for (int i = 0; i < (signed)polyline.vertexArray->size() - 1; i++)
		{
			const Vector& vertexA = (*polyline.vertexArray)[i];
			const Vector& vertexB = (*polyline.vertexArray)[i + 1];

			lineSegmentArray.push_back(LineSegment(vertexA, vertexB));
		}
	}

	this->ChopupPolygonArray(polygonArrayA, lineSegmentArray);
	this->ChopupPolygonArray(polygonArrayB, lineSegmentArray);

	ConvexPolygonMesh cutMeshA, cutMeshB;

	cutMeshA.FromConvexPolygonArray(polygonArrayA);
	cutMeshB.FromConvexPolygonArray(polygonArrayB);

	// This ensures that each edge is adjacent to at most two faces.
	cutMeshA.NormalizeEdges();
	cutMeshB.NormalizeEdges();

#if defined MESH_NINJA_DEBUG_MESH_SET_OPERATION
	objFileFormat.SaveMesh("Meshes/CutMeshA.obj", cutMeshA);
	objFileFormat.SaveMesh("Meshes/CutMeshB.obj", cutMeshB);
#endif //MESH_NINJA_DEBUG_MESH_SET_OPERATION

	Graph graphA, graphB;

	graphA.Generate(cutMeshA);
	graphB.Generate(cutMeshB);

	if (!graphA.ColorEdges(lineSegmentArray))
	{
		*this->error = "Failed to color edges of graph A.";
		return false;
	}

	if (!graphB.ColorEdges(lineSegmentArray))
	{
		*this->error = "Failed to color edges of graph B.";
		return false;
	}
	
	if (!graphA.ColorNodes(&graphB))
	{
		*this->error = "Failed to color nodes of graph A.";
		return false;
	}

	if (!graphB.ColorNodes(&graphA))
	{
		*this->error = "Failed to color nodes of graph B.";
		return false;
	}

	graphA.PopulatePolygonLists(polygonLists.meshA_insidePolygonList, polygonLists.meshA_outsidePolygonList);
	graphB.PopulatePolygonLists(polygonLists.meshB_insidePolygonList, polygonLists.meshB_outsidePolygonList);

	return true;
}

void MeshSetOperation::ChopupPolygonArray(std::vector<ConvexPolygon>& polygonArray, const std::vector<LineSegment>& lineSegmentArray)
{
	std::list<ConvexPolygon> polygonQueue;
	for (const ConvexPolygon& polygon : polygonArray)
		polygonQueue.push_back(polygon);

	polygonArray.clear();

	while (polygonQueue.size() > 0)
	{
		std::list<ConvexPolygon>::iterator iter = polygonQueue.begin();
		ConvexPolygon polygon = *iter;
		polygonQueue.erase(iter);

		ConvexPolygon polygonA, polygonB;
		if (!this->ChopupPolygon(polygon, polygonA, polygonB, lineSegmentArray))
			polygonArray.push_back(polygon);
		else
		{
			polygonQueue.push_back(polygonA);
			polygonQueue.push_back(polygonB);
		}
	}
}

bool MeshSetOperation::ChopupPolygon(const ConvexPolygon& polygon, ConvexPolygon& polygonA, ConvexPolygon& polygonB, const std::vector<LineSegment>& lineSegmentArray)
{
	for (const LineSegment& lineSegment : lineSegmentArray)
	{
		Plane plane;
		polygon.CalcPlane(plane);

		if (plane.WhichSide(lineSegment.vertexA) == Plane::Side::NEITHER &&
			plane.WhichSide(lineSegment.vertexB) == Plane::Side::NEITHER)
		{
			Vector normal = (lineSegment.vertexB - lineSegment.vertexA).Cross(plane.normal);
			Plane cuttingPlane(lineSegment.vertexA, normal);
			if (polygon.SplitAgainst(cuttingPlane, polygonA, polygonB))
			{
				polygonA.Compress();
				polygonB.Compress();

				if (polygonA.vertexArray->size() < 3 || polygonB.vertexArray->size() < 3)
					return false;
				
				return true;
			}
		}
	}

	return false;
}

//----------------------------------- MeshSetOperation::PolylineCollection -----------------------------------

MeshSetOperation::PolylineCollection::PolylineCollection()
{
}

/*virtual*/ MeshSetOperation::PolylineCollection::~PolylineCollection()
{
}

void MeshSetOperation::PolylineCollection::AddPolyline(const Polyline& givenPolyline)
{
	std::list<Polyline> polylineQueue;
	polylineQueue.push_back(givenPolyline);

	while (polylineQueue.size() > 0)
	{
		std::list<Polyline>::iterator iter = polylineQueue.begin();
		Polyline polyline = *iter;
		polylineQueue.erase(iter);

		bool mergeHappened = false;
		for (iter = this->polylineList.begin(); iter != this->polylineList.end(); iter++)
		{
			const Polyline& existingPolyline = *iter;
			Polyline mergedPolyline;
			if (mergedPolyline.Merge(polyline, existingPolyline))
			{
				this->polylineList.erase(iter);
				polylineQueue.push_back(mergedPolyline);
				mergeHappened = true;
				break;
			}
		}

		if (!mergeHappened)
			this->polylineList.push_back(polyline);
	}
}

//----------------------------------- MeshSetOperation::Node -----------------------------------

MeshSetOperation::Node::Node()
{
	this->side = Side::UNKNOWN;
}

/*virtual*/ MeshSetOperation::Node::~Node()
{
}

//----------------------------------- MeshSetOperation::Edge -----------------------------------

MeshSetOperation::Edge::Edge()
{
	this->type = Type::UNKNOWN;
}

/*virtual*/ MeshSetOperation::Edge::~Edge()
{
}

//----------------------------------- MeshSetOperation::Graph -----------------------------------

MeshSetOperation::Graph::Graph()
{
}

/*virtual*/ MeshSetOperation::Graph::~Graph()
{
}

/*virtual*/ MeshGraph::Node* MeshSetOperation::Graph::CreateNode()
{
	return new MeshSetOperation::Node();
}

/*virtual*/ MeshGraph::Edge* MeshSetOperation::Graph::CreateEdge()
{
	return new MeshSetOperation::Edge();
}

bool MeshSetOperation::Graph::ColorEdges(const std::vector<LineSegment>& lineSegmentArray)
{
	int cutCount = 0;

	for (Edge* edge : *this->edgeArray)
	{
		LineSegment edgeSegment((*this->mesh->vertexArray)[edge->pair.i], (*this->mesh->vertexArray)[edge->pair.j]);

		bool cutFound = false;

		for (const LineSegment& lineSegment : lineSegmentArray)
		{
			// TODO: Actually, I think we just need to check that the edge segment is contained within the line segment.
			if (edgeSegment.IsEqualTo(lineSegment))
			{
				cutCount++;
				cutFound = true;
				((MeshSetOperation::Edge*)edge)->type = MeshSetOperation::Edge::Type::CUT_BOUNDARY;
				break;
			}
		}

		if (!cutFound)
			((MeshSetOperation::Edge*)edge)->type = MeshSetOperation::Edge::Type::NORMAL;
	}

	return cutCount == lineSegmentArray.size();
}

bool MeshSetOperation::Graph::ColorNodes(const Graph* otherGraph)
{
	MeshSetOperation::Node* node = this->FindInitialOutsideNode(otherGraph);
	node->side = MeshSetOperation::Node::Side::OUTSIDE;

	std::list<MeshSetOperation::Node*> nodeQueue;
	nodeQueue.push_back(node);

	while (nodeQueue.size() > 0)
	{
		std::list<MeshSetOperation::Node*>::iterator iter = nodeQueue.begin();
		node = *iter;
		nodeQueue.erase(iter);

		for (Edge* edge : node->edgeArray)
		{
			MeshSetOperation::Node* adjacentNode = (MeshSetOperation::Node*)edge->Fallow(node);
			if (adjacentNode->side == MeshSetOperation::Node::Side::UNKNOWN)
			{
				switch(((MeshSetOperation::Edge*)edge)->type)
				{
					case MeshSetOperation::Edge::Type::CUT_BOUNDARY:
					{
						if (node->side == MeshSetOperation::Node::Side::INSIDE)
							adjacentNode->side = MeshSetOperation::Node::Side::OUTSIDE;
						else
							adjacentNode->side = MeshSetOperation::Node::Side::INSIDE;
						break;
					}
					case MeshSetOperation::Edge::Type::NORMAL:
					{
						adjacentNode->side = node->side;
						break;
					}
					default:
					{
						return false;
					}
				}

				nodeQueue.push_back(adjacentNode);
			}
		}
	}

	return true;
}

MeshSetOperation::Node* MeshSetOperation::Graph::FindInitialOutsideNode(const Graph* otherGraph)
{
	for (Node* node : *this->nodeArray)
	{
		ConvexPolygon polygon;
		node->facet->MakePolygon(polygon, this->mesh);

		Plane plane;
		polygon.CalcPlane(plane);

		if (plane.AllPointsNotOnSide(*this->mesh->vertexArray, Plane::Side::FRONT) && plane.AllPointsNotOnSide(*otherGraph->mesh->vertexArray, Plane::Side::FRONT))
		{
			return (MeshSetOperation::Node*)node;
		}
	}

	return nullptr;
}

void MeshSetOperation::Graph::PopulatePolygonLists(std::vector<ConvexPolygon>& insidePolygonList, std::vector<ConvexPolygon>& outsidePolygonList) const
{
	for (const MeshGraph::Node* baseNode : *this->nodeArray)
	{
		MeshSetOperation::Node* node = (MeshSetOperation::Node*)baseNode;
		
		ConvexPolygon polygon;
		node->facet->MakePolygon(polygon, this->mesh);

		switch (node->side)
		{
			case MeshSetOperation::Node::Side::INSIDE:
			{
				insidePolygonList.push_back(polygon);
				break;
			}
			case MeshSetOperation::Node::Side::OUTSIDE:
			{
				outsidePolygonList.push_back(polygon);
				break;
			}
			default:
			{
				assert(false);
				break;
			}
		}
	}
}

//----------------------------------- MeshUnion -----------------------------------

MeshUnion::MeshUnion()
{
}

/*virtual*/ MeshUnion::~MeshUnion()
{
}

/*virtual*/ bool MeshUnion::Perform(const ConvexPolygonMesh& meshA, const ConvexPolygonMesh& meshB, ConvexPolygonMesh& resultingMesh)
{
	PolygonLists polygonLists;
	if (!this->CalculatePolygonLists(meshA, meshB, polygonLists))
		return false;

	std::vector<ConvexPolygon> polygonArray;

	for (const ConvexPolygon& polygon : polygonLists.meshA_outsidePolygonList)
		polygonArray.push_back(polygon);

	for (const ConvexPolygon& polygon : polygonLists.meshB_outsidePolygonList)
		polygonArray.push_back(polygon);

	resultingMesh.FromConvexPolygonArray(polygonArray);
	return true;
}

//----------------------------------- MeshIntersection -----------------------------------

MeshIntersection::MeshIntersection()
{
}

/*virtual*/ MeshIntersection::~MeshIntersection()
{
}

/*virtual*/ bool MeshIntersection::Perform(const ConvexPolygonMesh& meshA, const ConvexPolygonMesh& meshB, ConvexPolygonMesh& resultingMesh)
{
	PolygonLists polygonLists;
	if (!this->CalculatePolygonLists(meshA, meshB, polygonLists))
		return false;

	std::vector<ConvexPolygon> polygonArray;

	for (const ConvexPolygon& polygon : polygonLists.meshA_insidePolygonList)
		polygonArray.push_back(polygon);

	for (const ConvexPolygon& polygon : polygonLists.meshB_insidePolygonList)
		polygonArray.push_back(polygon);

	resultingMesh.FromConvexPolygonArray(polygonArray);
	return true;
}

//----------------------------------- MeshSubtraction -----------------------------------

MeshSubtraction::MeshSubtraction()
{
}

/*virtual*/ MeshSubtraction::~MeshSubtraction()
{
}

/*virtual*/ bool MeshSubtraction::Perform(const ConvexPolygonMesh& meshA, const ConvexPolygonMesh& meshB, ConvexPolygonMesh& resultingMesh)
{
	PolygonLists polygonLists;
	if (!this->CalculatePolygonLists(meshA, meshB, polygonLists))
		return false;

	std::vector<ConvexPolygon> polygonArray;

	for (const ConvexPolygon& polygon : polygonLists.meshA_outsidePolygonList)
		polygonArray.push_back(polygon);

	for (const ConvexPolygon& polygon : polygonLists.meshB_insidePolygonList)
	{
		ConvexPolygon reversePolygon;
		reversePolygon.MakeReverseOf(polygon);
		polygonArray.push_back(reversePolygon);
	}

	resultingMesh.FromConvexPolygonArray(polygonArray);
	return true;
}
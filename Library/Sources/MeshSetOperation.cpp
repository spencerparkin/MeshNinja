#include "MeshSetOperation.h"
#include "ConvexPolygonMesh.h"
#include "LineSegment.h"
#include "Plane.h"
#include "AxisAlignedBoundingBox.h"
#include "Ray.h"
#include "BoundingBoxTree.h"
#if defined MESH_NINJA_DEBUG
#	include "FileFormats/ObjFileFormat.h"
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
#if defined MESH_NINJA_DEBUG
	ObjFileFormat objFileFormat;
#endif //MESH_NINJA_DEBUG

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

#if defined MESH_NINJA_DEBUG
					ConvexPolygonMesh debugMesh;
					std::vector<ConvexPolygon> debugPolygonArray;
					debugPolygonArray.push_back(polygonA);
					debugMesh.FromConvexPolygonArray(debugPolygonArray);
					objFileFormat.SaveMesh("Meshes/DebugMeshA.obj", debugMesh);
					debugPolygonArray.clear();
					debugPolygonArray.push_back(polygonB);
					debugMesh.FromConvexPolygonArray(debugPolygonArray);
					objFileFormat.SaveMesh("Meshes/DebugMeshB.obj", debugMesh);
#endif //MESH_NINJA_DEBUG

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

#if defined MESH_NINJA_DEBUG
	int fileCount = 0;
	for (const Polyline& polyline : polylineCollection.polylineList)
	{
		ConvexPolygonMesh tubeMesh;
		if (polyline.GenerateTubeMesh(tubeMesh, 0.05, 10))
			objFileFormat.SaveMesh(std::format("Meshes/polyline{}.obj", fileCount++), tubeMesh);
	}
#endif //MESH_NINJA_DEBUG

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

	// We do this to make sure that each edge representing a cut in each cut-mesh is a sub-set of a cut-boundary-line.
	for (const Polyline& polyline : polylineCollection.polylineList)
	{
		for (int i = 0; i < (signed)polyline.vertexArray->size(); i++)
		{
			const Vector& vertex = (*polyline.vertexArray)[i];
			cutMeshA.AddRedundantVertex(vertex);
			cutMeshB.AddRedundantVertex(vertex);
		}
	}

#if defined MESH_NINJA_DEBUG
	objFileFormat.SaveMesh("Meshes/CutMeshA.obj", cutMeshA);
	objFileFormat.SaveMesh("Meshes/CutMeshB.obj", cutMeshB);
#endif //MESH_NINJA_DEBUG

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
	
#if defined MESH_NINJA_DEBUG
	DebugDraw graphDebugDrawA;
	DebugDraw graphDebugDrawB;

	graphA.GenerateDebugDrawObjects(graphDebugDrawA);
	graphB.GenerateDebugDrawObjects(graphDebugDrawB);

	graphDebugDrawA.Save("Meshes/GraphDebugDrawA.json");
	graphDebugDrawB.Save("Meshes/GraphDebugDrawB.json");
#endif //MESH_NINJA_DEBUG

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
			bool performSplit = false;
			bool isInteriorPoint = false;

			if ((polygon.ContainsPoint(lineSegment.vertexA, &isInteriorPoint) && isInteriorPoint) ||
				(polygon.ContainsPoint(lineSegment.vertexB, &isInteriorPoint) && isInteriorPoint))
			{
				performSplit = true;
			}

			if (!performSplit)
			{
				Ray rayA(lineSegment.vertexA, lineSegment.vertexB - lineSegment.vertexA);
				Ray rayB(lineSegment.vertexB, lineSegment.vertexA - lineSegment.vertexB);

				double alpha = 0.0, beta = 0.0;

				if (rayA.CastAgainst(polygon, alpha) && 0.0 <= alpha && alpha <= 1.0 &&
					rayB.CastAgainst(polygon, beta) && 0.0 <= beta && beta <= 1.0)
				{
					Vector hitPointA = rayA.Lerp(alpha);
					Vector hitPointB = rayB.Lerp(beta);
					Vector point = (hitPointA + hitPointB) / 2.0;
					
					if (polygon.ContainsPoint(point, &isInteriorPoint) && isInteriorPoint)
						performSplit = true;
				}
			}

			if (performSplit)
			{
				Vector normal = (lineSegment.vertexB - lineSegment.vertexA).Cross(plane.normal);
				Plane cuttingPlane(lineSegment.vertexA, normal);
				if (polygon.SplitAgainst(cuttingPlane, polygonA, polygonB))
				{
					polygonA.Compress();
					polygonB.Compress();

					if (polygonA.vertexArray->size() < 3 || polygonB.vertexArray->size() < 3)
					{
#if defined MESH_NINJA_DEBUG
						std::vector<ConvexPolygon> polygonArray;
						polygonArray.push_back(polygon);
						ConvexPolygonMesh mesh;
						mesh.FromConvexPolygonArray(polygonArray);
						ObjFileFormat objFileFormat;
						objFileFormat.SaveMesh("Meshes/DebugMeshA.obj", mesh);
						Polyline polyline;
						polyline.vertexArray->push_back(lineSegment.vertexA);
						polyline.vertexArray->push_back(lineSegment.vertexB);
						polyline.GenerateTubeMesh(mesh, 0.1, 5);
						objFileFormat.SaveMesh("Meshes/DebugMeshB.obj", mesh);
#endif //MESH_NINJA_DEBUG

						return false;
					}

					return true;
				}
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

/*virtual*/ Vector MeshSetOperation::Node::GetDebugColor() const
{
	switch (this->side)
	{
		case Side::INSIDE:
		{
			return Vector(1.0, 0.0, 0.0);
		}
		case Side::OUTSIDE:
		{
			return Vector(0.0, 0.0, 1.0);
		}
	}

	return Vector(0.5, 0.5, 0.5);
}

//----------------------------------- MeshSetOperation::Edge -----------------------------------

MeshSetOperation::Edge::Edge()
{
	this->type = Type::UNKNOWN;
}

/*virtual*/ MeshSetOperation::Edge::~Edge()
{
}

/*virtual*/ Vector MeshSetOperation::Edge::GetDebugColor() const
{
	switch (this->type)
	{
		case Type::NORMAL:
		{
			return Vector(0.0, 1.0, 0.0);
		}
		case Type::CUT_BOUNDARY:
		{
			return Vector(1.0, 0.0, 0.0);
		}
	}

	return Vector(0.5, 0.5, 0.5);
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

bool MeshSetOperation::Graph::ColorEdges(const std::vector<LineSegment>& lineSegmentArray, double eps /*= MESH_NINJA_EPS*/)
{
	for (Edge* edge : *this->edgeArray)
	{
		LineSegment edgeSegment((*this->mesh->vertexArray)[edge->pair.i], (*this->mesh->vertexArray)[edge->pair.j]);

		bool cutFound = false;

		for (const LineSegment& lineSegment : lineSegmentArray)
		{
			if (lineSegment.ContainsPoint(edgeSegment.vertexA, eps) && lineSegment.ContainsPoint(edgeSegment.vertexB, eps))
			{
				cutFound = true;
				((MeshSetOperation::Edge*)edge)->type = MeshSetOperation::Edge::Type::CUT_BOUNDARY;
				break;
			}
		}

		if (!cutFound)
			((MeshSetOperation::Edge*)edge)->type = MeshSetOperation::Edge::Type::NORMAL;
	}

	return true;
}

bool MeshSetOperation::Graph::ColorNodes(const Graph* otherGraph)
{
	while (true)
	{
		MeshSetOperation::Node* node = this->FindInitialOutsideNode(otherGraph);
		if (!node)
			break;

		assert(node->side == MeshSetOperation::Node::Side::UNKNOWN);
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
					switch (((MeshSetOperation::Edge*)edge)->type)
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
	}

	return true;
}

MeshSetOperation::Node* MeshSetOperation::Graph::FindInitialOutsideNode(const Graph* otherGraph)
{
	for (Node* node : *this->nodeArray)
	{
		if (((MeshSetOperation::Node*)node)->side != MeshSetOperation::Node::Side::UNKNOWN)
			continue;

		ConvexPolygon polygon;
		node->facet->MakePolygon(polygon, this->mesh);

		Plane plane;
		polygon.CalcPlane(plane);

		if (plane.AllPointsNotOnSide(*this->mesh->vertexArray, Plane::Side::FRONT) && plane.AllPointsNotOnSide(*otherGraph->mesh->vertexArray, Plane::Side::FRONT))
		{
			return (MeshSetOperation::Node*)node;
		}
	}

	// Okay, if we get here, that doesn't mean that there doesn't exist an
	// outside polygon from which we can start the graph coloring algorithm.
	// We just have to go about finding one a different way.

	BoundingBoxTree thisTree, otherTree;

	class Entry : public BoundingBoxTree::Object
	{
	public:
		Entry(MeshSetOperation::Node* node, const Graph* graph)
		{
			this->node = node;
			this->graph = graph;
			this->node->facet->MakePolygon(this->polygon, this->graph->mesh);
			this->polygon.CalcBox(this->box);
		}

		virtual ~Entry()
		{
		}

		virtual AxisAlignedBoundingBox GetBoundingBox() const override
		{
			return box;
		}

		virtual bool IsHitByRay(const Ray& ray, double& alpha) const override
		{
			return ray.CastAgainst(this->polygon, alpha);
		}

		MeshSetOperation::Node* node;
		const Graph* graph;
		ConvexPolygon polygon;
		AxisAlignedBoundingBox box;
	};

	// TODO: Refactor the code so that we're not building these trees with every call.
	std::vector<BoundingBoxTree::Object*> objectArray;
	for (Node* node : *this->nodeArray)
		objectArray.push_back(new Entry((MeshSetOperation::Node*)node, this));
	thisTree.Rebuild(objectArray);
	objectArray.clear();
	for (Node* node : *otherGraph->nodeArray)
		objectArray.push_back(new Entry((MeshSetOperation::Node*)node, otherGraph));
	otherTree.Rebuild(objectArray);
	objectArray.clear();

	if (this->mesh->vertexArray->size() == 0)
		return nullptr;

	AxisAlignedBoundingBox aabb;
	aabb.min = (*this->mesh->vertexArray)[0];
	aabb.max = (*this->mesh->vertexArray)[0];
	for (const Vector& vertex : *this->mesh->vertexArray)
		aabb.ExpandToIncludePoint(vertex);
	for (const Vector& vertex : *otherGraph->mesh->vertexArray)
		aabb.ExpandToIncludePoint(vertex);
	aabb.ScaleAboutCenter(1.5);

	for (Node* node : *this->nodeArray)
	{
		if (((MeshSetOperation::Node*)node)->side != MeshSetOperation::Node::Side::UNKNOWN)
			continue;

		ConvexPolygon polygon;
		node->facet->MakePolygon(polygon, this->mesh);

		Vector center = polygon.CalcCenter();

		for (int i = 0; i < 6; i++)
		{
			Ray ray;

			switch (i)
			{
				case 0: ray.origin = Vector(center.x, center.y, aabb.max.z); break;
				case 1: ray.origin = Vector(center.x, center.y, aabb.min.z); break;
				case 2: ray.origin = Vector(center.x, aabb.max.y, center.z); break;
				case 3: ray.origin = Vector(center.x, aabb.min.y, center.z); break;
				case 4: ray.origin = Vector(aabb.max.x, center.y, center.z); break;
				case 5: ray.origin = Vector(aabb.min.x, center.y, center.z); break;
			}

			ray.direction = center - ray.origin;

#if 0
			// Does the ray hit the polygon center?
			double alpha = 0.0;
			if (ray.CastAgainst(*this->mesh, alpha) && ray.Lerp(alpha).IsEqualTo(center))
			{
				// And does the ray NOT hit something closer in the other mesh?
				double beta = 0.0;
				if (!ray.CastAgainst(*otherGraph->mesh, beta) || (beta > alpha && aabb.ContainsPoint(ray.Lerp(beta))))
				{
					return (MeshSetOperation::Node*)node;
				}
			}
#else
			// Does the ray hit the polygon center?
			double alpha = 0.0;
			Entry* thisEntry = (Entry*)thisTree.FindClosestHit(ray, &alpha);
			if (thisEntry && thisEntry->node == (MeshSetOperation::Node*)node)
			{
				// And does the ray NOT hit something closer in the other mesh?
				double beta = 0.0;
				Entry* otherEntry = (Entry*)otherTree.FindClosestHit(ray, &beta);
				if (!otherEntry || alpha < beta)
				{
					return (MeshSetOperation::Node*)node;
				}
			}
#endif
		}
	}

	// At this point, we may still have just not found an outside polygon,
	// or indeed, one may not actually exist.

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
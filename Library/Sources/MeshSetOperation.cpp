#include "MeshSetOperation.h"
#include "ConvexPolygonMesh.h"
#include "LineSegment.h"
#include "Plane.h"
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
	ObjFileFormat objFileFormat;
	int fileCount = 0;
	for (const Polyline& polyline : polylineCollection.polylineList)
	{
		ConvexPolygonMesh tubeMesh;
		if (polyline.GenerateTubeMesh(tubeMesh, 0.5, 10))
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

#if defined MESH_NINJA_DEBUG_MESH_SET_OPERATION
	objFileFormat.SaveMesh("Meshes/CutMeshA.obj", cutMeshA);
	objFileFormat.SaveMesh("Meshes/CutMeshB.obj", cutMeshB);
#endif //MESH_NINJA_DEBUG_MESH_SET_OPERATION

	// TODO: Create cutMeshA and cutMeshB, then provide a way to perform a BFS across the facets.
	//       As we walk the meshes, we can label each facet as inside or outside.  We start the BFS
	//       on the outside (somehow), then we flip from outside to inside (or vise-versa) whenever
	//       we cross a line-loop boundary.

	return false;
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
		bool isInteriorPoint = false;
		Vector midPoint = lineSegment.CalcMidpoint();
		if ((polygon.ContainsPoint(lineSegment.vertexA, &isInteriorPoint) && isInteriorPoint) ||
			(polygon.ContainsPoint(lineSegment.vertexB, &isInteriorPoint) && isInteriorPoint) ||
			(polygon.ContainsPoint(midPoint, &isInteriorPoint) && isInteriorPoint))
		{
			Plane plane = polygon.CalcPlane();
			Vector normal = (lineSegment.vertexB - lineSegment.vertexA).Cross(plane.normal);
			Plane cuttingPlane(lineSegment.vertexA, normal);
			bool split = polygon.SplitAgainst(cuttingPlane, polygonA, polygonB);
			assert(split);
			return true;
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
		polygonArray.push_back(polygon);

	resultingMesh.FromConvexPolygonArray(polygonArray);
	return true;
}
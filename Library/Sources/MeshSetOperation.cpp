#include "MeshSetOperation.h"
#include "ConvexPolygonMesh.h"

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
					// It is certainly possible to define the intersection, untion and subtraction of meshes in
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

	// TODO: Cut the polygons against the line-loops.

	// TODO: Create cutMeshA and cutMeshB, then provide a way to perform a BFS across the facets.
	//       As we walk the meshes, we can label each facet as inside or outside.  We start the BFS
	//       on the outside (somehow), then we flip from outside to inside (or vise-versa) whenever
	//       we cross a line-loop boundary.

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
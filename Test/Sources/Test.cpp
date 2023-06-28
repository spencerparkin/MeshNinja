#include "MeshFileFormat.h"
#include "ConvexPolygonMesh.h"
#include "MeshBinaryOperation.h"
#include "ConvexPolygon.h"

int main(int argc, char** argv)
{
	using namespace MeshNinja;

	ConvexPolygonMesh meshA, meshB;
	ObjFileFormat fileFormat;

	if (!fileFormat.LoadMesh("Meshes/BoxA.obj", meshA))
	{
		fprintf(stderr, "Failed to load first box!\n");
		return 1;
	}

	if (!fileFormat.LoadMesh("Meshes/BoxB.obj", meshB))
	{
		fprintf(stderr, "Failed to load second box!\n");
		return 1;
	}

	MeshMergeOperation mergeOp;
	ConvexPolygonMesh mergedMesh;
	if (!mergeOp.Perform(meshA, meshB, mergedMesh))
	{
		fprintf(stderr, "Failed to merge the two boxes!\n");
		return 1;
	}

	if (!fileFormat.SaveMesh("Meshes/MergedMesh.obj", mergedMesh))
	{
		fprintf(stderr, "Failed to save merged mesh!\n");
		return 1;
	}

	ConvexPolygon polygonA, polygonB;

	polygonA.vertexArray->push_back(Vector(-1.0, -1.0, 0.0));
	polygonA.vertexArray->push_back(Vector(1.0, -1.0, 0.0));
	polygonA.vertexArray->push_back(Vector(1.0, 1.0, 0.0));
	polygonA.vertexArray->push_back(Vector(-1.0, 1.0, 0.0));

	polygonB.vertexArray->push_back(Vector(0.0, 0.0, -1.0));
	polygonB.vertexArray->push_back(Vector(2.0, 0.0, -1.0));
	polygonB.vertexArray->push_back(Vector(2.0, 0.0, 1.0));
	polygonB.vertexArray->push_back(Vector(0.0, 0.0, 1.0));

	ConvexPolygon intersection;
	if (!intersection.Intersect(polygonA, polygonB))
	{
		fprintf(stderr, "Polygons do not intersect!");
		return 1;
	}

	printf("Intersection...\n");
	for (int i = 0; i < (signed)intersection.vertexArray->size(); i++)
	{
		const Vector& vertex = (*intersection.vertexArray)[i];
		printf("%d -> %f, %f, %f\n", i, vertex.x, vertex.y, vertex.z);
	}

	return 0;
}
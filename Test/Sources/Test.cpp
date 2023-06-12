#include "MeshFileFormat.h"
#include "ConvexPolygonMesh.h"
#include "MeshBinaryOperation.h"

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

	return 0;
}
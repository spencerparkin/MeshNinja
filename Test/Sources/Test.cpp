#include "MeshFileFormat.h"
#include "ConvexPolygonMesh.h"
#include "MeshBinaryOperation.h"
#include "MeshSetOperation.h"
#include "ConvexPolygon.h"

int main(int argc, char** argv)
{
	using namespace MeshNinja;

	ConvexPolygonMesh meshA, meshB;
	ObjFileFormat fileFormat;

	std::string meshA_path = "Meshes/SphereA.obj";
	std::string meshB_path = "Meshes/CylinderA.obj";

	//std::string meshA_path = "Meshes/DebugMeshA.obj";
	//std::string meshB_path = "Meshes/DebugMeshB.obj";

	if (!fileFormat.LoadMesh(meshA_path, meshA))
	{
		fprintf(stderr, "Failed to load first mesh!\n");
		return 1;
	}

	if (!fileFormat.LoadMesh(meshB_path, meshB))
	{
		fprintf(stderr, "Failed to load second mesh!\n");
		return 1;
	}

	MeshMergeOperation mergeOp;
	ConvexPolygonMesh mergedMesh;
	if (!mergeOp.Perform(meshA, meshB, mergedMesh))
	{
		fprintf(stderr, "Failed to merge the two meshes!\n");
		return 1;
	}

	if (!fileFormat.SaveMesh("Meshes/MergedMesh.obj", mergedMesh))
	{
		fprintf(stderr, "Failed to save merged mesh!\n");
		return 1;
	}

	MeshSubtraction subtractOp;
	ConvexPolygonMesh differenceMesh;
	if (!subtractOp.Perform(meshA, meshB, differenceMesh))
	{
		fprintf(stderr, "Failed to subject the two meshes!\n");
		return 1;
	}

	if (!fileFormat.SaveMesh("Meshes/DifferenceMesh.obj", differenceMesh))
	{
		fprintf(stderr, "Failed to save difference mesh!\n");
		return 1;
	}

	return 0;
}
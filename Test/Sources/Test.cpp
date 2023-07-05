#include "MeshFileFormat.h"
#include "ConvexPolygonMesh.h"
#include "MeshBinaryOperation.h"
#include "MeshSetOperation.h"
#include "ConvexPolygon.h"

int main(int argc, char** argv)
{
	using namespace MeshNinja;

	ObjFileFormat fileFormat;

#if 0
	ConvexPolygonMesh meshA, meshB;

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

	MeshIntersection meshOp;
	ConvexPolygonMesh resultMesh;
	if (!meshOp.Perform(meshA, meshB, resultMesh))
	{
		fprintf(stderr, "Failed to perform operation on the two given meshes!\n");
		return 1;
	}

	if (!fileFormat.SaveMesh("Meshes/ResultMesh.obj", resultMesh))
	{
		fprintf(stderr, "Failed to save result mesh!\n");
		return 1;
	}
#endif

#if 1
	ConvexPolygonMesh mesh;
	
	mesh.GeneratePolyhedron(ConvexPolygonMesh::Polyhedron::TETRAHEDRON);
	fileFormat.SaveMesh("Meshes/Tetrahedron.obj", mesh);

	mesh.GeneratePolyhedron(ConvexPolygonMesh::Polyhedron::OCTAHEDRON);
	fileFormat.SaveMesh("Meshes/Octahedron.obj", mesh);

	mesh.GeneratePolyhedron(ConvexPolygonMesh::Polyhedron::DODECAHEDRON);
	mesh.UntessellateFaces();
	fileFormat.SaveMesh("Meshes/Dodecahedron.obj", mesh);

	mesh.GeneratePolyhedron(ConvexPolygonMesh::Polyhedron::ICOSAHEDRON);
	fileFormat.SaveMesh("Meshes/Icosahedron.obj", mesh);

	mesh.GeneratePolyhedron(ConvexPolygonMesh::Polyhedron::ICOSIDODECAHEDRON);
	mesh.UntessellateFaces();
	fileFormat.SaveMesh("Meshes/Icosidodecahedron.obj", mesh);

	mesh.GeneratePolyhedron(ConvexPolygonMesh::Polyhedron::RHOMBICOSIDODECAHEDRON);
	mesh.UntessellateFaces();
	fileFormat.SaveMesh("Meshes/Rhombicosidodecahedron.obj", mesh);

#endif

	return 0;
}
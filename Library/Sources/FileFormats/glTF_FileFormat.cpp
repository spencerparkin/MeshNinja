#include "glTF_FileFormat.h"
#include "../JSON/JsonValue.h"
#include "../RenderMesh.h"

using namespace MeshNinja;

glTF_FileFormat::glTF_FileFormat()
{
}

/*virtual*/ glTF_FileFormat::~glTF_FileFormat()
{
}

/*virtual*/ std::string glTF_FileFormat::GetExtension() const
{
	return ".gltf";
}

/*virtual*/ bool glTF_FileFormat::LoadMesh(const std::string& filePath, ConvexPolygonMesh& mesh, int meshNumber /*= 0*/)
{
	return false;
}

/*virtual*/ bool glTF_FileFormat::SaveMesh(const std::string& filePath, const ConvexPolygonMesh& mesh)
{
	return false;
}

/*virtual*/ bool glTF_FileFormat::LoadRenderMesh(const std::string& filePath, RenderMesh& mesh, int meshNumber /*= 0*/)
{
	return false;
}

/*virtual*/ bool glTF_FileFormat::SaveRenderMesh(const std::string& filePath, const RenderMesh& mesh)
{
	bool success = false;
	std::ofstream jsonFileStream;
	std::ofstream binFileStream;
	JsonObject* jsonRootObject = nullptr;

	while (true)
	{
		if (!mesh.IsTriangleMesh())
			break;

		jsonFileStream.open(filePath, std::ios::out);
		if (!jsonFileStream.is_open())
			break;

		std::string binFilePath = std::filesystem::path(filePath).replace_extension(".bin").string();
		binFileStream.open(binFilePath, std::ios::binary | std::ios::out);
		if (!binFileStream.is_open())
			break;

		jsonRootObject = new JsonObject();

		JsonObject* jsonAssetObject = new JsonObject();
		jsonAssetObject->SetValue("version", new JsonString("2.0"));
		jsonAssetObject->SetValue("generator", new JsonString("Mesh Ninja"));
		jsonAssetObject->SetValue("copyright", new JsonString("2023 (c) Spencer T. Parkin"));
		jsonRootObject->SetValue("asset", jsonAssetObject);

		JsonArray* jsonBufferViewsArray = new JsonArray();
		jsonRootObject->SetValue("bufferViews", jsonBufferViewsArray);

		JsonArray* jsonAccessorsArray = new JsonArray();
		jsonRootObject->SetValue("accessor", jsonAccessorsArray);

		JsonArray* jsonMeshesArray = new JsonArray();
		jsonRootObject->SetValue("meshes", jsonMeshesArray);

		JsonObject* jsonMeshObject = new JsonObject();
		jsonMeshesArray->PushValue(jsonMeshObject);

		JsonArray* jsonPrimArray = new JsonArray();
		jsonMeshObject->SetValue("primitives", jsonPrimArray);

		JsonObject* jsonPrim = new JsonObject();
		jsonPrimArray->PushValue(jsonPrim);
		jsonPrim->SetValue("mode", new JsonInt(MESH_NINJA_GLTF_TOPO_MODE_TRIANGLES));

		if (!this->WriteIndexBuffer(mesh, binFileStream, jsonBufferViewsArray, jsonAccessorsArray, jsonPrim))
			break;

		if (!this->WriteVertexBuffer(mesh, binFileStream, jsonBufferViewsArray, jsonAccessorsArray, jsonPrim))
			break;

		JsonArray* jsonBuffersArray = new JsonArray();
		jsonRootObject->SetValue("buffers", jsonBuffersArray);

		JsonObject* jsonBufferObject = new JsonObject();
		jsonBufferObject->SetValue("byteLength", new JsonInt((long)binFileStream.tellp()));
		jsonBufferObject->SetValue("uri", new JsonString(std::filesystem::path(binFilePath).filename().string()));
		jsonBuffersArray->PushValue(jsonBufferObject);

		std::string jsonString;
		if (!jsonRootObject->PrintJson(jsonString))
			break;

		jsonFileStream << jsonString;

		success = true;
		break;
	}

	if (jsonFileStream.is_open())
		jsonFileStream.close();

	if (binFileStream.is_open())
		binFileStream.close();

	delete jsonRootObject;
	return success;
}

bool glTF_FileFormat::WriteIndexBuffer(const RenderMesh& mesh, std::ofstream& binFileStream, JsonArray* jsonBufferViewsArray, JsonArray* jsonAccessorsArray, JsonObject* jsonPrim)
{
	long offsetA = (long)binFileStream.tellp();

	for (const RenderMesh::Facet& facet : *mesh.facetArray)
		for (int i = 0; i < (signed)facet.vertexArray->size(); i++)
			binFileStream << uint32_t(facet[i]);

	long offsetB = (long)binFileStream.tellp();
	long byteLength = offsetB - offsetA;

	JsonObject* jsonBufferView = new JsonObject();
	int bufferViewNumber = jsonBufferViewsArray->GetSize();
	jsonBufferViewsArray->PushValue(jsonBufferView);
	jsonBufferView->SetValue("buffer", new JsonInt(0));
	jsonBufferView->SetValue("byteLength", new JsonInt(byteLength));
	jsonBufferView->SetValue("byteOffset", new JsonInt(offsetA));
	jsonBufferView->SetValue("target", new JsonInt(MESH_NINJA_GLTF_ARRAY_BUFFER));

	JsonObject* jsonAccessor = new JsonObject();
	int accessorNumber = jsonAccessorsArray->GetSize();
	jsonAccessorsArray->PushValue(jsonAccessor);
	jsonAccessor->SetValue("bufferView", new JsonInt(bufferViewNumber));
	jsonAccessor->SetValue("byteOffset", new JsonInt(0));
	jsonAccessor->SetValue("componentType", new JsonInt(MESH_NINJA_GLTF_ACCESSOR_DT_UNSIGNED_INT));
	jsonAccessor->SetValue("type", new JsonString("SCALAR"));

	jsonPrim->SetValue("indices", new JsonInt(accessorNumber));

	return true;
}

bool glTF_FileFormat::WriteVertexBuffer(const RenderMesh& mesh, std::ofstream& binFileStream, JsonArray* jsonBufferViewsArray, JsonArray* jsonAccessorsArray, JsonObject* jsonPrim)
{
	long offsetA = (long)binFileStream.tellp();

	for (const RenderMesh::Vertex& vertex : *mesh.vertexArray)
	{
		binFileStream << float(vertex.position.x) << float(vertex.position.y) << float(vertex.position.z);
		binFileStream << float(vertex.normal.x) << float(vertex.normal.y) << float(vertex.normal.z);
		binFileStream << float(vertex.color.x) << float(vertex.color.y) << float(vertex.color.z);
	}

	long offsetB = (long)binFileStream.tellp();
	long byteLength = offsetB - offsetA;

	JsonObject* jsonPositionBufferView = new JsonObject();
	int positionBufferViewNumber = jsonBufferViewsArray->GetSize();
	jsonBufferViewsArray->PushValue(jsonPositionBufferView);
	jsonPositionBufferView->SetValue("buffer", new JsonInt(0));
	jsonPositionBufferView->SetValue("byteLength", new JsonInt(sizeof(float) * 3 * mesh.vertexArray->size()));
	jsonPositionBufferView->SetValue("byteOffset", new JsonInt(0));
	jsonPositionBufferView->SetValue("byteStride", new JsonInt(sizeof(float) * 9));
	jsonPositionBufferView->SetValue("target", new JsonInt(MESH_NINJA_GLTF_ELEMENT_ARRAY_BUFFER));

	JsonObject* jsonNormalBufferView = new JsonObject();
	int normalBufferViewNumber = jsonBufferViewsArray->GetSize();
	jsonBufferViewsArray->PushValue(jsonNormalBufferView);
	jsonNormalBufferView->SetValue("buffer", new JsonInt(0));
	jsonNormalBufferView->SetValue("byteLength", new JsonInt(sizeof(float) * 3 * mesh.vertexArray->size()));
	jsonNormalBufferView->SetValue("byteOffset", new JsonInt(sizeof(float) * 3));
	jsonNormalBufferView->SetValue("byteStride", new JsonInt(sizeof(float) * 9));
	jsonNormalBufferView->SetValue("target", new JsonInt(MESH_NINJA_GLTF_ELEMENT_ARRAY_BUFFER));

	JsonObject* jsonColorBufferView = new JsonObject();
	int colorBufferViewNumber = jsonBufferViewsArray->GetSize();
	jsonBufferViewsArray->PushValue(jsonColorBufferView);
	jsonColorBufferView->SetValue("buffer", new JsonInt(0));
	jsonColorBufferView->SetValue("bufferLength", new JsonInt(sizeof(float) * 3 * mesh.vertexArray->size()));
	jsonColorBufferView->SetValue("bufferOffset", new JsonInt(sizeof(float) * 6));
	jsonColorBufferView->SetValue("bufferStride", new JsonInt(sizeof(float) * 9));
	jsonColorBufferView->SetValue("target", new JsonInt(MESH_NINJA_GLTF_ELEMENT_ARRAY_BUFFER));

	JsonObject* jsonPositionAccessor = new JsonObject();
	int positionAccessorNumber = jsonAccessorsArray->GetSize();
	jsonAccessorsArray->PushValue(jsonPositionAccessor);
	jsonPositionAccessor->SetValue("bufferView", new JsonInt(positionBufferViewNumber));
	jsonPositionAccessor->SetValue("byteOffset", new JsonInt(0));
	jsonPositionAccessor->SetValue("componentType", new JsonInt(MESH_NINJA_GLTF_ACCESSOR_DT_FLOAT));
	jsonPositionAccessor->SetValue("count", new JsonInt(mesh.vertexArray->size() * 3));
	jsonPositionAccessor->SetValue("type", new JsonString("VEC3"));

	JsonObject* jsonNormalAccessor = new JsonObject();
	int normalAccessorNumber = jsonAccessorsArray->GetSize();
	jsonAccessorsArray->PushValue(jsonNormalAccessor);
	jsonNormalAccessor->SetValue("bufferView", new JsonInt(normalBufferViewNumber));
	jsonNormalAccessor->SetValue("bufferOffset", new JsonInt(0));
	jsonNormalAccessor->SetValue("componentType", new JsonInt(MESH_NINJA_GLTF_ACCESSOR_DT_FLOAT));
	jsonNormalAccessor->SetValue("count", new JsonInt(mesh.vertexArray->size() * 3));
	jsonNormalAccessor->SetValue("type", new JsonString("VEC3"));

	JsonObject* jsonColorAccessor = new JsonObject();
	int colorAccessorNumber = jsonAccessorsArray->GetSize();
	jsonAccessorsArray->PushValue(jsonColorAccessor);
	jsonColorAccessor->SetValue("bufferView", new JsonInt(colorBufferViewNumber));
	jsonColorAccessor->SetValue("bufferOffset", new JsonInt(0));
	jsonColorAccessor->SetValue("componentType", new JsonInt(MESH_NINJA_GLTF_ACCESSOR_DT_FLOAT));
	jsonColorAccessor->SetValue("count", new JsonInt(mesh.vertexArray->size() * 3));
	jsonColorAccessor->SetValue("type", new JsonString("VEC3"));

	JsonObject* jsonAttributes = new JsonObject();
	jsonPrim->SetValue("attributes", jsonAttributes);
	jsonAttributes->SetValue("POSITION", new JsonInt(positionAccessorNumber));
	jsonAttributes->SetValue("NORMAL", new JsonInt(normalAccessorNumber));
	jsonAttributes->SetValue("COLOR", new JsonInt(colorAccessorNumber));

	return true;
}
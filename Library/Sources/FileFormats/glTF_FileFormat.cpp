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
		binFileStream.open(binFilePath, std::ios::binary);
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
		jsonRootObject->SetValue("accessors", jsonAccessorsArray);

		JsonArray* jsonMeshesArray = new JsonArray();
		jsonRootObject->SetValue("meshes", jsonMeshesArray);

		JsonObject* jsonMeshObject = new JsonObject();
		jsonMeshesArray->PushValue(jsonMeshObject);

		JsonArray* jsonPrimArray = new JsonArray();
		jsonMeshObject->SetValue("primitives", jsonPrimArray);

		JsonObject* jsonPrim = new JsonObject();
		jsonPrimArray->PushValue(jsonPrim);
		jsonPrim->SetValue("mode", new JsonInt(MESH_NINJA_GLTF_TOPO_MODE_TRIANGLES));
		jsonPrim->SetValue("material", new JsonInt(0));

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

		JsonArray* jsonScenesArray = new JsonArray();
		jsonRootObject->SetValue("scenes", jsonScenesArray);

		JsonObject* jsonSceneObject = new JsonObject();
		jsonScenesArray->PushValue(jsonSceneObject);

		JsonArray* jsonSceneNodesArray = new JsonArray();
		jsonSceneObject->SetValue("nodes", jsonSceneNodesArray);
		jsonSceneNodesArray->PushValue(new JsonInt(0));

		JsonArray* jsonNodesArray = new JsonArray();
		jsonRootObject->SetValue("nodes", jsonNodesArray);

		JsonObject* jsonNode = new JsonObject();
		jsonNodesArray->PushValue(jsonNode);
		jsonNode->SetValue("mesh", new JsonInt(0));

		JsonArray* jsonMaterialsArray = new JsonArray();
		jsonRootObject->SetValue("materials", jsonMaterialsArray);

		JsonObject* jsonMaterial = new JsonObject();
		jsonMaterialsArray->PushValue(jsonMaterial);
		
		JsonObject* jsonPBRMetallicRoughness = new JsonObject();
		jsonMaterial->SetValue("pbrMetallicRoughness", jsonPBRMetallicRoughness);
		jsonPBRMetallicRoughness->SetValue("baseColorFactor", new JsonArray(std::vector<double>{ 1.0, 1.0, 1.0, 1.0 }));
		jsonPBRMetallicRoughness->SetValue("metallicFactor", new JsonFloat(0.0));
		jsonPBRMetallicRoughness->SetValue("roughnessFactor", new JsonFloat(0.0));

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
	long offset = (long)binFileStream.tellp();

	for (const RenderMesh::Facet& facet : *mesh.facetArray)
	{
		for (int i = 0; i < (signed)facet.vertexArray->size(); i++)
		{
			uint32_t j = facet[i];
			this->WriteUnsignedInt(binFileStream, j);
		}
	}

	JsonObject* jsonBufferView = new JsonObject();
	int bufferViewNumber = jsonBufferViewsArray->GetSize();
	jsonBufferViewsArray->PushValue(jsonBufferView);
	jsonBufferView->SetValue("buffer", new JsonInt(0));
	jsonBufferView->SetValue("byteLength", new JsonInt(sizeof(uint32_t) * 3 * mesh.facetArray->size()));
	jsonBufferView->SetValue("byteOffset", new JsonInt(offset));
	jsonBufferView->SetValue("target", new JsonInt(MESH_NINJA_GLTF_ELEMENT_ARRAY_BUFFER));

	JsonObject* jsonAccessor = new JsonObject();
	int accessorNumber = jsonAccessorsArray->GetSize();
	jsonAccessorsArray->PushValue(jsonAccessor);
	jsonAccessor->SetValue("bufferView", new JsonInt(bufferViewNumber));
	jsonAccessor->SetValue("byteOffset", new JsonInt(0));
	jsonAccessor->SetValue("componentType", new JsonInt(MESH_NINJA_GLTF_ACCESSOR_DT_UNSIGNED_INT));
	jsonAccessor->SetValue("type", new JsonString("SCALAR"));
	jsonAccessor->SetValue("count", new JsonInt(mesh.facetArray->size() * 3));

	jsonPrim->SetValue("indices", new JsonInt(accessorNumber));

	return true;
}

bool glTF_FileFormat::WriteVertexBuffer(const RenderMesh& mesh, std::ofstream& binFileStream, JsonArray* jsonBufferViewsArray, JsonArray* jsonAccessorsArray, JsonObject* jsonPrim)
{
	// Note that I tried to interleave the vertex data (for better cache coherency), but
	// the glTF validator kept complaining about strides not to be used unless a buffer
	// view is into "raw vertex data".  Isn't all vertex data "raw"?

	long offset = (long)binFileStream.tellp();

	MeshNinja::Vector minPosition(DBL_MAX, DBL_MAX, DBL_MAX);
	MeshNinja::Vector maxPosition(DBL_MIN, DBL_MIN, DBL_MIN);

	for (const RenderMesh::Vertex& vertex : *mesh.vertexArray)
	{
		this->WriteFloat(binFileStream, (float)vertex.position.x);
		this->WriteFloat(binFileStream, (float)vertex.position.y);
		this->WriteFloat(binFileStream, (float)vertex.position.z);

		minPosition.Min(minPosition, vertex.position);
		maxPosition.Max(maxPosition, vertex.position);
	}

	JsonArray* jsonMinArray = new JsonArray();
	jsonMinArray->PushValue(new JsonFloat(minPosition.x));
	jsonMinArray->PushValue(new JsonFloat(minPosition.y));
	jsonMinArray->PushValue(new JsonFloat(minPosition.z));

	JsonArray* jsonMaxArray = new JsonArray();
	jsonMaxArray->PushValue(new JsonFloat(maxPosition.x));
	jsonMaxArray->PushValue(new JsonFloat(maxPosition.y));
	jsonMaxArray->PushValue(new JsonFloat(maxPosition.z));

	JsonObject* jsonPositionBufferView = new JsonObject();
	int positionBufferViewNumber = jsonBufferViewsArray->GetSize();
	jsonBufferViewsArray->PushValue(jsonPositionBufferView);
	jsonPositionBufferView->SetValue("buffer", new JsonInt(0));
	jsonPositionBufferView->SetValue("byteLength", new JsonInt(sizeof(float) * 3 * mesh.vertexArray->size()));
	jsonPositionBufferView->SetValue("byteOffset", new JsonInt(offset));
	jsonPositionBufferView->SetValue("target", new JsonInt(MESH_NINJA_GLTF_ARRAY_BUFFER));

	JsonObject* jsonPositionAccessor = new JsonObject();
	int positionAccessorNumber = jsonAccessorsArray->GetSize();
	jsonAccessorsArray->PushValue(jsonPositionAccessor);
	jsonPositionAccessor->SetValue("bufferView", new JsonInt(positionBufferViewNumber));
	jsonPositionAccessor->SetValue("byteOffset", new JsonInt(0));
	jsonPositionAccessor->SetValue("componentType", new JsonInt(MESH_NINJA_GLTF_ACCESSOR_DT_FLOAT));
	jsonPositionAccessor->SetValue("count", new JsonInt(mesh.vertexArray->size()));
	jsonPositionAccessor->SetValue("type", new JsonString("VEC3"));
	jsonPositionAccessor->SetValue("min", jsonMinArray);
	jsonPositionAccessor->SetValue("max", jsonMaxArray);

	offset = (long)binFileStream.tellp();

	for (const RenderMesh::Vertex& vertex : *mesh.vertexArray)
	{
		this->WriteFloat(binFileStream, (float)vertex.normal.x);
		this->WriteFloat(binFileStream, (float)vertex.normal.y);
		this->WriteFloat(binFileStream, (float)vertex.normal.z);
	}

	JsonObject* jsonNormalBufferView = new JsonObject();
	int normalBufferViewNumber = jsonBufferViewsArray->GetSize();
	jsonBufferViewsArray->PushValue(jsonNormalBufferView);
	jsonNormalBufferView->SetValue("buffer", new JsonInt(0));
	jsonNormalBufferView->SetValue("byteLength", new JsonInt(sizeof(float) * 3 * mesh.vertexArray->size()));
	jsonNormalBufferView->SetValue("byteOffset", new JsonInt(offset));
	jsonNormalBufferView->SetValue("target", new JsonInt(MESH_NINJA_GLTF_ARRAY_BUFFER));

	JsonObject* jsonNormalAccessor = new JsonObject();
	int normalAccessorNumber = jsonAccessorsArray->GetSize();
	jsonAccessorsArray->PushValue(jsonNormalAccessor);
	jsonNormalAccessor->SetValue("bufferView", new JsonInt(normalBufferViewNumber));
	jsonNormalAccessor->SetValue("byteOffset", new JsonInt(0));
	jsonNormalAccessor->SetValue("componentType", new JsonInt(MESH_NINJA_GLTF_ACCESSOR_DT_FLOAT));
	jsonNormalAccessor->SetValue("count", new JsonInt(mesh.vertexArray->size()));
	jsonNormalAccessor->SetValue("type", new JsonString("VEC3"));

	offset = (long)binFileStream.tellp();

	for (const RenderMesh::Vertex& vertex : *mesh.vertexArray)
	{
		this->WriteFloat(binFileStream, (float)vertex.color.x);
		this->WriteFloat(binFileStream, (float)vertex.color.y);
		this->WriteFloat(binFileStream, (float)vertex.color.z);
	}

	JsonObject* jsonColorBufferView = new JsonObject();
	int colorBufferViewNumber = jsonBufferViewsArray->GetSize();
	jsonBufferViewsArray->PushValue(jsonColorBufferView);
	jsonColorBufferView->SetValue("buffer", new JsonInt(0));
	jsonColorBufferView->SetValue("byteLength", new JsonInt(sizeof(float) * 3 * mesh.vertexArray->size()));
	jsonColorBufferView->SetValue("byteOffset", new JsonInt(offset));
	jsonColorBufferView->SetValue("target", new JsonInt(MESH_NINJA_GLTF_ARRAY_BUFFER));

	JsonObject* jsonColorAccessor = new JsonObject();
	int colorAccessorNumber = jsonAccessorsArray->GetSize();
	jsonAccessorsArray->PushValue(jsonColorAccessor);
	jsonColorAccessor->SetValue("bufferView", new JsonInt(colorBufferViewNumber));
	jsonColorAccessor->SetValue("byteOffset", new JsonInt(0));
	jsonColorAccessor->SetValue("componentType", new JsonInt(MESH_NINJA_GLTF_ACCESSOR_DT_FLOAT));
	jsonColorAccessor->SetValue("count", new JsonInt(mesh.vertexArray->size()));
	jsonColorAccessor->SetValue("type", new JsonString("VEC3"));

	JsonObject* jsonAttributes = new JsonObject();
	jsonPrim->SetValue("attributes", jsonAttributes);
	jsonAttributes->SetValue("POSITION", new JsonInt(positionAccessorNumber));
	jsonAttributes->SetValue("NORMAL", new JsonInt(normalAccessorNumber));
	jsonAttributes->SetValue("COLOR_0", new JsonInt(colorAccessorNumber));

	return true;
}

bool glTF_FileFormat::WriteFloat(std::ofstream& binFileStream, float value)
{
	return this->WriteWord(binFileStream, (const char*)&value);
}

bool glTF_FileFormat::WriteUnsignedInt(std::ofstream& binFileStream, unsigned int value)
{
	return this->WriteWord(binFileStream, (const char*)&value);
}

bool glTF_FileFormat::WriteWord(std::ofstream& binFileStream, const char* word)
{
	// Write the word in Little Endian style as per the glTF specification.
	binFileStream.write(&word[0], 1);
	binFileStream.write(&word[1], 1);
	binFileStream.write(&word[2], 1);
	binFileStream.write(&word[3], 1);

	return true;
}
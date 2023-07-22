#include "RenderMesh.h"
#include "Plane.h"

using namespace MeshNinja;

RenderMesh::RenderMesh()
{
	this->extraFaceDataArray = new std::vector<ExtraFaceData>();
	this->extraVertexDataArray = new std::vector<ExtraVertexData>();
}

/*virtual*/ RenderMesh::~RenderMesh()
{
	delete this->extraFaceDataArray;
	delete this->extraVertexDataArray;
}

/*virtual*/ void RenderMesh::ApplyTransform(const Transform& transform)
{
	ConvexPolygonMesh::ApplyTransform(transform);

	for (ExtraFaceData& faceData : *this->extraFaceDataArray)
	{
		faceData.normal = transform.TransformVector(faceData.normal);
		faceData.center = transform.TransformPosition(faceData.center);
	}

	for (ExtraVertexData& vertexData : *this->extraVertexDataArray)
	{
		vertexData.normal = transform.TransformVector(vertexData.normal);
	}
}

void RenderMesh::RegenerateNormals()
{
	this->FixArraySizes();

	for (int i = 0; i < (signed)this->extraFaceDataArray->size(); i++)
	{
		const Facet& facet = (*this->facetArray)[i];
		ExtraFaceData& faceData = (*this->extraFaceDataArray)[i];

		ConvexPolygon polygon;
		facet.MakePolygon(polygon, this);

		Plane plane;
		polygon.CalcPlane(plane);

		faceData.normal = plane.normal;
		faceData.center = Vector(0.0, 0.0, 0.0);
	}

	for (ExtraVertexData& vertexData : *this->extraVertexDataArray)
		vertexData.normal = Vector(0.0, 0.0, 0.0);

	for (int i = 0; i < (signed)this->extraFaceDataArray->size(); i++)
	{
		const Facet& facet = (*this->facetArray)[i];
		ExtraFaceData& faceData = (*this->extraFaceDataArray)[i];

		for (int j = 0; j < (signed)facet.vertexArray->size(); j++)
		{
			ExtraVertexData& vertexData = (*this->extraVertexDataArray)[facet[j]];
			vertexData.normal += faceData.normal;

			faceData.center += (*this->vertexArray)[facet[j]];
		}

		if (facet.vertexArray->size() != 0)
			faceData.center /= float(facet.vertexArray->size());
	}

	for (ExtraVertexData& vertexData : *this->extraVertexDataArray)
		vertexData.normal.Normalize();
}

void RenderMesh::FixArraySizes()
{
	while (this->extraFaceDataArray->size() < this->facetArray->size())
		this->extraFaceDataArray->push_back(ExtraFaceData{});

	while (this->extraFaceDataArray->size() > this->facetArray->size())
		this->extraFaceDataArray->pop_back();

	while (this->extraVertexDataArray->size() < this->vertexArray->size())
		this->extraVertexDataArray->push_back(ExtraVertexData{});

	while (this->extraVertexDataArray->size() > this->vertexArray->size())
		this->extraVertexDataArray->pop_back();
}
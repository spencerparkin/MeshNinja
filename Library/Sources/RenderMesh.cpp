#include "RenderMesh.h"
#include "Plane.h"
#include "ConvexPolygonMesh.h"

using namespace MeshNinja;

RenderMesh::RenderMesh()
{
	this->facetArray = new std::vector<Facet>();
	this->vertexArray = new std::vector<Vertex>();
}

/*virtual*/ RenderMesh::~RenderMesh()
{
	delete this->facetArray;
	delete this->vertexArray;
}

void RenderMesh::Clear()
{
	this->facetArray->clear();
	this->vertexArray->clear();
}

void RenderMesh::ApplyTransform(const Transform& transform)
{
	for (Facet& facet : *this->facetArray)
	{
		facet.normal = transform.TransformVector(facet.normal);
		facet.center = transform.TransformPosition(facet.center);
	}

	for (Vertex& vertex : *this->vertexArray)
	{
		vertex.position = transform.TransformPosition(vertex.position);
		vertex.normal = transform.TransformVector(vertex.normal);
	}
}

void RenderMesh::FromConvexPolygonMesh(const ConvexPolygonMesh& mesh)
{
	this->Clear();

	for (const Vector& vertex : *mesh.vertexArray)
	{
		Vertex renderVertex;
		
		renderVertex.position = vertex;
		renderVertex.normal = Vector(0.0, 0.0, 0.0);
		renderVertex.color = Vector(1.0, 1.0, 1.0);
		renderVertex.texCoords = Vector(0.0, 0.0, 0.0);

		this->vertexArray->push_back(renderVertex);
	}

	for (const ConvexPolygonMesh::Facet& facet : *mesh.facetArray)
	{
		Facet renderFacet;
		renderFacet.center = Vector(0.0, 0.0, 0.0);

		for (int i = 0; i < (signed)facet.vertexArray->size(); i++)
		{
			renderFacet.vertexArray->push_back(facet[i]);
			renderFacet.center += (*mesh.vertexArray)[facet[i]];
		}

		if (facet.vertexArray->size() > 0)
			renderFacet.center /= double(facet.vertexArray->size());

		ConvexPolygon polygon;
		facet.MakePolygon(polygon, &mesh);

		Plane plane;
		polygon.CalcPlane(plane);

		renderFacet.normal = plane.normal;
		renderFacet.color = Vector(1.0, 1.0, 1.0);

		this->facetArray->push_back(renderFacet);
	}

	for (Facet& facet : *this->facetArray)
	{
		for (int i = 0; i < (signed)facet.vertexArray->size(); i++)
		{
			Vertex& vertex = (*this->vertexArray)[facet[i]];
			vertex.normal += facet.normal;
		}
	}

	for (Vertex& vertex : *this->vertexArray)
		vertex.normal.Normalize();
}

void RenderMesh::ToConvexPolygonMesh(ConvexPolygonMesh& mesh) const
{
	// TODO: Write this.
}

RenderMesh::Facet::Facet()
{
	this->vertexArray = new std::vector<int>();
}

RenderMesh::Facet::Facet(const Facet& facet)
{
	this->vertexArray = new std::vector<int>();

	for (int i : *facet.vertexArray)
		this->vertexArray->push_back(i);

	this->color = facet.color;
	this->normal = facet.normal;
	this->center = facet.center;
}

/*virtual*/ RenderMesh::Facet::~Facet()
{
	delete this->vertexArray;
}

RenderMesh::Vertex::Vertex()
{
}

RenderMesh::Vertex::Vertex(const Vertex& vertex)
{
	this->position = vertex.position;
	this->normal = vertex.normal;
	this->color = vertex.color;
	this->texCoords = vertex.texCoords;
}

/*virtual*/ RenderMesh::Vertex::~Vertex()
{
}
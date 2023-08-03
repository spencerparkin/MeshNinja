#include "RenderMesh.h"
#include "Plane.h"
#include "ConvexPolygonMesh.h"
#include "AxisAlignedBoundingBox.h"

using namespace MeshNinja;

RenderMesh::RenderMesh()
{
	this->facetArray = new std::vector<Facet>();
	this->vertexArray = new std::vector<Vertex>();
}

RenderMesh::RenderMesh(const RenderMesh& renderMesh)
{
	this->facetArray = new std::vector<Facet>();
	this->vertexArray = new std::vector<Vertex>();

	this->Copy(renderMesh);
}

/*virtual*/ RenderMesh::~RenderMesh()
{
	delete this->facetArray;
	delete this->vertexArray;
}

void RenderMesh::Copy(const RenderMesh& renderMesh)
{
	this->Clear();

	for (const Vertex& vertex : *renderMesh.vertexArray)
		this->vertexArray->push_back(vertex);

	for (const Facet& facet : *renderMesh.facetArray)
		this->facetArray->push_back(facet);
}

void RenderMesh::Clear()
{
	this->facetArray->clear();
	this->vertexArray->clear();
}

bool RenderMesh::IsTriangleMesh() const
{
	for (const Facet& facet : *this->facetArray)
		if (facet.vertexArray->size() != 3)
			return false;

	return true;
}

void RenderMesh::SetColor(const Vector& color)
{
	for (Facet& facet : *this->facetArray)
		facet.color = color;

	for (Vertex& vertex : *this->vertexArray)
		vertex.color = color;
}

void RenderMesh::MakeRainbowColors()
{
	AxisAlignedBoundingBox box;
	if (this->CalcBoundingBox(box))
	{
		for (Vertex& vertex : *this->vertexArray)
		{
			box.CalcUVWs(vertex.position, vertex.color);
		}
	}
}

bool RenderMesh::CalcBoundingBox(AxisAlignedBoundingBox& box) const
{
	if (this->vertexArray->size() == 0)
		return false;

	box.min = (*this->vertexArray)[0].position;
	box.max = box.min;

	for (const Vertex& vertex : *this->vertexArray)
		box.ExpandToIncludePoint(vertex.position);

	return true;
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

void RenderMesh::FromConvexPolygonMesh(const ConvexPolygonMesh& mesh, const Options& options)
{
	this->Clear();

	int vertexCounter = 0;

	for (const ConvexPolygonMesh::Facet& facet : *mesh.facetArray)
	{
		Facet renderFacet;
		renderFacet.center = Vector(0.0, 0.0, 0.0);

		for (int i = 0; i < (signed)facet.vertexArray->size(); i++)
		{
			if (options.normalType == Options::NormalType::VERTEX_BASED)
				renderFacet.vertexArray->push_back(facet[i]);
			else if (options.normalType == Options::NormalType::FACET_BASED)
				renderFacet.vertexArray->push_back(vertexCounter++);

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

	if (options.normalType == Options::NormalType::VERTEX_BASED)
	{
		for (const Vector& vertex : *mesh.vertexArray)
		{
			Vertex renderVertex;

			renderVertex.position = vertex;
			renderVertex.normal = Vector(0.0, 0.0, 0.0);
			renderVertex.color = options.color;
			renderVertex.texCoords = Vector(0.0, 0.0, 0.0);

			this->vertexArray->push_back(renderVertex);
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
	else if (options.normalType == Options::NormalType::FACET_BASED)
	{
		for (int i = 0; i < (signed)mesh.facetArray->size(); i++)
		{
			const ConvexPolygonMesh::Facet& facet = (*mesh.facetArray)[i];
			const Facet& renderFacet = (*this->facetArray)[i];

			for (int j = 0; j < (signed)facet.vertexArray->size(); j++)
			{
				const Vector& vertex = (*mesh.vertexArray)[facet[j]];

				Vertex renderVertex;

				renderVertex.position = vertex;
				renderVertex.normal = renderFacet.normal;
				renderVertex.color = options.color;
				renderVertex.texCoords = Vector(0.0, 0.0, 0.0);

				this->vertexArray->push_back(renderVertex);
			}
		}
	}
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
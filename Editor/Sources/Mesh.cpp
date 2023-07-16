#include "Mesh.h"
#include "MeshFileFormat.h"
#include "ConvexPolygonMesh.h"
#include "Plane.h"
#include "Camera.h"

Mesh::Mesh()
{
	this->isSelected = false;
	this->renderMeshDirty = true;
	this->color.x = 1.0;
	this->color.y = 0.0;
	this->color.z = 0.0;
}

/*virtual*/ Mesh::~Mesh()
{
}

void Mesh::Render(GLint renderMode, const Camera* camera) const
{
	if (this->renderMeshDirty)
	{
		this->renderMeshDirty = false;
		this->renderMesh.Copy(this->mesh);
		this->renderMesh.TessellateFaces();
	}

	glBegin(GL_TRIANGLES);
	glColor3d(this->color.x, this->color.y, this->color.z);

	for (const MeshNinja::ConvexPolygonMesh::Facet& facet : *this->renderMesh.facetArray)
	{
		if (facet.vertexArray->size() == 3)
		{
			MeshNinja::ConvexPolygon polygon;
			facet.MakePolygon(polygon, &this->mesh);

			MeshNinja::Plane plane;
			polygon.CalcPlane(plane);

			for (int i : *facet.vertexArray)
			{
				const MeshNinja::Vector& vertex = (*this->renderMesh.vertexArray)[i];
				glVertex3d(vertex.x, vertex.y, vertex.z);
				glNormal3d(plane.normal.x, plane.normal.y, plane.normal.z);
			}
		}
	}

	glEnd();

	// This will draw each edge twice, but whatever.
	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 0.0f);

	for (const MeshNinja::ConvexPolygonMesh::Facet& facet : *this->mesh.facetArray)
	{
		for (int i = 0; i < (signed)facet.vertexArray->size(); i++)
		{
			int j = (i + 1) % facet.vertexArray->size();

			MeshNinja::ConvexPolygon polygon;
			facet.MakePolygon(polygon, &this->mesh);

			MeshNinja::Plane plane;
			polygon.CalcPlane(plane);

			MeshNinja::Vector center = polygon.CalcCenter();

			// I don't know why lines aren't being Z-tested, so do our own test here.
			if (plane.normal.AngleBetweenThisAnd(center - camera->position) > MESH_NINJA_PI / 2.0)
			{
				const MeshNinja::Vector& vertexA = (*this->mesh.vertexArray)[(*facet.vertexArray)[i]];
				const MeshNinja::Vector& vertexB = (*this->mesh.vertexArray)[(*facet.vertexArray)[j]];

				glVertex3d(vertexA.x, vertexA.y, vertexA.z);
				glVertex3d(vertexB.x, vertexB.y, vertexB.z);
			}
		}
	}

	glEnd();
}

bool Mesh::Load()
{
	MeshNinja::MeshFileFormat* fileFormat = this->MakeFileFormatObject();
	if (!fileFormat)
		return false;

	bool success = fileFormat->LoadMesh((const char*)this->fileSource.c_str(), this->mesh);
	delete fileFormat;
	return success;
}

bool Mesh::Save()
{
	MeshNinja::MeshFileFormat* fileFormat = this->MakeFileFormatObject();
	if (!fileFormat)
		return false;

	bool success = fileFormat->SaveMesh((const char*)this->fileSource.c_str(), this->mesh);
	delete fileFormat;
	return success;
}

MeshNinja::MeshFileFormat* Mesh::MakeFileFormatObject()
{
	// TODO: Look at extension to know what to return here.
	return new MeshNinja::ObjFileFormat();
}
#include "Mesh.h"
#include "MeshFileFormat.h"
#include "ConvexPolygonMesh.h"
#include "Plane.h"
#include "Camera.h"
#include "MeshGraph.h"

Mesh::Mesh()
{
	this->isSelected = false;
	this->isVisible = true;
	this->renderMeshDirty = true;
	this->transform.SetIdentity();

	static int i = 0;
	switch (i)
	{
		case 0:
		{
			this->color.x = 1.0;
			this->color.y = 0.0;
			this->color.z = 0.0;
			break;
		}
		case 1:
		{
			this->color.x = 0.0;
			this->color.y = 1.0;
			this->color.z = 0.0;
			break;
		}
		case 2:
		{
			this->color.x = 0.0;
			this->color.y = 0.0;
			this->color.z = 1.0;
			break;
		}
		case 3:
		{
			this->color.x = 1.0;
			this->color.y = 1.0;
			this->color.z = 0.0;
			break;
		}
		case 4:
		{
			this->color.x = 1.0;
			this->color.y = 0.0;
			this->color.z = 1.0;
			break;
		}
		case 5:
		{
			this->color.x = 0.0;
			this->color.y = 1.0;
			this->color.z = 1.0;
			break;
		}
	}

	i = (i + 1) % 6;
}

/*virtual*/ Mesh::~Mesh()
{
}

/*virtual*/ void Mesh::Render(GLint renderMode, const Camera* camera) const
{
	if (!this->isVisible)
		return;

	if (this->renderMeshDirty)
	{
		this->renderMeshDirty = false;
		this->renderMesh.Copy(this->mesh);
		this->renderMesh.TessellateFaces();
	}

	if (renderMode == GL_SELECT)
		glPushName(this->id);

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
				MeshNinja::Vector vertex = (*this->renderMesh.vertexArray)[i];

				MeshNinja::Vector point = this->transform.TransformPosition(vertex);
				MeshNinja::Vector normal = this->transform.TransformVector(plane.normal);

				glVertex3d(point.x, point.y, point.z);
				glNormal3d(normal.x, normal.y, normal.z);
			}
		}
	}

	glEnd();

	if (renderMode == GL_SELECT)
		glPopName();

	if (renderMode == GL_RENDER)
	{
		glBegin(GL_LINES);

		if (this->isSelected)
			glColor3f(1.0f, 1.0f, 1.0f);
		else
			glColor3f(0.3f, 0.3f, 0.3f);

		std::set<MeshNinja::MeshGraph::VertexPair> vertexPairSet;

		for (const MeshNinja::ConvexPolygonMesh::Facet& facet : *this->mesh.facetArray)
		{
			for (int i = 0; i < (signed)facet.vertexArray->size(); i++)
			{
				int j = (i + 1) % facet.vertexArray->size();

				MeshNinja::MeshGraph::VertexPair pair{ (*facet.vertexArray)[i], (*facet.vertexArray)[j] };
				if (vertexPairSet.find(pair) == vertexPairSet.end())
				{
					vertexPairSet.insert(pair);

					MeshNinja::Vector vertexA = (*this->mesh.vertexArray)[(*facet.vertexArray)[i]];
					MeshNinja::Vector vertexB = (*this->mesh.vertexArray)[(*facet.vertexArray)[j]];

					vertexA = this->transform.TransformPosition(vertexA);
					vertexB = this->transform.TransformPosition(vertexB);

					MeshNinja::Vector lineOfSightDirection = (vertexA + vertexB) / 2.0 - camera->position;
					MeshNinja::Vector offset = lineOfSightDirection.Normalized() * -0.05;

					// This is to prevent Z-fighting.
					vertexA += offset;
					vertexB += offset;

					glVertex3d(vertexA.x, vertexA.y, vertexA.z);
					glVertex3d(vertexB.x, vertexB.y, vertexB.z);
				}
			}
		}

		glEnd();
	}
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

	MeshNinja::ConvexPolygonMesh transformedMesh(this->mesh);
	transformedMesh.ApplyTransform(this->transform);

	bool success = fileFormat->SaveMesh((const char*)this->fileSource.c_str(), transformedMesh);
	delete fileFormat;
	return success;
}

void Mesh::BakeTransform()
{
	this->mesh.ApplyTransform(this->transform);
	this->transform.SetIdentity();
	this->renderMeshDirty = true;
}

MeshNinja::MeshFileFormat* Mesh::MakeFileFormatObject()
{
	// TODO: Look at extension to know what to return here.
	return new MeshNinja::ObjFileFormat();
}

void Mesh::SetSelected(bool selected) const
{
	this->isSelected = selected;
	if (selected)
		this->selectionTime = wxDateTime::Now();
}

bool Mesh::GetSelected() const
{
	return this->isSelected;
}

void Mesh::SetVisible(bool visible) const
{
	this->isVisible = visible;
}

bool Mesh::GetVisible() const
{
	return this->isVisible;
}
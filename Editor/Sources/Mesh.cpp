#include "Mesh.h"
#include "MeshFileFormat.h"
#include "ConvexPolygonMesh.h"
#include "Plane.h"
#include "Camera.h"
#include "MeshGraph.h"
#include "Application.h"
#include "Ray.h"

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

/*virtual*/ MeshNinja::Vector Mesh::GetPosition() const
{
	return this->transform.TransformPosition(this->mesh.CalcCenter());
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

	if (wxGetApp().lightingMode == Application::LightingMode::LIT)
	{
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
	}

	glBegin(GL_TRIANGLES);

	MeshNinja::Vector renderedColor(color);
	if (this->isSelected)
	{
		renderedColor *= 1.1;
		renderedColor.x = MESH_NINJA_CLAMP(renderedColor.x, 0.0, 1.0);
		renderedColor.y = MESH_NINJA_CLAMP(renderedColor.y, 0.0, 1.0);
		renderedColor.z = MESH_NINJA_CLAMP(renderedColor.z, 0.0, 1.0);
	}
	
	glColor3d(renderedColor.x, renderedColor.y, renderedColor.z);

	std::vector<MeshNinja::Ray> faceNormalArray;

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

				if (wxGetApp().renderFaceNormals)
					faceNormalArray.push_back(MeshNinja::Ray(polygon.CalcCenter(), normal));
			}
		}
	}

	glEnd();

	if (wxGetApp().lightingMode == Application::LightingMode::LIT)
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
	}

	if (renderMode == GL_SELECT)
		glPopName();

	if (renderMode == GL_RENDER && wxGetApp().renderFaceNormals)
	{
		glBegin(GL_LINES);
		glColor3f(1.0f, 1.0f, 1.0f);

		for (const MeshNinja::Ray& ray : faceNormalArray)
		{
			MeshNinja::Vector pointA = ray.origin;
			MeshNinja::Vector pointB = ray.origin + ray.direction;

			glVertex3dv(&pointA.x);
			glVertex3dv(&pointB.x);
		}

		glEnd();
	}

	if (renderMode == GL_RENDER && wxGetApp().renderEdges)
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
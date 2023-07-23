#include "Mesh.h"
#include "MeshFileFormat.h"
#include "ConvexPolygonMesh.h"
#include "Plane.h"
#include "Camera.h"
#include "MeshGraph.h"
#include "Application.h"

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
		this->renderMesh.RegenerateNormals();
	}

	if (renderMode == GL_SELECT)
		glPushName(this->id);

	if (wxGetApp().lightingMode != Application::LightingMode::UNLIT)
	{
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
	}

	glBegin(GL_TRIANGLES);

	MeshNinja::Vector renderedColor(color);
	if (this->isSelected)
	{
		renderedColor += MeshNinja::Vector(0.2, 0.2, 0.2);
		renderedColor.x = MESH_NINJA_CLAMP(renderedColor.x, 0.0, 1.0);
		renderedColor.y = MESH_NINJA_CLAMP(renderedColor.y, 0.0, 1.0);
		renderedColor.z = MESH_NINJA_CLAMP(renderedColor.z, 0.0, 1.0);
	}
	
	if (wxGetApp().lightingMode == Application::LightingMode::UNLIT)
		glColor3d(renderedColor.x, renderedColor.y, renderedColor.z);
	else
	{
		GLfloat diffuseColor[] = { (GLfloat)renderedColor.x, (GLfloat)renderedColor.y, (GLfloat)renderedColor.z, 1.0f };
		GLfloat specularColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		GLfloat ambientColor[] = { (GLfloat)renderedColor.x, (GLfloat)renderedColor.y, (GLfloat)renderedColor.z, 1.0f };
		GLfloat shininess[] = { 30.0f };

		glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);
		glMaterialfv(GL_FRONT, GL_SPECULAR, specularColor);
		glMaterialfv(GL_FRONT, GL_AMBIENT, ambientColor);
		glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

		glMaterialfv(GL_BACK, GL_DIFFUSE, diffuseColor);
		glMaterialfv(GL_BACK, GL_SPECULAR, specularColor);
		glMaterialfv(GL_BACK, GL_AMBIENT, ambientColor);
		glMaterialfv(GL_BACK, GL_SHININESS, shininess);
	}

	for(int i = 0; i < (signed)this->renderMesh.facetArray->size(); i++)
	{
		const MeshNinja::RenderMesh::Facet& facet = (*this->renderMesh.facetArray)[i];
		const MeshNinja::RenderMesh::ExtraFaceData& faceData = (*this->renderMesh.extraFaceDataArray)[i];

		if (facet.vertexArray->size() == 3)
		{
			if (wxGetApp().lightingMode == Application::LightingMode::FACE_LIT)
			{
				const MeshNinja::Vector faceNormal = this->transform.TransformVector(faceData.normal);
				glNormal3d(faceNormal.x, faceNormal.y, faceNormal.z);
			}

			for (int j : *facet.vertexArray)
			{
				const MeshNinja::Vector vertex = this->transform.TransformPosition((*this->renderMesh.vertexArray)[j]);
				const MeshNinja::RenderMesh::ExtraVertexData& vertexData = (*this->renderMesh.extraVertexDataArray)[j];

				if (wxGetApp().lightingMode == Application::LightingMode::VERTEX_LIT)
				{
					const MeshNinja::Vector vertexNormal = this->transform.TransformVector(vertexData.normal);
					glNormal3d(vertexNormal.x, vertexNormal.y, vertexNormal.z);
				}

				glVertex3d(vertex.x, vertex.y, vertex.z);
			}
		}
	}

	glEnd();

	if (wxGetApp().lightingMode != Application::LightingMode::UNLIT)
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

		for (const MeshNinja::RenderMesh::ExtraFaceData& faceData : *this->renderMesh.extraFaceDataArray)
		{
			const MeshNinja::Vector pointA = this->transform.TransformPosition(faceData.center);
			const MeshNinja::Vector pointB = pointA + this->transform.TransformVector(faceData.normal);

			glVertex3dv(&pointA.x);
			glVertex3dv(&pointB.x);
		}

		glEnd();
	}

	if (renderMode == GL_RENDER && wxGetApp().renderVertexNormals)
	{
		glBegin(GL_LINES);
		glColor3f(1.0f, 1.0f, 1.0f);

		for (int i = 0; i < (signed)this->renderMesh.vertexArray->size(); i++)
		{
			const MeshNinja::RenderMesh::ExtraVertexData& vertexData = (*this->renderMesh.extraVertexDataArray)[i];

			const MeshNinja::Vector pointA = this->transform.TransformPosition((*this->renderMesh.vertexArray)[i]);
			const MeshNinja::Vector pointB = pointA + this->transform.TransformVector(vertexData.normal);

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

		std::set<MeshNinja::MeshGraph::VertexPair<false>> vertexPairSet;

		for (const MeshNinja::ConvexPolygonMesh::Facet& facet : *this->mesh.facetArray)
		{
			for (int i = 0; i < (signed)facet.vertexArray->size(); i++)
			{
				int j = (i + 1) % facet.vertexArray->size();

				MeshNinja::MeshGraph::VertexPair<false> pair{ (*facet.vertexArray)[i], (*facet.vertexArray)[j] };
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
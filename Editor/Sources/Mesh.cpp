#include "Mesh.h"
#include "FileFormats/ObjFileFormat.h"
#include "FileFormats/glTF_FileFormat.h"
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

void Mesh::DirtyRenderFlag() const
{
	this->renderMeshDirty = true;
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
		
		MeshNinja::ConvexPolygonMesh tessellatedMesh(this->mesh);
		tessellatedMesh.TessellateFaces();

		this->renderMesh.FromConvexPolygonMesh(tessellatedMesh);

		this->meshGraph.Generate(this->mesh);
	}

	if (renderMode == GL_SELECT)
		glPushName(this->id);

	if (wxGetApp().lightingMode != Application::LightingMode::UNLIT)
	{
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
	}

	glBegin(GL_TRIANGLES);

	if (wxGetApp().lightingMode == Application::LightingMode::UNLIT)
		glColor3d(this->color.x, this->color.y, this->color.z);
	else
	{
		GLfloat diffuseColor[] = { (GLfloat)this->color.x, (GLfloat)this->color.y, (GLfloat)this->color.z, 1.0f };
		GLfloat specularColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		GLfloat ambientColor[] = { (GLfloat)this->color.x, (GLfloat)this->color.y, (GLfloat)this->color.z, 1.0f };
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

		if (facet.vertexArray->size() == 3)
		{
			if (wxGetApp().lightingMode == Application::LightingMode::FACE_LIT)
			{
				MeshNinja::Vector faceNormal = this->transform.TransformVector(facet.normal);
				faceNormal.Normalize();
				glNormal3d(faceNormal.x, faceNormal.y, faceNormal.z);
			}

			for (int j : *facet.vertexArray)
			{
				const MeshNinja::RenderMesh::Vertex& vertex = (*this->renderMesh.vertexArray)[j];
				MeshNinja::Vector vertexPosition = this->transform.TransformPosition(vertex.position);

				if (wxGetApp().lightingMode == Application::LightingMode::VERTEX_LIT)
				{
					MeshNinja::Vector vertexNormal = this->transform.TransformVector(vertex.normal);
					vertexNormal.Normalize();
					glNormal3d(vertexNormal.x, vertexNormal.y, vertexNormal.z);
				}

				glVertex3d(vertexPosition.x, vertexPosition.y, vertexPosition.z);
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

	glLineWidth(1.0f);

	if (renderMode == GL_RENDER && wxGetApp().renderFaceNormals)
	{
		glBegin(GL_LINES);
		glColor3f(1.0f, 1.0f, 1.0f);

		for (const MeshNinja::RenderMesh::Facet& facet : *this->renderMesh.facetArray)
		{
			const MeshNinja::Vector pointA = this->transform.TransformPosition(facet.center);
			const MeshNinja::Vector pointB = pointA + this->transform.TransformVector(facet.normal);

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
			const MeshNinja::RenderMesh::Vertex& vertex = (*this->renderMesh.vertexArray)[i];

			const MeshNinja::Vector pointA = this->transform.TransformPosition(vertex.position);
			const MeshNinja::Vector pointB = pointA + this->transform.TransformVector(vertex.normal);

			glVertex3dv(&pointA.x);
			glVertex3dv(&pointB.x);
		}

		glEnd();
	}

	if (renderMode == GL_RENDER && wxGetApp().renderEdges)
	{
		glBegin(GL_LINES);
		glColor3f(0.7f, 0.7f, 0.7f);

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
					MeshNinja::Vector offset = lineOfSightDirection.Normalized() * -0.01;

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

	if (renderMode == GL_RENDER && this->isSelected)
	{
		std::set<MeshNinja::MeshGraph::VertexPair<false>> edgeSet;
		this->meshGraph.CollectSilhouetteEdges(camera->position, edgeSet, this->transform);

		glLineWidth(4.0f);
		glBegin(GL_LINES);
		glColor3d(1.0, 1.0, 1.0);

		for (MeshNinja::MeshGraph::VertexPair<false> pair : edgeSet)
		{
			MeshNinja::Vector vertexA = this->transform.TransformPosition((*this->mesh.vertexArray)[pair.i]);
			MeshNinja::Vector vertexB = this->transform.TransformPosition((*this->mesh.vertexArray)[pair.j]);

			glVertex3dv(&vertexA.x);
			glVertex3dv(&vertexB.x);
		}

		glEnd();
		glLineWidth(1.0f);
	}
}

bool Mesh::Load(bool loadRenderMesh /*= false*/)
{
	bool success = false;

	MeshNinja::MeshFileFormat* fileFormat = this->GetFileFormatObject();
	if (fileFormat)
	{
		if (loadRenderMesh)
		{
			success = fileFormat->LoadRenderMesh((const char*)this->fileSource.c_str(), this->renderMesh);
			if (success)
			{
				this->renderMeshDirty = false;
				this->renderMesh.ToConvexPolygonMesh(this->mesh);
			}
		}
		else
		{
			success = fileFormat->LoadMesh((const char*)this->fileSource.c_str(), this->mesh);
			this->renderMeshDirty = true;
		}
	}

	return success;
}

bool Mesh::Save(bool saveRenderMesh /*= false*/)
{
	bool success = false;

	MeshNinja::MeshFileFormat* fileFormat = this->GetFileFormatObject();
	if (fileFormat)
	{
		if (saveRenderMesh)
		{
			MeshNinja::RenderMesh transformedMesh(this->renderMesh);
			transformedMesh.ApplyTransform(this->transform);
			transformedMesh.SetColor(this->color);
			success = fileFormat->SaveRenderMesh((const char*)this->fileSource.c_str(), transformedMesh);
		}
		else
		{
			MeshNinja::ConvexPolygonMesh transformedMesh(this->mesh);
			transformedMesh.ApplyTransform(this->transform);
			success = fileFormat->SaveMesh((const char*)this->fileSource.c_str(), transformedMesh);
		}
	}

	return success;
}

void Mesh::BakeTransform()
{
	this->mesh.ApplyTransform(this->transform);
	this->transform.SetIdentity();
	this->renderMeshDirty = true;
}

MeshNinja::MeshFileFormat* Mesh::GetFileFormatObject()
{
	std::string ext = std::filesystem::path(std::string(this->fileSource.c_str())).extension().string();

	static MeshNinja::ObjFileFormat objFileFormat;
	static MeshNinja::glTF_FileFormat gltfFileFormat;

	std::vector<MeshNinja::MeshFileFormat*> fileFormatArray;

	fileFormatArray.push_back(&objFileFormat);
	fileFormatArray.push_back(&gltfFileFormat);

	for (MeshNinja::MeshFileFormat* fileFormat : fileFormatArray)
		if (fileFormat->GetExtension() == ext)
			return fileFormat;

	return nullptr;
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
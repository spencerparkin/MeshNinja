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

/*virtual*/ MeshNinja::Vector3 Mesh::GetPosition() const
{
	return this->transform.TransformPosition(this->mesh.CalcCenter());
}

void Mesh::IssueColor(const MeshNinja::Vector3& givenColor) const
{
	if (wxGetApp().lightingMode == Application::LightingMode::UNLIT)
		glColor3d(givenColor.x, givenColor.y, givenColor.z);
	else
	{
		GLfloat diffuseColor[] = { (GLfloat)givenColor.x, (GLfloat)givenColor.y, (GLfloat)givenColor.z, 1.0f };
		GLfloat specularColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		GLfloat ambientColor[] = { (GLfloat)givenColor.x, (GLfloat)givenColor.y, (GLfloat)givenColor.z, 1.0f };
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

		MeshNinja::RenderMesh::Options options;
		options.normalType = MeshNinja::RenderMesh::Options::NormalType::FACET_BASED;

		this->renderMesh.FromConvexPolygonMesh(tessellatedMesh, options);
		this->renderMesh.MakeRainbowColors();

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

	if (wxGetApp().coloringMode == Application::ColoringMode::USE_MESH_COLOR)
		this->IssueColor(this->color);

	MeshNinja::Matrix3x3 matrixInvT;
	matrixInvT.SetInverseTranspose(this->transform.matrix);

	for(int i = 0; i < (signed)this->renderMesh.facetArray->size(); i++)
	{
		const MeshNinja::RenderMesh::Facet& facet = (*this->renderMesh.facetArray)[i];

		if (facet.vertexArray->size() == 3)
		{
			if (wxGetApp().coloringMode == Application::ColoringMode::USE_FACE_COLORS)
				this->IssueColor(facet.color);

			if (wxGetApp().lightingMode == Application::LightingMode::FACE_LIT)
			{
				MeshNinja::Vector3 faceNormal;
				matrixInvT.MultiplyRight(facet.normal, faceNormal);
				faceNormal.Normalize();
				glNormal3d(faceNormal.x, faceNormal.y, faceNormal.z);
			}

			for (int j : *facet.vertexArray)
			{
				const MeshNinja::RenderMesh::Vertex& vertex = (*this->renderMesh.vertexArray)[j];
				MeshNinja::Vector3 vertexPosition = this->transform.TransformPosition(vertex.position);

				if (wxGetApp().coloringMode == Application::ColoringMode::USE_VERTEX_COLORS)
					this->IssueColor(vertex.color);

				if (wxGetApp().lightingMode == Application::LightingMode::VERTEX_LIT)
				{
					MeshNinja::Vector3 vertexNormal;
					matrixInvT.MultiplyRight(vertex.normal, vertexNormal);
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
			MeshNinja::Vector3 faceNormal;
			matrixInvT.MultiplyRight(facet.normal, faceNormal);

			const MeshNinja::Vector3 pointA = this->transform.TransformPosition(facet.center);
			const MeshNinja::Vector3 pointB = pointA + faceNormal;

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

			MeshNinja::Vector3 vertexNormal;
			matrixInvT.MultiplyRight(vertex.normal, vertexNormal);

			const MeshNinja::Vector3 pointA = this->transform.TransformPosition(vertex.position);
			const MeshNinja::Vector3 pointB = pointA + vertexNormal;

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

					MeshNinja::Vector3 vertexA = (*this->mesh.vertexArray)[(*facet.vertexArray)[i]];
					MeshNinja::Vector3 vertexB = (*this->mesh.vertexArray)[(*facet.vertexArray)[j]];

					vertexA = this->transform.TransformPosition(vertexA);
					vertexB = this->transform.TransformPosition(vertexB);

					MeshNinja::Vector3 lineOfSightDirection = (vertexA + vertexB) / 2.0 - camera->position;
					MeshNinja::Vector3 offset = lineOfSightDirection.Normalized() * -0.01;

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
		// TODO: I think we actually need to send in a transform that takes us form object space all the way to projective space for this to really work.
		//       In that case, the view direction would always just be the -Z-axis, I think.
		std::set<MeshNinja::MeshGraph::VertexPair<false>> edgeSet;
		this->meshGraph.CollectSilhouetteEdges(camera->GetViewDirection(), edgeSet, this->transform);

		glLineWidth(4.0f);
		glBegin(GL_LINES);
		glColor3d(1.0, 1.0, 1.0);

		for (MeshNinja::MeshGraph::VertexPair<false> pair : edgeSet)
		{
			MeshNinja::Vector3 vertexA = this->transform.TransformPosition((*this->mesh.vertexArray)[pair.i]);
			MeshNinja::Vector3 vertexB = this->transform.TransformPosition((*this->mesh.vertexArray)[pair.j]);

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
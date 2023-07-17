#pragma once

#include <wx/string.h>
#include <wx/glcanvas.h>
#include "ConvexPolygonMesh.h"
#include "MeshFileFormat.h"
#include "Scene.h"
#include "Transform.h"

class Camera;

class Mesh : public Scene::Object
{
public:
	Mesh();
	virtual ~Mesh();

	bool Load();
	bool Save();

	virtual void Render(GLint renderMode, const Camera* camera) const override;

	void BakeTransform();

	MeshNinja::MeshFileFormat* MakeFileFormatObject();

	MeshNinja::Vector color;
	MeshNinja::ConvexPolygonMesh mesh;
	MeshNinja::Transform transform;
	mutable MeshNinja::ConvexPolygonMesh renderMesh;
	wxString fileSource;
	mutable bool isSelected;
	mutable bool isVisible;
	mutable bool renderMeshDirty;
};
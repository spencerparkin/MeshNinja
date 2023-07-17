#pragma once

#include <wx/string.h>
#include <wx/glcanvas.h>
#include "ConvexPolygonMesh.h"
#include "MeshFileFormat.h"
#include "Scene.h"

class Camera;

class Mesh : public Scene::Object
{
public:
	Mesh();
	virtual ~Mesh();

	bool Load();
	bool Save();

	virtual void Render(GLint renderMode, const Camera* camera) const override;

	MeshNinja::Vector color;
	MeshNinja::ConvexPolygonMesh mesh;
	mutable MeshNinja::ConvexPolygonMesh renderMesh;
	wxString fileSource;
	mutable bool isSelected;
	mutable bool renderMeshDirty;

	MeshNinja::MeshFileFormat* MakeFileFormatObject();
};
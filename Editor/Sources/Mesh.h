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

	void SetSelected(bool selected) const;
	bool GetSelected() const;

	void SetVisible(bool visible) const;
	bool GetVisible() const;

	MeshNinja::MeshFileFormat* MakeFileFormatObject();

	MeshNinja::Vector color;
	MeshNinja::ConvexPolygonMesh mesh;
	MeshNinja::Transform transform;
	wxString fileSource;

private:

	mutable MeshNinja::ConvexPolygonMesh renderMesh;
	mutable bool isSelected;
	mutable bool isVisible;
	mutable bool renderMeshDirty;
	mutable wxDateTime selectionTime;
};
#pragma once

#include <wx/string.h>
#include <wx/glcanvas.h>
#include <wx/datetime.h>
#include "RenderMesh.h"
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
	virtual MeshNinja::Vector GetPosition() const override;

	void BakeTransform();

	void SetSelected(bool selected) const;
	bool GetSelected() const;

	void SetVisible(bool visible) const;
	bool GetVisible() const;

	const wxDateTime& GetSelectionTime() const { return this->selectionTime; }

	MeshNinja::MeshFileFormat* MakeFileFormatObject();

	MeshNinja::Vector color;
	MeshNinja::ConvexPolygonMesh mesh;
	MeshNinja::Transform transform;
	wxString fileSource;

private:

	mutable MeshNinja::RenderMesh renderMesh;
	mutable bool isSelected;
	mutable bool isVisible;
	mutable bool renderMeshDirty;
	mutable wxDateTime selectionTime;
};
#pragma once

#include <wx/string.h>
#include <wx/glcanvas.h>
#include <wx/datetime.h>
#include "RenderMesh.h"
#include "ConvexPolygonMesh.h"
#include "MeshFileFormat.h"
#include "MeshGraph.h"
#include "Scene.h"
#include "Transform.h"

class Camera;

class Mesh : public Scene::Object
{
public:
	Mesh();
	virtual ~Mesh();

	bool Load(bool loadRenderMesh = false);
	bool Save(bool saveRenderMesh = false);

	virtual void Render(GLint renderMode, const Camera* camera) const override;
	virtual MeshNinja::Vector GetPosition() const override;

	void BakeTransform();

	void SetSelected(bool selected) const;
	bool GetSelected() const;

	void SetVisible(bool visible) const;
	bool GetVisible() const;

	void DirtyRenderFlag() const;

	const wxDateTime& GetSelectionTime() const { return this->selectionTime; }

	MeshNinja::MeshFileFormat* GetFileFormatObject();

	MeshNinja::Vector color;
	MeshNinja::ConvexPolygonMesh mesh;
	MeshNinja::Transform transform;
	wxString fileSource;

private:

	void IssueColor(const MeshNinja::Vector& givenColor) const;

	mutable MeshNinja::RenderMesh renderMesh;
	mutable MeshNinja::MeshGraph meshGraph;
	mutable bool isSelected;
	mutable bool isVisible;
	mutable bool renderMeshDirty;
	mutable wxDateTime selectionTime;
};
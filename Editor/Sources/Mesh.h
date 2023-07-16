#pragma once

#include <wx/string.h>
#include <wx/glcanvas.h>
#include "ConvexPolygonMesh.h"
#include "MeshFileFormat.h"
#include "MeshGraph.h"

class Camera;

class Mesh
{
public:
	Mesh();
	virtual ~Mesh();

	bool Load();
	bool Save();

	void Render(GLint renderMode, const Camera* camera) const;

	MeshNinja::Vector color;
	MeshNinja::ConvexPolygonMesh mesh;
	mutable MeshNinja::ConvexPolygonMesh renderMesh;
	mutable MeshNinja::MeshGraph renderMeshGraph;
	wxString fileSource;
	bool isSelected;
	mutable bool renderMeshDirty;

	MeshNinja::MeshFileFormat* MakeFileFormatObject();
};
#pragma once

#include <wx/string.h>
#include <wx/glcanvas.h>
#include "ConvexPolygonMesh.h"
#include "MeshFileFormat.h"

class Mesh
{
public:
	Mesh();
	virtual ~Mesh();

	bool Load();
	bool Save();

	void Render(GLint renderMode);

	MeshNinja::ConvexPolygonMesh mesh;
	wxString fileSource;
	bool isSelected;

	MeshNinja::MeshFileFormat* MakeFileFormatObject();
};
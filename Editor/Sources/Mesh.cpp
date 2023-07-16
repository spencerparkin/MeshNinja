#include "Mesh.h"
#include "MeshFileFormat.h"

Mesh::Mesh()
{
	this->isSelected = false;
}

/*virtual*/ Mesh::~Mesh()
{
}

void Mesh::Render(GLint renderMode)
{
	//...
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

	bool success = fileFormat->SaveMesh((const char*)this->fileSource.c_str(), this->mesh);
	delete fileFormat;
	return success;
}

MeshNinja::MeshFileFormat* Mesh::MakeFileFormatObject()
{
	// TODO: Look at extension to know what to return here.
	return new MeshNinja::ObjFileFormat();
}
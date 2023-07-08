#include "MeshListControl.h"
#include "Application.h"
#include "MeshCollectionScene.h"
#include "Mesh.h"
#include <wx/filename.h>

MeshListControl::MeshListControl(wxWindow* parent) : wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_VIRTUAL | wxLC_REPORT)
{
	this->AppendColumn("File");
	this->AppendColumn("# Verts");
	this->AppendColumn("# Facets");
}

/*virtual*/ MeshListControl::~MeshListControl()
{
}

/*virtual*/ wxString MeshListControl::OnGetItemText(long item, long column) const
{
	const MeshCollectionScene* meshScene = wxGetApp().GetMeshScene();
	if (0 <= item && item < (signed)meshScene->GetMeshList().size())
	{
		const Mesh* mesh = meshScene->GetMeshList()[item];
		if (mesh)
		{
			switch (column)
			{
				case 0:
				{
					wxFileName fileName(mesh->fileSource);
					return fileName.GetName();
				}
				case 1:
				{
					return wxString::Format("%d", mesh->mesh.vertexArray->size());
				}
				case 2:
				{
					return wxString::Format("%d", mesh->mesh.facetArray->size());
				}
			}
		}
	}

	return "?";
}
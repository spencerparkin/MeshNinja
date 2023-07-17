#include "MeshListControl.h"
#include "Application.h"
#include "MeshCollectionScene.h"
#include "Mesh.h"
#include "Frame.h"
#include <wx/filename.h>

MeshListControl::MeshListControl(wxWindow* parent) : wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_VIRTUAL | wxLC_REPORT)
{
	this->AppendColumn("File");
	this->AppendColumn("# Verts");
	this->AppendColumn("# Facets");

	this->Bind(wxEVT_LIST_ITEM_SELECTED, &MeshListControl::OnListItemSelected, this);
	this->Bind(wxEVT_LIST_ITEM_DESELECTED, &MeshListControl::OnListItemUnselected, this);
}

/*virtual*/ MeshListControl::~MeshListControl()
{
}

void MeshListControl::UpdateSelectionState(const wxListItem& listItem, long item)
{
#if 0		// TODO: This doesn't work.  Why?
	const MeshCollectionScene* meshScene = wxGetApp().GetMeshScene();
	if (0 <= item && item < (signed)meshScene->GetMeshList().size())
	{
		const Mesh* mesh = meshScene->GetMeshList()[item];
		if (mesh)
		{
			int state = this->GetItemState(item, wxLIST_MASK_STATE);

			if ((state & wxLIST_STATE_SELECTED) != 0)
				mesh->isSelected = true;
			else
				mesh->isSelected = false;

			wxCommandEvent sceneChangedEvent(EVT_SCENE_CHANGED);
			wxPostEvent(wxGetApp().GetFrame(), sceneChangedEvent);
		}
	}
#endif
}

void MeshListControl::OnListItemSelected(wxListEvent& event)
{
	this->UpdateSelectionState(event.GetItem(), event.GetIndex());
}

void MeshListControl::OnListItemUnselected(wxListEvent& event)
{
	this->UpdateSelectionState(event.GetItem(), event.GetIndex());
}

void MeshListControl::Update()
{
#if 0	// TODO: This doesn't work.  Why?
	const MeshCollectionScene* meshScene = wxGetApp().GetMeshScene();
	long item = -1;
	while (true)
	{
		item = this->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
		if (item == -1)
			break;

		long state = wxLIST_STATE_DONTCARE;
		const Mesh* mesh = meshScene->GetMeshList()[item];
		if (mesh && mesh->isSelected)
			state = wxLIST_STATE_SELECTED;
		
		this->SetItemState(item, state, wxLIST_MASK_STATE);
	}
#endif
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
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

void MeshListControl::OnListItemSelected(wxListEvent& event)
{
	this->PushControlSelectionToScene();
}

void MeshListControl::OnListItemUnselected(wxListEvent& event)
{
	this->PushControlSelectionToScene();
}

void MeshListControl::PushControlSelectionToScene()
{
	// TODO: Why doesn't this work in the case of multiple list item selections?
	const MeshCollectionScene* meshScene = wxGetApp().GetMeshScene();
	long item = -1;
	while (true)
	{
		item = this->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
		if (item == -1)
			break;

		const Mesh* mesh = meshScene->GetMeshList()[item];

		int state = this->GetItemState(item, wxLIST_STATE_SELECTED);
		if ((state & wxLIST_STATE_SELECTED) != 0)
			mesh->isSelected = true;
		else
			mesh->isSelected = false;
	}

	wxCommandEvent sceneChangedEvent(EVT_SCENE_CHANGED);
	wxPostEvent(wxGetApp().GetFrame(), sceneChangedEvent);
}

void MeshListControl::PullControlSelectionFromScene()
{
#if 0		// TODO: Not sure how to synchronize the control selection with the scene.
	const MeshCollectionScene* meshScene = wxGetApp().GetMeshScene();
	long item = -1;
	while (true)
	{
		item = this->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
		if (item == -1)
			break;

		long oldState = this->GetItemState(item, wxLIST_STATE_SELECTED);
		
		long newState = 0;
		const Mesh* mesh = meshScene->GetMeshList()[item];
		if (mesh && mesh->isSelected)
			newState = wxLIST_STATE_SELECTED;
		
		if (newState != oldState)
			this->SetItemState(item, newState, wxLIST_STATE_SELECTED);
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
			// I'm not sure where else to do this; so do it here.
			const_cast<MeshListControl*>(this)->SetItemPtrData(item, wxUIntPtr(this));

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
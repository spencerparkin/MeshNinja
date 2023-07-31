#include "MeshListControl.h"
#include "Application.h"
#include "MeshCollectionScene.h"
#include "Mesh.h"
#include "Frame.h"
#include "MeshGraph.h"
#include <wx/filename.h>
#include <wx/msgdlg.h>

MeshListControl::MeshListControl(wxWindow* parent) : wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_VIRTUAL | wxLC_REPORT)
{
	this->AppendColumn("File");
	this->AppendColumn("# Verts");
	this->AppendColumn("# Facets");
	this->AppendColumn("Visible");

	this->Bind(wxEVT_LIST_ITEM_SELECTED, &MeshListControl::OnListItemSelected, this);
	this->Bind(wxEVT_LIST_ITEM_DESELECTED, &MeshListControl::OnListItemUnselected, this);
	this->Bind(wxEVT_LIST_ITEM_RIGHT_CLICK, &MeshListControl::OnListItemRightClick, this);
}

/*virtual*/ MeshListControl::~MeshListControl()
{
}

void MeshListControl::OnListItemRightClick(wxListEvent& event)
{
	wxMenu contextMenu;

	contextMenu.Append(new wxMenuItem(&contextMenu, ID_ToggleVisibility, "Toggle Visibility", "Turn the visibility of the mesh on/off."));
	contextMenu.Append(new wxMenuItem(&contextMenu, ID_MakeDual, "Make Dual", "Add the dual of this mesh to the scene."));
	contextMenu.Append(new wxMenuItem(&contextMenu, ID_ClearFileSource, "Clear File Source", "Unbind this mesh from any known file on the file system."));
	contextMenu.Append(new wxMenuItem(&contextMenu, ID_GenerateGraphDebugDraw, "Generate Graph Debug Draw", "Generate points and lines to visualize a graph made as a function of the mesn."));
	contextMenu.Append(new wxMenuItem(&contextMenu, ID_NormalizeEdges, "Normalize Edges", "Get rid of redundant edges."));
	contextMenu.Append(new wxMenuItem(&contextMenu, ID_ReverseFaces, "Reverse Faces", "Reverse the winding on all polygons of the mesh."));

	contextMenu.Bind(wxEVT_MENU, &MeshListControl::OnToggleVisibility, this, ID_ToggleVisibility);
	contextMenu.Bind(wxEVT_MENU, &MeshListControl::OnMakeDual, this, ID_MakeDual);
	contextMenu.Bind(wxEVT_MENU, &MeshListControl::OnClearFileSource, this, ID_ClearFileSource);
	contextMenu.Bind(wxEVT_MENU, &MeshListControl::OnGenerateGraphDebugDraw, this, ID_GenerateGraphDebugDraw);
	contextMenu.Bind(wxEVT_MENU, &MeshListControl::OnNormalizeEdges, this, ID_NormalizeEdges);
	contextMenu.Bind(wxEVT_MENU, &MeshListControl::OnReverseFaces, this, ID_ReverseFaces);
	contextMenu.Bind(wxEVT_UPDATE_UI, &MeshListControl::OnUpdateUI, this, ID_ClearFileSource);

	this->PopupMenu(&contextMenu);
}

void MeshListControl::OnUpdateUI(wxUpdateUIEvent& event)
{
	// TODO: Why is this never called?
	switch (event.GetId())
	{
		case ID_ClearFileSource:
		{
			Mesh* mesh = this->GetSelectedMesh();
			event.Enable(mesh && mesh->fileSource.Len() > 0);
			break;
		}
	}
}

void MeshListControl::OnGenerateGraphDebugDraw(wxCommandEvent& event)
{
	Mesh* mesh = this->GetSelectedMesh();
	if (mesh)
	{
		MeshNinja::MeshGraph graph;
		if (graph.Generate(mesh->mesh))
		{
			graph.GenerateDebugDrawObjects(wxGetApp().GetMeshScene()->debugDraw);
			wxPostEvent(wxGetApp().GetFrame(), wxCommandEvent(EVT_SCENE_CHANGED));
		}
	}
}

void MeshListControl::OnNormalizeEdges(wxCommandEvent& event)
{
	Mesh* mesh = this->GetSelectedMesh();
	if (mesh)
	{
		mesh->mesh.NormalizeEdges();
		mesh->DirtyRenderFlag();
		wxPostEvent(wxGetApp().GetFrame(), wxCommandEvent(EVT_SCENE_CHANGED));
	}
}

void MeshListControl::OnReverseFaces(wxCommandEvent& event)
{
	Mesh* mesh = this->GetSelectedMesh();
	if (mesh)
	{
		mesh->mesh.ReverseAllPolygons();
		mesh->DirtyRenderFlag();
		wxPostEvent(wxGetApp().GetFrame(), wxCommandEvent(EVT_SCENE_CHANGED));
	}
}

void MeshListControl::OnToggleVisibility(wxCommandEvent& event)
{
	Mesh* mesh = this->GetSelectedMesh();
	if (mesh)
	{
		mesh->SetVisible(!mesh->GetVisible());

		wxCommandEvent sceneChangedEvent(EVT_SCENE_CHANGED);
		wxPostEvent(wxGetApp().GetFrame(), sceneChangedEvent);
	}
}

void MeshListControl::OnMakeDual(wxCommandEvent& event)
{
	Mesh* mesh = this->GetSelectedMesh();
	if (mesh)
	{
		MeshNinja::ConvexPolygonMesh transformedMesh(mesh->mesh);
		transformedMesh.ApplyTransform(mesh->transform);

		MeshNinja::MeshGraph meshGraph;
		if (!meshGraph.Generate(transformedMesh))
		{
			wxMessageBox("Failed to generate mesh graph!", "Error", wxICON_ERROR | wxOK, this);
			return;
		}

		Mesh* dualMesh = new Mesh();

		if (!meshGraph.GenerateDual(dualMesh->mesh))
		{
			wxMessageBox("Failed to generate dual mesh!", "Error", wxICON_ERROR | wxOK, this);
			delete dualMesh;
			return;
		}

		wxGetApp().GetMeshScene()->GetMeshList().push_back(dualMesh);

		wxCommandEvent sceneChangedEvent(EVT_SCENE_CHANGED);
		wxPostEvent(wxGetApp().GetFrame(), sceneChangedEvent);
	}
}

void MeshListControl::OnClearFileSource(wxCommandEvent& event)
{
	Mesh* mesh = this->GetSelectedMesh();
	if (mesh)
	{
		mesh->fileSource = "";

		wxPostEvent(wxGetApp().GetFrame(), wxCommandEvent(EVT_SCENE_CHANGED));
	}
}

Mesh* MeshListControl::GetSelectedMesh()
{
	long selectedItem = this->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (selectedItem < 0)
		return nullptr;

	const MeshCollectionScene* meshScene = wxGetApp().GetMeshScene();
	if (selectedItem >= (signed)meshScene->GetMeshList().size())
		return nullptr;

	return meshScene->GetMeshList()[selectedItem];
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
			mesh->SetSelected(true);
		else
			mesh->SetSelected(false);
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
			switch (column)
			{
				case 0:
				{
					wxFileName fileName(mesh->fileSource);
					wxString name = fileName.GetName();
					if (name.Len() > 0)
						return name;
					return "<no name>";
				}
				case 1:
				{
					return wxString::Format("%d", mesh->mesh.vertexArray->size());
				}
				case 2:
				{
					return wxString::Format("%d", mesh->mesh.facetArray->size());
				}
				case 3:
				{
					if (mesh->GetVisible())
						return wxString("Yes");
					else
						return wxString("No");
				}
			}
		}
	}

	return "?";
}
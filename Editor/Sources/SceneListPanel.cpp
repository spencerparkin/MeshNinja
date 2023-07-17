#include "SceneListPanel.h"
#include "MeshListControl.h"
#include "MeshCollectionScene.h"
#include "Application.h"

wxIMPLEMENT_DYNAMIC_CLASS(SceneListPanel, Panel);

SceneListPanel::SceneListPanel()
{
	this->listControl = nullptr;
}

/*virtual*/ SceneListPanel::~SceneListPanel()
{
}

/*virtual*/ void SceneListPanel::MakePaneInfo(wxAuiPaneInfo& paneInfo)
{
	paneInfo.Name("SceneList");
	paneInfo.Caption("Scene List");
	paneInfo.BestSize(wxSize(500, 500));
	paneInfo.Dockable(true);
}

/*virtual*/ void SceneListPanel::MakeControls()
{
	this->listControl = new MeshListControl(this);

	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	boxSizer->Add(this->listControl, 1, wxALL | wxGROW, 0);
	this->SetSizer(boxSizer);
}

/*virtual*/ void SceneListPanel::Update()
{
	this->listControl->SetItemCount(wxGetApp().GetMeshScene()->GetMeshList().size());
	this->listControl->Update();
	this->listControl->Refresh();
}
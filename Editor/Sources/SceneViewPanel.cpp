#include "SceneViewPanel.h"
#include "Canvas.h"
#include "Application.h"
#include "MeshCollectionScene.h"

wxIMPLEMENT_DYNAMIC_CLASS(SceneViewPanel, Panel);

SceneViewPanel::SceneViewPanel()
{
	this->canvas = nullptr;
}

/*virtual*/ SceneViewPanel::~SceneViewPanel()
{
}

/*virtual*/ void SceneViewPanel::MakePaneInfo(wxAuiPaneInfo& paneInfo)
{
	Panel::MakePaneInfo(paneInfo);

	paneInfo.Caption("Scene View");
	paneInfo.CenterPane();
	paneInfo.Name("SceneView");
}

/*virtual*/ void SceneViewPanel::MakeControls()
{
	this->canvas = new Canvas(this);

	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	boxSizer->Add(this->canvas, 1, wxALL | wxGROW, 0);
	this->SetSizer(boxSizer);
}

/*virtual*/ void SceneViewPanel::Update()
{
	this->canvas->SetScene(wxGetApp().GetMeshScene());
	this->canvas->Refresh();
}

/*virtual*/ void SceneViewPanel::Tick()
{
	this->canvas->Tick();
}
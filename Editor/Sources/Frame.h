#pragma once

#include <wx/frame.h>
#include <wx/aui/aui.h>
#include <wx/timer.h>
#include <wx/combobox.h>

wxDECLARE_EVENT(EVT_SCENE_CHANGED, wxCommandEvent);

class Panel;

class Frame : public wxFrame
{
public:
	Frame(wxWindow* parent, const wxPoint& pos, const wxSize& size);
	virtual ~Frame();

	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnImportMesh(wxCommandEvent& event);
	void OnExportMesh(wxCommandEvent& event);
	void OnSceneChanged(wxCommandEvent& event);
	void OnTimerTick(wxTimerEvent& event);
	void OnMeshSetOperation(wxCommandEvent& event);
	void OnAddMesh(wxCommandEvent& event);
	void OnToggle(wxCommandEvent& event);
	void OnUpdateUI(wxUpdateUIEvent& event);
	void OnClearScene(wxCommandEvent& event);
	void OnGenerateMaze(wxCommandEvent& event);
	void OnLoadDebugDrawData(wxCommandEvent& event);

	enum
	{
		ID_Exit = wxID_HIGHEST,
		ID_About,
		ID_ImportMesh,
		ID_ExportMesh,
		ID_IntersectMeshes,
		ID_UnionMeshes,
		ID_SubtractMeshes,
		ID_AddMesh,
		ID_RenderUnlit,
		ID_RenderFaceLit,
		ID_RenderVertexLit,
		ID_RenderAxes,
		ID_ToggleEdgeRender,
		ID_ToggleFaceNormalRender,
		ID_ToggleVertexNormalRender,
		ID_UseFaceColors,
		ID_UseVertexColors,
		ID_ClearScene,
		ID_GenerateMaze,
		ID_LoadDebugDrawData
	};

private:

	void MakePanels();
	void UpdatePanels();
	void ForAllPanels(std::function<void(Panel*)> callback);

	wxComboBox* meshComboBox;
	wxAuiManager auiManager;
	wxTimer timer;
	bool inTimerTick;
};
#pragma once

#include <wx/frame.h>
#include <wx/aui/aui.h>
#include <wx/timer.h>

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

	enum
	{
		ID_Exit = wxID_HIGHEST,
		ID_About,
		ID_ImportMesh,
		ID_ExportMesh
	};

private:

	void MakePanels();
	void UpdatePanels();
	void ForAllPanels(std::function<void(Panel*)> callback);

	wxAuiManager auiManager;
	wxTimer timer;
	bool inTimerTick;
};
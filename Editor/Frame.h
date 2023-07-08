#pragma once

#include <wx/frame.h>

class Frame : public wxFrame
{
public:
	Frame(wxWindow* parent, const wxPoint& pos, const wxSize& size);
	virtual ~Frame();

	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnImportMesh(wxCommandEvent& event);
	void OnExportMesh(wxCommandEvent& event);

	enum
	{
		ID_Exit = wxID_HIGHEST,
		ID_About,
		ID_ImportMesh,
		ID_ExportMesh
	};
};
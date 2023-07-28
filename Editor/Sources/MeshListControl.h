#pragma once

#include <wx/listctrl.h>

class Mesh;

class MeshListControl : public wxListCtrl
{
public:
	MeshListControl(wxWindow* parent);
	virtual ~MeshListControl();

	virtual wxString OnGetItemText(long item, long column) const override;

	void PushControlSelectionToScene();
	void PullControlSelectionFromScene();

private:

	void OnListItemSelected(wxListEvent& event);
	void OnListItemUnselected(wxListEvent& event);
	void OnListItemRightClick(wxListEvent& event);
	void OnToggleVisibility(wxCommandEvent& event);
	void OnMakeDual(wxCommandEvent& event);
	void OnClearFileSource(wxCommandEvent& event);
	void OnUpdateUI(wxUpdateUIEvent& event);

	enum
	{
		ID_ToggleVisibility = wxID_HIGHEST + 500,
		ID_MakeDual,
		ID_ClearFileSource
	};

	Mesh* GetSelectedMesh();
};
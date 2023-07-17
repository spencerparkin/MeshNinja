#pragma once

#include <wx/listctrl.h>

class MeshListControl : public wxListCtrl
{
public:
	MeshListControl(wxWindow* parent);
	virtual ~MeshListControl();

	virtual wxString OnGetItemText(long item, long column) const override;

	void OnListItemSelected(wxListEvent& event);
	void OnListItemUnselected(wxListEvent& event);

	void UpdateSelectionState(const wxListItem& listItem, long item);
	void Update();
};
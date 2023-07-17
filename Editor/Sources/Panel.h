#pragma once

#include <wx/panel.h>
#include <wx/aui/aui.h>

class Panel : public wxPanel
{
public:
	wxDECLARE_DYNAMIC_CLASS(Panel);

	Panel();
	virtual ~Panel();

	virtual void MakePaneInfo(wxAuiPaneInfo& paneInfo);
	virtual void MakeControls();
	virtual void Update();
	virtual void Tick();
};
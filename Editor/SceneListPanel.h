#pragma once

#include "Panel.h"
#include <wx/listctrl.h>

class MeshListControl;

class SceneListPanel : public Panel
{
public:
	wxDECLARE_DYNAMIC_CLASS(SceneListPanel);

	SceneListPanel();
	virtual ~SceneListPanel();

	virtual void MakePaneInfo(wxAuiPaneInfo& paneInfo) override;
	virtual void MakeControls() override;
	virtual void Update() override;

	MeshListControl* listControl;
};
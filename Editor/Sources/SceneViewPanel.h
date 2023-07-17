#pragma once

#include "Panel.h"

class Canvas;

class SceneViewPanel : public Panel
{
public:
	wxDECLARE_DYNAMIC_CLASS(SceneViewPanel);

	SceneViewPanel();
	virtual ~SceneViewPanel();

	virtual void MakePaneInfo(wxAuiPaneInfo& paneInfo) override;
	virtual void MakeControls() override;
	virtual void Update() override;
	virtual void Tick() override;

private:

	Canvas* canvas;
};
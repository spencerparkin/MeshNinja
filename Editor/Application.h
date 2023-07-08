#pragma once

#include <wx/app.h>

class Frame;

class Application : public wxApp
{
public:
	Application();
	virtual ~Application();

	virtual bool OnInit(void) override;
	virtual int OnExit(void) override;

	Frame* GetFrame() { return this->frame; }

private:

	Frame* frame;
};

wxDECLARE_APP(Application);
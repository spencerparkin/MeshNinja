#include "Application.h"
#include "Frame.h"

wxIMPLEMENT_APP(Application);

Application::Application()
{
	this->frame = nullptr;
}

/*virtual*/ Application::~Application()
{
}

/*virtual*/ bool Application::OnInit(void)
{
	if (!wxApp::OnInit())
		return false;

	this->frame = new Frame(nullptr, wxDefaultPosition, wxDefaultSize);
	this->frame->Show();

	this->SetTopWindow(this->frame);

	return true;
}

/*virtual*/ int Application::OnExit(void)
{
	return 0;
}
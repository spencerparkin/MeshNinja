#include "Application.h"
#include "Frame.h"
#include "MeshCollectionScene.h"

wxIMPLEMENT_APP(Application);

Application::Application()
{
	this->frame = nullptr;
	this->meshScene = new MeshCollectionScene();
	this->lightingMode = LightingMode::FACE_LIT;
	this->renderEdges = false;
	this->renderFaceNormals = false;
	this->renderVertexNormals = false;
}

/*virtual*/ Application::~Application()
{
	delete this->meshScene;
}

/*virtual*/ bool Application::OnInit(void)
{
	if (!wxApp::OnInit())
		return false;

	wxInitAllImageHandlers();

	this->frame = new Frame(nullptr, wxDefaultPosition, wxSize(1600, 800));
	this->frame->Show();

	this->SetTopWindow(this->frame);

	return true;
}

/*virtual*/ int Application::OnExit(void)
{
	return 0;
}
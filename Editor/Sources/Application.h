#pragma once

#include "Scene.h"
#include <wx/app.h>

class Frame;
class MeshCollectionScene;

class Application : public wxApp
{
public:
	Application();
	virtual ~Application();

	virtual bool OnInit(void) override;
	virtual int OnExit(void) override;

	Frame* GetFrame() { return this->frame; }
	MeshCollectionScene* GetMeshScene() { return this->meshScene; }

	enum class LightingMode
	{
		UNLIT,
		LIT
	};

	LightingMode lightingMode;
	bool renderEdges;
	bool renderFaceNormals;

private:

	Frame* frame;
	MeshCollectionScene* meshScene;
};

wxDECLARE_APP(Application);
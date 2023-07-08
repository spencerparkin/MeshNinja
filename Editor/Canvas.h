#pragma once

#include <wx/glcanvas.h>

class Scene;

class Canvas : public wxGLCanvas
{
public:
	Canvas(wxWindow* parent);
	virtual ~Canvas();

	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);

	void SetScene(Scene* scene);
	Scene* GetScene() { return this->scene; }

private:

	void BindContext();
	void RenderScene(GLint renderMode);

	wxGLContext* context;
	static int attributeList[];
	Scene* scene;
};
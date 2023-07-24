#pragma once

#include <wx/glcanvas.h>
#include "Camera.h"
#include "Scene.h"

class Canvas : public wxGLCanvas
{
public:
	Canvas(wxWindow* parent);
	virtual ~Canvas();

	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnRightMouseDown(wxMouseEvent& event);
	void OnLeftMouseDown(wxMouseEvent& event);
	void OnLeftMouseUp(wxMouseEvent& event);
	void OnMouseMove(wxMouseEvent& event);
	void OnMouseWheel(wxMouseEvent& event);

	void Tick();

	void SetScene(Scene* scene);
	Scene* GetScene() { return this->scene; }

private:

	enum ScaleMode
	{
		UNIFORM,
		X_AXIS,
		Y_AXIS,
		Z_AXIS
	};

	void BindContext();
	const Scene::Object* RenderScene(GLint renderMode, const wxPoint* pickingPoint = nullptr);

	wxGLContext* context;
	static int attributeList[];
	Scene* scene;
	Camera* camera;
	bool dragging;
	wxPoint lastMousePos;
	ScaleMode scaleMode;
};
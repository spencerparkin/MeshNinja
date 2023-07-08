#pragma once

#include <wx/glcanvas.h>

class Canvas : public wxGLCanvas
{
public:
	Canvas(wxWindow* parent);
	virtual ~Canvas();

	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);

private:

	void BindContext();
	void RenderScene(GLint renderMode);

	wxGLContext* context;
	static int attributeList[];
};
#include "Canvas.h"

int Canvas::attributeList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, 0 };

Canvas::Canvas(wxWindow* parent) : wxGLCanvas(parent, wxID_ANY, attributeList, wxDefaultPosition, wxDefaultSize)
{
	this->context = nullptr;

	this->Bind(wxEVT_PAINT, &Canvas::OnPaint, this);
	this->Bind(wxEVT_SIZE, &Canvas::OnSize, this);
}

/*virtual*/ Canvas::~Canvas()
{
	delete this->context;
}

void Canvas::RenderScene(GLint renderMode)
{
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glFlush();
}

void Canvas::OnPaint(wxPaintEvent& event)
{
	this->BindContext();

	this->RenderScene(GL_RENDER);

	this->SwapBuffers();
}

void Canvas::OnSize(wxSizeEvent& event)
{
	this->BindContext();

	wxSize size = event.GetSize();
	glViewport(0, 0, size.GetWidth(), size.GetHeight());

	this->Refresh();
}

void Canvas::BindContext()
{
	if (!this->context)
		this->context = new wxGLContext(this);

	this->SetCurrent(*this->context);
}
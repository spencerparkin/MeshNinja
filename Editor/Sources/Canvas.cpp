#include "Canvas.h"
#include "Scene.h"
#include <wx/kbdstate.h>

int Canvas::attributeList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, 0 };

Canvas::Canvas(wxWindow* parent) : wxGLCanvas(parent, wxID_ANY, attributeList, wxDefaultPosition, wxDefaultSize)
{
	this->context = nullptr;
	this->scene = nullptr;
	this->camera = new Camera();
	this->dragging = false;

	this->Bind(wxEVT_PAINT, &Canvas::OnPaint, this);
	this->Bind(wxEVT_SIZE, &Canvas::OnSize, this);
	this->Bind(wxEVT_KEY_DOWN, &Canvas::OnKeyDown, this);
	this->Bind(wxEVT_RIGHT_DOWN, &Canvas::OnRightMouseDown, this);
	this->Bind(wxEVT_LEFT_DOWN, &Canvas::OnLeftMouseDown, this);
	this->Bind(wxEVT_LEFT_UP, &Canvas::OnLeftMouseUp, this);
	this->Bind(wxEVT_MOTION, &Canvas::OnMouseMove, this);
}

/*virtual*/ Canvas::~Canvas()
{
	delete this->context;
	delete this->camera;
}

void Canvas::Tick()
{
	bool refreshNeeded = false;

	if (wxGetKeyState(wxKeyCode::WXK_UP))
	{
		this->camera->MoveForwardBackward(-0.5);
		refreshNeeded = true;
	}

	if (wxGetKeyState(wxKeyCode::WXK_DOWN))
	{
		this->camera->MoveForwardBackward(0.5);
		refreshNeeded = true;
	}

	if (wxGetKeyState(wxKeyCode::WXK_LEFT))
	{
		this->camera->StrafeLeftRight(-0.5);
		refreshNeeded = true;
	}

	if (wxGetKeyState(wxKeyCode::WXK_RIGHT))
	{
		this->camera->StrafeLeftRight(0.5);
		refreshNeeded = true;
	}

	if (refreshNeeded)
		this->Refresh();
}

void Canvas::OnKeyDown(wxKeyEvent& event)
{
	switch (event.GetKeyCode())
	{
		case 's':
		case 'S':
		{
			this->camera->StrafeLeftRight(-0.5);
			break;
		}
		case 'f':
		case 'F':
		{
			this->camera->StrafeLeftRight(0.5);
			break;
		}
		case 'e':
		case 'E':
		{
			this->camera->MoveForwardBackward(-0.5);
			break;
		}
		case 'd':
		case 'D':
		{
			this->camera->MoveForwardBackward(0.5);
			break;
		}
	}

	this->Refresh();
}

void Canvas::OnRightMouseDown(wxMouseEvent& event)
{
	wxPoint pickingPoint = event.GetPosition();
	const Scene::Object* object = this->RenderScene(GL_SELECT, &pickingPoint);
	this->scene->HandlePick(object, event.ShiftDown());
}

void Canvas::OnLeftMouseDown(wxMouseEvent& event)
{
	this->dragging = true;
	this->lastMousePos = event.GetPosition();
}

void Canvas::OnLeftMouseUp(wxMouseEvent& event)
{
	this->dragging = false;
}

void Canvas::OnMouseMove(wxMouseEvent& event)
{
	if (this->dragging)
	{
		wxPoint curMousePos = event.GetPosition();
		wxPoint mouseDelta = curMousePos - this->lastMousePos;
		this->lastMousePos = curMousePos;

		double motionFactor = 0.01;

		double angleDeltaY = motionFactor * double(-mouseDelta.y);
		this->camera->LookUpDown(angleDeltaY);

		double angleDeltaX = motionFactor * double(-mouseDelta.x);
		this->camera->LookLeftRight(angleDeltaX);

		this->Refresh();
	}
}

void Canvas::SetScene(Scene* scene)
{
	this->scene = scene;
}

const Scene::Object* Canvas::RenderScene(GLint renderMode, const wxPoint* pickingPoint /*= nullptr*/)
{
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);

	this->camera->PreRender(renderMode, pickingPoint);

	if (renderMode == GL_RENDER)
	{
		glBegin(GL_LINES);

		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(10.0f, 0.0f, 0.0f);

		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 10.0f, 0.0f);

		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 10.0f);

		glEnd();
	}

	if (this->scene)
		this->scene->Render(renderMode, this->camera);

	return this->camera->PostRender(renderMode, this->scene);
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

	this->camera->aspectRatio = double(size.GetWidth()) / double(size.GetHeight());

	this->Refresh();
}

void Canvas::BindContext()
{
	if (!this->context)
		this->context = new wxGLContext(this);

	this->SetCurrent(*this->context);
}
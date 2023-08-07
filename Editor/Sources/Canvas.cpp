#include "Canvas.h"
#include "Scene.h"
#include "Application.h"
#include <wx/kbdstate.h>

int Canvas::attributeList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, 0 };

Canvas::Canvas(wxWindow* parent) : wxGLCanvas(parent, wxID_ANY, attributeList, wxDefaultPosition, wxDefaultSize)
{
	this->context = nullptr;
	this->scene = nullptr;
	this->camera = new Camera();
	this->dragging = false;
	this->scaleMode = ScaleMode::UNIFORM;

	this->Bind(wxEVT_PAINT, &Canvas::OnPaint, this);
	this->Bind(wxEVT_SIZE, &Canvas::OnSize, this);
	this->Bind(wxEVT_KEY_DOWN, &Canvas::OnKeyDown, this);
	this->Bind(wxEVT_RIGHT_DOWN, &Canvas::OnRightMouseDown, this);
	this->Bind(wxEVT_LEFT_DOWN, &Canvas::OnLeftMouseDown, this);
	this->Bind(wxEVT_LEFT_UP, &Canvas::OnLeftMouseUp, this);
	this->Bind(wxEVT_MOTION, &Canvas::OnMouseMove, this);
	this->Bind(wxEVT_MOUSEWHEEL, &Canvas::OnMouseWheel, this);
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
		case 'x':
		case 'X':
		{
			this->scaleMode = ScaleMode::X_AXIS;
			break;
		}
		case 'y':
		case 'Y':
		{
			this->scaleMode = ScaleMode::Y_AXIS;
			break;
		}
		case 'z':
		case 'Z':
		{
			this->scaleMode = ScaleMode::Z_AXIS;
			break;
		}
		case 'u':
		case 'U':
		{
			this->scaleMode = ScaleMode::UNIFORM;
			break;
		}
		default:
		{
			this->scene->HandleKey(event.GetKeyCode());
			break;
		}
	}

	this->Refresh();
}

void Canvas::OnRightMouseDown(wxMouseEvent& event)
{
	this->SetFocus();

	wxPoint pickingPoint = event.GetPosition();
	const Scene::Object* object = this->RenderScene(GL_SELECT, &pickingPoint);
	this->scene->HandlePick(object, event.ShiftDown());

	if (event.AltDown())
	{
		if (object)
			this->camera->target = object->GetPosition();
		else
			this->camera->target = MeshNinja::Vector3(0.0, 0.0, 0.0);
	}
}

void Canvas::OnLeftMouseDown(wxMouseEvent& event)
{
	this->SetFocus();

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

		if (event.ShiftDown())
		{
			double sensativityFactor = 0.1;
			
			MeshNinja::Vector3 xAxis, yAxis, zAxis;
			this->camera->MakeFrame(xAxis, yAxis, zAxis);

			MeshNinja::Transform transform;
			transform.SetIdentity();
			transform.translation = (xAxis * double(mouseDelta.x) + yAxis * double(-mouseDelta.y)) * sensativityFactor;
			this->scene->HandleTransform(transform);
		}
		else
		{
			double sensativityFactor = 0.01;

			double angleDeltaY = sensativityFactor * double(-mouseDelta.y);
			this->camera->LookUpDown(angleDeltaY, event.AltDown());

			double angleDeltaX = sensativityFactor * double(-mouseDelta.x);
			this->camera->LookLeftRight(angleDeltaX, event.AltDown());
		}

		this->Refresh();
	}
}

void Canvas::OnMouseWheel(wxMouseEvent& event)
{
	int mouseWheelTicks = event.GetWheelRotation() / 120;		// Multiples of 120 are used.  I'm not sure why.

	if (event.ShiftDown())
	{
		MeshNinja::Transform transform;
		transform.SetIdentity();

		if (event.AltDown())
		{
			double scale = 1.0 + double(mouseWheelTicks) * 0.1;

			switch (this->scaleMode)
			{
				case ScaleMode::UNIFORM:
				{
					transform.matrix.SetCol(0, MeshNinja::Vector3(scale, 0.0, 0.0));
					transform.matrix.SetCol(1, MeshNinja::Vector3(0.0, scale, 0.0));
					transform.matrix.SetCol(2, MeshNinja::Vector3(0.0, 0.0, scale));
					break;
				}
				case ScaleMode::X_AXIS:
				{
					transform.matrix.SetCol(0, MeshNinja::Vector3(scale, 0.0, 0.0));
					break;
				}
				case ScaleMode::Y_AXIS:
				{
					transform.matrix.SetCol(1, MeshNinja::Vector3(0.0, scale, 0.0));
					break;
				}
				case ScaleMode::Z_AXIS:
				{
					transform.matrix.SetCol(2, MeshNinja::Vector3(0.0, 0.0, scale));
					break;
				}
			}
		}
		else
		{
			double angle = double(-mouseWheelTicks) * (MESH_NINJA_PI / 180.0);

			MeshNinja::Vector3 xAxis, yAxis, zAxis;
			this->camera->MakeFrame(xAxis, yAxis, zAxis);

			transform.matrix.SetFromAxisAngle(xAxis, angle);
		}

		this->scene->HandleTransform(transform);
	}
	else
	{
		double zoomFactor = 1.0 + 0.1 * double(mouseWheelTicks);
		this->camera->Zoom(zoomFactor);
	}

	this->Refresh();
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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	glShadeModel(GL_SMOOTH);

	this->camera->PreRender(renderMode, pickingPoint);

	if (renderMode == GL_RENDER && wxGetApp().renderAxes)
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
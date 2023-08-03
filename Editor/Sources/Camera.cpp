#include "Camera.h"
#include "Scene.h"
#include <wx/glcanvas.h>
#include <gl/GLU.h>

Camera::Camera()
{
	this->position.x = 50.0f;
	this->position.y = 50.0f;
	this->position.z = 50.0f;

	this->target.x = 0.0f;
	this->target.y = 0.0f;
	this->target.z = 0.0f;

	this->foviAngle = 60.0f;
	this->aspectRatio = 1.0f;

	this->hitBuffer = nullptr;
	this->hitBufferSize = 0;
}

/*virtual*/ Camera::~Camera()
{
}

MeshNinja::Vector Camera::GetViewDirection() const
{
	return this->target - this->position;
}

/*virtual*/ void Camera::PreRender(int renderMode, const wxPoint* pickingPoint)
{
	if (renderMode == GL_SELECT)
	{
		this->hitBufferSize = 512;
		this->hitBuffer = new GLuint[this->hitBufferSize];
		glSelectBuffer(hitBufferSize, this->hitBuffer);
		glRenderMode(GL_SELECT);
		glInitNames();
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (renderMode == GL_SELECT)
	{
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		gluPickMatrix(double(pickingPoint->x), double(viewport[3]) - double(pickingPoint->y), 2.0, 2.0, viewport);
	}

	gluPerspective(this->foviAngle, this->aspectRatio, 0.1, 10000.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(this->position.x, this->position.y, this->position.z, this->target.x, this->target.y, this->target.z, 0.0, 1.0, 0.0);
}

/*virtual*/ const Scene::Object* Camera::PostRender(int renderMode, const Scene* scene)
{
	glFlush();

	const Scene::Object* object = nullptr;

	if (renderMode == GL_SELECT)
	{
		GLuint hitCount = glRenderMode(GL_RENDER);

		object = scene->ProcessHitBuffer(this->hitBuffer, this->hitBufferSize, hitCount);

		delete[] this->hitBuffer;
		this->hitBuffer = nullptr;
		this->hitBufferSize = 0;
	}

	return object;
}

void Camera::MakeFrame(MeshNinja::Vector& xAxis, MeshNinja::Vector& yAxis, MeshNinja::Vector& zAxis) const
{
	zAxis = this->position - this->target;
	zAxis.Normalize();
	xAxis = MeshNinja::Vector(0.0, 1.0, 0.0).Cross(zAxis);
	xAxis.Normalize();
	yAxis = zAxis.Cross(xAxis);
}

void Camera::StrafeLeftRight(double delta)
{
	MeshNinja::Vector xAxis, yAxis, zAxis;
	this->MakeFrame(xAxis, yAxis, zAxis);

	this->position += xAxis * delta;
	this->target += xAxis * delta;
}

void Camera::MoveForwardBackward(double delta)
{
	MeshNinja::Vector xAxis, yAxis, zAxis;
	this->MakeFrame(xAxis, yAxis, zAxis);

	this->position += zAxis * delta;
	this->target += zAxis * delta;
}

void Camera::LookLeftRight(double angleDelta, bool pivotAboutTarget)
{
	MeshNinja::Vector xAxis, yAxis, zAxis;
	this->MakeFrame(xAxis, yAxis, zAxis);
	this->RotateLookDirection(yAxis, angleDelta, pivotAboutTarget);
}

void Camera::LookUpDown(double angleDelta, bool pivotAboutTarget)
{
	MeshNinja::Vector xAxis, yAxis, zAxis;
	this->MakeFrame(xAxis, yAxis, zAxis);
	this->RotateLookDirection(xAxis, angleDelta, pivotAboutTarget);
}

void Camera::RotateLookDirection(const MeshNinja::Vector& axis, double angleDelta, bool pivotAboutTarget)
{
	MeshNinja::Vector lookDirection = this->target - this->position;
	lookDirection.RotateAbout(axis, angleDelta);

	if (pivotAboutTarget)
	{
		this->position = this->target - lookDirection;
	}
	else
	{
		this->target = this->position + lookDirection;
	}
}

void Camera::Zoom(double zoomFactor)
{
	MeshNinja::Vector lookVector = this->target - this->position;
	double length = lookVector.Length();
	length *= zoomFactor;
	lookVector.Normalize();
	lookVector *= length;
	this->position = this->target - lookVector;
}
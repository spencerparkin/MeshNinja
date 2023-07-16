#include "Camera.h"
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
}

/*virtual*/ Camera::~Camera()
{
}

/*virtual*/ void Camera::PreRender(int renderMode)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(this->foviAngle, this->aspectRatio, 0.1, 10000.0);

	if (renderMode == GL_SELECT)
	{
		//...
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(this->position.x, this->position.y, this->position.z, this->target.x, this->target.y, this->target.z, 0.0, 1.0, 0.0);
}

/*virtual*/ void Camera::PostRender(int renderMode)
{
	if (renderMode == GL_SELECT)
	{
		//...
	}
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

void Camera::LookLeftRight(double angleDelta)
{
	MeshNinja::Vector xAxis, yAxis, zAxis;
	this->MakeFrame(xAxis, yAxis, zAxis);

	MeshNinja::Vector lookDirection = this->target - this->position;
	lookDirection.RotateAbout(yAxis, angleDelta);

	this->target = this->position + lookDirection;
}

void Camera::LookUpDown(double angleDelta)
{
	MeshNinja::Vector xAxis, yAxis, zAxis;
	this->MakeFrame(xAxis, yAxis, zAxis);

	MeshNinja::Vector lookDirection = this->target - this->position;
	lookDirection.RotateAbout(xAxis, angleDelta);

	this->target = this->position + lookDirection;
}
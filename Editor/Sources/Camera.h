#pragma once

#include "Vector.h"
#include "Scene.h"
#include <wx/glcanvas.h>

class Scene;

class Camera
{
public:
	Camera();
	virtual ~Camera();

	virtual void PreRender(int renderMode, const wxPoint* pickingPoint);
	virtual const Scene::Object* PostRender(int renderMode, const Scene* scene);

	void StrafeLeftRight(double delta);
	void MoveForwardBackward(double delta);
	void LookLeftRight(double angleDelta);
	void LookUpDown(double angleDelta);

	void MakeFrame(MeshNinja::Vector& xAxis, MeshNinja::Vector& yAxis, MeshNinja::Vector& zAxis) const;

	MeshNinja::Vector position;
	MeshNinja::Vector target;
	double foviAngle;
	double aspectRatio;
	GLuint* hitBuffer;
	GLuint hitBufferSize;
};
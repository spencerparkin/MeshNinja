#pragma once

#include "Math/Vector3.h"
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
	void LookLeftRight(double angleDelta, bool pivotAboutTarget);
	void LookUpDown(double angleDelta, bool pivotAboutTarget);
	void Zoom(double zoomFactor);

	void MakeFrame(MeshNinja::Vector3& xAxis, MeshNinja::Vector3& yAxis, MeshNinja::Vector3& zAxis) const;
	void RotateLookDirection(const MeshNinja::Vector3& axis, double angleDelta, bool pivotAboutTarget);

	MeshNinja::Vector3 GetViewDirection() const;

	MeshNinja::Vector3 position;
	MeshNinja::Vector3 target;
	double foviAngle;
	double aspectRatio;
	GLuint* hitBuffer;
	GLuint hitBufferSize;
};
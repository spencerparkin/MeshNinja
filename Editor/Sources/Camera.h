#pragma once

#include "Vector.h"

class Camera
{
public:
	Camera();
	virtual ~Camera();

	virtual void PreRender(int renderMode);
	virtual void PostRender(int renderMode);

	void StrafeLeftRight(double delta);
	void MoveForwardBackward(double delta);
	void LookLeftRight(double angleDelta);
	void LookUpDown(double angleDelta);

	void MakeFrame(MeshNinja::Vector& xAxis, MeshNinja::Vector& yAxis, MeshNinja::Vector& zAxis) const;

	MeshNinja::Vector position;
	MeshNinja::Vector target;
	double foviAngle;
	double aspectRatio;
};
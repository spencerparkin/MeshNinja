#pragma once

#include <wx/glcanvas.h>

class Camera;

class Scene
{
public:
	Scene();
	virtual ~Scene();

	virtual void Render(GLint renderMode, const Camera* camera) const = 0;
};
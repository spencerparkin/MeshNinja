#pragma once

#include <wx/glcanvas.h>

class Scene
{
public:
	Scene();
	virtual ~Scene();

	virtual void Render(GLint renderMode) = 0;
};
#include "DebugDrawGL.h"
#include <wx/glcanvas.h>

//----------------------------- DebugDrawGL -----------------------------

DebugDrawGL::DebugDrawGL()
{
}

/*virtual*/ DebugDrawGL::~DebugDrawGL()
{
}

/*virtual*/ void DebugDrawGL::Render(GLint renderMode, const Camera* camera) const
{
	if (renderMode == GL_RENDER)
		this->Draw();
}

/*virtual*/ MeshNinja::DebugDraw::Object* DebugDrawGL::Factory(const std::string& objectType) const
{
	if (objectType == "point")
		return new PointGL();
	else if (objectType == "line")
		return new LineGL();

	return nullptr;
}

/*virtual*/ bool DebugDrawGL::Draw() const
{
	if (this->objectArray->size() == 0)
		return true;

	glPointSize(3.0f);
	glBegin(GL_POINTS);

	for (const MeshNinja::DebugDraw::Object* object : *this->objectArray)
	{
		if (object->GetType() == "point")
		{
			const PointGL* point = (const PointGL*)object;
			point->Draw();
		}
	}

	glEnd();
	glPointSize(1.0f);
	glLineWidth(1.5f);
	glBegin(GL_LINES);

	for (const MeshNinja::DebugDraw::Object* object : *this->objectArray)
	{
		if (object->GetType() == "line")
		{
			const LineGL* line = (const LineGL*)object;
			line->Draw();
		}
	}

	glEnd();
	glLineWidth(1.0f);

	return true;
}

//----------------------------- DebugDrawGL::PointGL -----------------------------

DebugDrawGL::PointGL::PointGL()
{
}

/*virtual*/ DebugDrawGL::PointGL::~PointGL()
{
}

/*virtual*/ bool DebugDrawGL::PointGL::Draw() const
{
	glColor3dv(&this->color.x);
	glVertex3dv(&this->vertex.x);
	return true;
}

//----------------------------- DebugDrawGL::LineGL -----------------------------

DebugDrawGL::LineGL::LineGL()
{
}

/*virtual*/ DebugDrawGL::LineGL::~LineGL()
{
}

/*virtual*/ bool DebugDrawGL::LineGL::Draw() const
{
	glColor3dv(&this->color.x);
	glVertex3dv(&this->vertex[0].x);
	glVertex3dv(&this->vertex[1].x);
	return true;
}
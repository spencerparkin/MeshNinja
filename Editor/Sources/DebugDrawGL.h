#pragma once

#include "DebugDraw.h"
#include "Scene.h"

class DebugDrawGL : public Scene::Object, public MeshNinja::DebugDraw
{
public:
	DebugDrawGL();
	virtual ~DebugDrawGL();

	virtual bool Draw() const override;
	virtual MeshNinja::DebugDraw::Object* Factory(const std::string& objectType) const override;
	virtual void Render(GLint renderMode, const Camera* camera) const override;

	class PointGL : public Point
	{
	public:
		PointGL();
		virtual ~PointGL();

		virtual bool Draw() const override;
	};

	class LineGL : public Line
	{
	public:
		LineGL();
		virtual ~LineGL();

		virtual bool Draw() const override;
	};
};
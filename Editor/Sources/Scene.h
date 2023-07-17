#pragma once

#include <wx/glcanvas.h>

class Camera;

class Scene
{
public:
	Scene();
	virtual ~Scene();

	class Object;

	virtual void Render(GLint renderMode, const Camera* camera) const;
	virtual void HandlePick(const Object* object, bool shiftDown);
	virtual int GetSceneObjectCount() const = 0;
	virtual const Object* GetSceneObject(int i) const = 0;

	const Object* ProcessHitBuffer(const GLuint* hitBuffer, GLuint hitBufferSize, GLuint hitCount) const;

	class Object
	{
	public:
		Object();
		virtual ~Object();

		virtual void Render(GLint renderMode, const Camera* camera) const = 0;

		GLuint id;
		static GLuint nextId;
	};
};
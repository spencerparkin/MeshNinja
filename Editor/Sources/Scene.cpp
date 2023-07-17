#include "Scene.h"

Scene::Scene()
{
}

/*virtual*/ Scene::~Scene()
{
}

/*virtual*/ void Scene::Render(GLint renderMode, const Camera* camera) const
{
	for (int i = 0; i < this->GetSceneObjectCount(); i++)
		this->GetSceneObject(i)->Render(renderMode, camera);
}

/*virtual*/ void Scene::HandlePick(const Object* object, bool shiftDown)
{
}

/*virtual*/ void Scene::HandleTransform(const MeshNinja::Transform& transform)
{
}

const Scene::Object* Scene::ProcessHitBuffer(const GLuint* hitBuffer, GLuint hitBufferSize, GLuint hitCount) const
{
	const GLuint* hitRecord = hitBuffer;

	int pickedId = -1;
	float smallestZ = FLT_MAX;
	for (int i = 0; i < (signed)hitCount; i++)
	{
		GLuint nameCount = hitRecord[0];
		float minZ = float(hitRecord[1]) / float(0x7FFFFFFF);		
		if (minZ < smallestZ)
		{
			smallestZ = minZ;

			for (int j = 0; j < (signed)nameCount; j++)
			{
				int id = (signed)hitRecord[3 + j];
				if (id >= 0)
					pickedId = id;
			}
		}

		hitRecord += 3 + nameCount;
	}

	for (int i = 0; i < this->GetSceneObjectCount(); i++)
	{
		const Object* object = this->GetSceneObject(i);
		if (object->id == pickedId)
			return object;
	}

	return nullptr;
}

GLuint Scene::Object::nextId = 0;

Scene::Object::Object()
{
	this->id = nextId++;
}

/*virtual*/ Scene::Object::~Object()
{
}
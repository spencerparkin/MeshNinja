#pragma once

#include "Scene.h"
#include "DebugDrawGL.h"
#include <vector>

class Mesh;

class MeshCollectionScene : public Scene
{
public:
	MeshCollectionScene();
	virtual ~MeshCollectionScene();

	virtual void Render(GLint renderMode, const Camera* camera) const override;
	virtual void HandlePick(const Object* object, bool shiftDown) override;
	virtual void HandleTransform(const MeshNinja::Transform& transform) override;
	virtual void HandleKey(char key) override;
	virtual int GetSceneObjectCount() const override;
	virtual const Object* GetSceneObject(int i) const override;

	void Clear();

	typedef std::vector<Mesh*> MeshList;

	const MeshList& GetMeshList() const { return this->meshList; }
	MeshList& GetMeshList() { return this->meshList; }

	const Mesh* FindFirstSelectedMesh() const;
	Mesh* FindFirstSelectedMesh();

	bool GetSelectedMeshes(std::list<Mesh*>& selectedMeshList, bool sortedByTimeStamp);

	void ForAllMeshes(std::function<void(Mesh*)> callback, bool mustBeSelected = false);

	DebugDrawGL debugDraw;

private:
	MeshList meshList;
};
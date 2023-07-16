#pragma once

#include "Scene.h"
#include <vector>

class Mesh;

class MeshCollectionScene : public Scene
{
public:
	MeshCollectionScene();
	virtual ~MeshCollectionScene();

	virtual void Render(GLint renderMode, const Camera* camera) const override;

	void Clear();

	typedef std::vector<Mesh*> MeshList;

	const MeshList& GetMeshList() const { return this->meshList; }
	MeshList& GetMeshList() { return this->meshList; }

	const Mesh* FindFirstSelectedMesh() const;
	Mesh* FindFirstSelectedMesh();

private:
	MeshList meshList;
};
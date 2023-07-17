#include "MeshCollectionScene.h"
#include "Mesh.h"
#include "Application.h"
#include "Frame.h"

MeshCollectionScene::MeshCollectionScene()
{
}

/*virtual*/ MeshCollectionScene::~MeshCollectionScene()
{
	this->Clear();
}

void MeshCollectionScene::Clear()
{
	for (Mesh* mesh : this->meshList)
		delete mesh;

	this->meshList.clear();
}

/*virtual*/ int MeshCollectionScene::GetSceneObjectCount() const
{
	return this->meshList.size();
}

/*virtual*/ const Scene::Object* MeshCollectionScene::GetSceneObject(int i) const
{
	if (i < 0 || i >= (signed)this->meshList.size())
		return nullptr;

	return this->meshList[i];
}

/*virtual*/ void MeshCollectionScene::HandlePick(const Object* object, bool shiftDown)
{
	const Mesh* mesh = dynamic_cast<const Mesh*>(object);
	if (!mesh || !shiftDown)
		for (int i = 0; i < (signed)this->meshList.size(); i++)
			this->meshList[i]->SetSelected(false);
	
	if (mesh)
		mesh->SetSelected(!mesh->GetSelected());

	wxCommandEvent sceneChangedEvent(EVT_SCENE_CHANGED);
	wxPostEvent(wxGetApp().GetFrame(), sceneChangedEvent);
}

/*virtual*/ void MeshCollectionScene::HandleTransform(const MeshNinja::Transform& transform)
{
	this->ForAllMeshes([&transform](Mesh* mesh)
		{
			if (transform.translation != MeshNinja::Vector(0.0, 0.0, 0.0))
				mesh->transform = transform * mesh->transform;
			else
			{
				// Conjugation makes more sense in this case, but for some reason I failed to make that work.
				mesh->transform.matrix = transform.matrix * mesh->transform.matrix;
			}
		}, true);
}

/*virtual*/ void MeshCollectionScene::HandleKey(char key)
{
	if (key == ' ')
	{
		this->ForAllMeshes([](Mesh* mesh)
			{
				mesh->SetVisible(!mesh->GetVisible());
			}, true);

		wxCommandEvent sceneChangedEvent(EVT_SCENE_CHANGED);
		wxPostEvent(wxGetApp().GetFrame(), sceneChangedEvent);
	}
}

/*virtual*/ void MeshCollectionScene::Render(GLint renderMode, const Camera* camera) const
{
	if (renderMode == GL_SELECT)
		glPushName(-1);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glLineWidth(1.0f);

	Scene::Render(renderMode, camera);

	if (renderMode == GL_SELECT)
		glPopName();
}

const Mesh* MeshCollectionScene::FindFirstSelectedMesh() const
{
	return const_cast<MeshCollectionScene*>(this)->FindFirstSelectedMesh();
}

Mesh* MeshCollectionScene::FindFirstSelectedMesh()
{
	for (Mesh* mesh : this->meshList)
		if (mesh->GetSelected())
			return mesh;

	return nullptr;
}

void MeshCollectionScene::ForAllMeshes(std::function<void(Mesh*)> callback, bool mustBeSelected /*= false*/)
{
	for (Mesh* mesh : this->meshList)
		if (!mustBeSelected || mesh->GetSelected())
			callback(mesh);
}
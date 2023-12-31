#include "MeshCollectionScene.h"
#include "Mesh.h"
#include "Application.h"
#include "Frame.h"
#include "Camera.h"

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
	this->debugDraw.Clear();
}

/*virtual*/ int MeshCollectionScene::GetSceneObjectCount() const
{
	return this->meshList.size() + 1;
}

/*virtual*/ const Scene::Object* MeshCollectionScene::GetSceneObject(int i) const
{
	if (0 <= i && i < (signed)this->meshList.size())
		return this->meshList[i];

	if (i == this->meshList.size())
		return &this->debugDraw;

	return nullptr;
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
			if (transform.translation != MeshNinja::Vector3(0.0, 0.0, 0.0))
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
	switch (key)
	{
		case ' ':
		{
			this->ForAllMeshes([](Mesh* mesh)
				{
					mesh->SetVisible(!mesh->GetVisible());
				}, true);

			wxCommandEvent sceneChangedEvent(EVT_SCENE_CHANGED);
			wxPostEvent(wxGetApp().GetFrame(), sceneChangedEvent);
			break;
		}
		case WXK_DELETE:
		{
			for (int i = this->meshList.size() - 1; i >= 0; i--)
			{
				Mesh* mesh = this->meshList[i];
				if (mesh->GetSelected())
				{
					delete mesh;
					if (i != this->meshList.size() - 1)
						this->meshList[i] = this->meshList[this->meshList.size() - 1];
					this->meshList.pop_back();		
				}
			}

			wxCommandEvent sceneChangedEvent(EVT_SCENE_CHANGED);
			wxPostEvent(wxGetApp().GetFrame(), sceneChangedEvent);
			break;
		}
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

	if (renderMode == GL_RENDER)
	{
		if (wxGetApp().lightingMode != Application::LightingMode::UNLIT)
		{
			GLfloat lightColor[] = { 1.f, 1.f, 1.f, 1.f };
			GLfloat lightPos[] = { (GLfloat)camera->position.x, (GLfloat)camera->position.y, (GLfloat)camera->position.z, 1.0f };
			GLfloat lightSpec[] = { 0.1f, 0.1f, 0.1f, 0.1f };
			glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
			glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
			glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);
		}
	}

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

bool MeshCollectionScene::GetSelectedMeshes(std::list<Mesh*>& selectedMeshList, bool sortedByTimeStamp)
{
	selectedMeshList.clear();

	for (Mesh* mesh : this->meshList)
		if (mesh->GetSelected())
			selectedMeshList.push_back(mesh);

	if (sortedByTimeStamp)
	{
		selectedMeshList.sort([](Mesh* meshA, Mesh* meshB) -> bool
			{
				return meshA->GetSelectionTime() < meshB->GetSelectionTime();
			});
	}

	return selectedMeshList.size() > 0;
}
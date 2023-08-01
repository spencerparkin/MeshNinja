#include "BoundingBoxTree.h"

using namespace MeshNinja;

//------------------------------ BoundingBoxTree ------------------------------

BoundingBoxTree::BoundingBoxTree()
{
	this->rootNode = nullptr;
}

/*virtual*/ BoundingBoxTree::~BoundingBoxTree()
{
	this->Clear();
}

void BoundingBoxTree::Clear()
{
	delete this->rootNode;
	this->rootNode = nullptr;
}

bool BoundingBoxTree::Insert(Object* object)
{
	if (!this->rootNode)
		return false;

	return this->rootNode->Insert(object);
}

void BoundingBoxTree::Rebuild(const std::vector<Object*>& objectArray)
{
	this->Clear();

	if (objectArray.size() == 0)
		return;

	AxisAlignedBoundingBox aabb = objectArray[0]->GetBoundingBox();

	for (Object* object : objectArray)
		aabb.Merge(aabb, object->GetBoundingBox());

	this->rootNode = new Node(aabb);

	for (Object* object : objectArray)
	{
		bool inserted = this->Insert(object);
		assert(inserted);
	}
}

void BoundingBoxTree::ForOverlappingObjects(const AxisAlignedBoundingBox& aabb, std::function<bool(Object*)> callback)
{
	if (!this->rootNode)
		return;

	std::list<Node*> nodeQueue;
	nodeQueue.push_back(this->rootNode);
	while (nodeQueue.size() > 0)
	{
		std::list<Node*>::iterator iter = nodeQueue.begin();
		Node* node = *iter;
		nodeQueue.erase(iter);

		if (node->aabb.OverlapsWith(aabb))
		{
			for (Object* object : node->objectArray)
				if (!callback(object))
					return;

			for (Node* childNode : node->childArray)
				nodeQueue.push_back(childNode);
		}
	}
}

//------------------------------ BoundingBoxTree::Object ------------------------------

BoundingBoxTree::Object::Object()
{
}

/*virtual*/ BoundingBoxTree::Object::~Object()
{
}

//------------------------------ BoundingBoxTree::Node ------------------------------
	
BoundingBoxTree::Node::Node(const AxisAlignedBoundingBox& aabb)
{
	this->aabb = aabb;
}

/*virtual*/ BoundingBoxTree::Node::~Node()
{
	for (Node* childNode : this->childArray)
		delete childNode;
}

bool BoundingBoxTree::Node::Insert(Object* object)
{
	if (!this->aabb.ContainsBox(object->GetBoundingBox()))
		return false;

	if (this->childArray.size() == 0)
	{
		AxisAlignedBoundingBox aabbA, aabbB;
		this->aabb.SplitReasonably(aabbA, aabbB);
		this->childArray.push_back(new Node(aabbA));
		this->childArray.push_back(new Node(aabbB));
	}

	for (Node* childNode : this->childArray)
		if (childNode->Insert(object))
			return true;

	this->objectArray.push_back(object);
	return true;
}
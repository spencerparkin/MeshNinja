#include "BoundingBoxTree.h"
#include "Ray.h"

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
			{
				if (aabb.OverlapsWith(object->GetBoundingBox()))
				{
					if (!callback(object))
						return;
				}
			}

			for (Node* childNode : node->childArray)
				nodeQueue.push_back(childNode);
		}
	}
}

void BoundingBoxTree::ForHitObjects(const Ray& ray, std::function<bool(Object*, double)> callback)
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

		double alpha = 0.0;
		if (ray.CastAgainst(node->aabb, alpha))
		{
			for (Object* object : node->objectArray)
			{
				if (ray.CastAgainst(object->GetBoundingBox(), alpha))
				{
					if (object->IsHitByRay(ray, alpha))
					{
						if (!callback(object, alpha))
							return;
					}
				}
			}

			for (Node* childNode : node->childArray)
				nodeQueue.push_back(childNode);
		}
	}
}

BoundingBoxTree::Object* BoundingBoxTree::FindClosestHit(const Ray& ray, double* beta /*= nullptr*/)
{
	BoundingBoxTree::Object* foundObject = nullptr;
	double smallestAlpha = DBL_MAX;

	this->ForHitObjects(ray, [&foundObject, &ray, &smallestAlpha](Object* object, double alpha) -> bool
		{
			if (alpha < smallestAlpha)
			{
				smallestAlpha = alpha;
				foundObject = object;
			}
			return true;
		});

	if (beta)
		*beta = smallestAlpha;

	return foundObject;
}

bool BoundingBoxTree::GetBoundingBox(AxisAlignedBoundingBox& box) const
{
	if (!this->rootNode)
		return false;

	box = this->rootNode->aabb;
	return true;
}

bool BoundingBoxTree::IsEmpty() const
{
	return this->Size() == 0;
}

int BoundingBoxTree::Size() const
{
	if (!this->rootNode)
		return 0;

	return this->rootNode->Count();
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

	for (Object* object : this->objectArray)
		delete object;
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

int BoundingBoxTree::Node::Count() const
{
	int count = this->objectArray.size();

	for (Node* childNode : this->childArray)
		count += childNode->Count();

	return count;
}
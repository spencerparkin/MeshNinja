#include "PointCloud.h"
#include "AxisAlignedBoundingBox.h"

using namespace MeshNinja;

//----------------------------------- PointCloud -----------------------------------

PointCloud::PointCloud()
{
	this->rootSpace = nullptr;
}

/*virtual*/ PointCloud::~PointCloud()
{
	this->Clear();
}

void PointCloud::Clear()
{
	delete this->rootSpace;
	this->rootSpace = nullptr;
}

void PointCloud::ToPointArray(std::vector<Vector3>& pointArray) const
{
	if (this->rootSpace)
		this->rootSpace->ToPointArray(pointArray);
}

void PointCloud::FromPointArray(const std::vector<Vector3>& pointArray, int maxPointsPerLeaf /*= 10*/)
{
	this->Clear();

	if ((signed)pointArray.size() > maxPointsPerLeaf)
		this->rootSpace = new InternalNode();
	else
		this->rootSpace = new LeafNode();

	this->rootSpace->FromPointArray(pointArray, maxPointsPerLeaf);
}

const Vector3* PointCloud::FindNearestPoint(const Vector3& givenPoint, double& distance) const
{
	if (!this->rootSpace)
		return nullptr;

	return this->rootSpace->FindNearestPoint(givenPoint, distance);
}

//----------------------------------- PointCloud::Node -----------------------------------

PointCloud::Node::Node()
{
}

/*virtual*/ PointCloud::Node::~Node()
{
}

//----------------------------------- PointCloud::InternalNode -----------------------------------

PointCloud::InternalNode::InternalNode()
{
	this->backSpace = nullptr;
	this->frontSpace = nullptr;
}

/*virtual*/ PointCloud::InternalNode::~InternalNode()
{
	delete this->backSpace;
	delete this->frontSpace;
}

/*virtual*/ void PointCloud::InternalNode::ToPointArray(std::vector<Vector3>& givenPointArray) const
{
	if (this->backSpace)
		this->backSpace->ToPointArray(givenPointArray);

	if (this->frontSpace)
		this->frontSpace->ToPointArray(givenPointArray);
}

void PointCloud::InternalNode::FromPointArray(const std::vector<Vector3>& givenPointArray, int maxPointsPerLeaf)
{
	AxisAlignedBoundingBox box(givenPointArray[0]);
	Vector3 averagePoint(0.0, 0.0, 0.0);
	for (const Vector3& point : givenPointArray)
	{
		box.ExpandToIncludePoint(point);
		averagePoint += point;
	}

	averagePoint /= double(givenPointArray.size());

	double width = box.Width();
	double height = box.Height();
	double depth = box.Depth();

	double largestDimension = MESH_NINJA_MAX(width, MESH_NINJA_MAX(height, depth));

	if (largestDimension == width)
		this->partitioningPlane = Plane(averagePoint, Vector3(1.0, 0.0, 0.0));
	else if (largestDimension == height)
		this->partitioningPlane = Plane(averagePoint, Vector3(0.0, 1.0, 0.0));
	else if (largestDimension == depth)
		this->partitioningPlane = Plane(averagePoint, Vector3(0.0, 0.0, 1.0));

	std::vector<Vector3> backPointArray, frontPointArray;

	for (const Vector3& point : givenPointArray)
	{
		Plane::Side side = this->partitioningPlane.WhichSide(point, 0.0);
		switch (side)
		{
			case Plane::Side::BACK:
			{
				backPointArray.push_back(point);
				break;
			}
			case Plane::Side::FRONT:
			case Plane::Side::NEITHER:
			{
				frontPointArray.push_back(point);
				break;
			}
		}
	}

	if (backPointArray.size() > 0)
	{
		this->backSpace = ((signed)backPointArray.size() < maxPointsPerLeaf) ? (Node*)new LeafNode() : (Node*)new InternalNode();
		this->backSpace->FromPointArray(backPointArray, maxPointsPerLeaf);
	}

	if (frontPointArray.size() > 0)
	{
		this->frontSpace = ((signed)frontPointArray.size() < maxPointsPerLeaf) ? (Node*)new LeafNode() : (Node*)new InternalNode();
		this->frontSpace->FromPointArray(frontPointArray, maxPointsPerLeaf);
	}
}

const Vector3* PointCloud::InternalNode::FindNearestPoint(const Vector3& givenPoint, double& distance) const
{
	Plane::Side side = this->partitioningPlane.WhichSide(givenPoint);

	Node* firstNode = nullptr;
	Node* secondNode = nullptr;

	switch (side)
	{
		case Plane::Side::BACK:
		{
			firstNode = this->backSpace;
			secondNode = this->frontSpace;
			break;
		}
		case Plane::Side::FRONT:
		case Plane::Side::NEITHER:
		{
			firstNode = this->frontSpace;
			secondNode = this->backSpace;
			break;
		}
	}

	double firstDistance = 0.0;
	const Vector3* firstNearestPoint = firstNode->FindNearestPoint(givenPoint, firstDistance);
	if (firstNearestPoint)
	{
		double distanceToPlane = ::abs(this->partitioningPlane.SignedDistanceToPoint(givenPoint));
		if (distanceToPlane > firstDistance)
		{
			distance = firstDistance;
			return firstNearestPoint;
		}
	}

	double secondDistance = 0.0;
	const Vector3* secondNearestPoint = secondNode->FindNearestPoint(givenPoint, secondDistance);
	if (!secondNearestPoint || firstDistance < secondDistance)
	{
		distance = firstDistance;
		return firstNearestPoint;
	}

	distance = secondDistance;
	return secondNearestPoint;
}

//----------------------------------- PointCloud::LeafNode -----------------------------------

PointCloud::LeafNode::LeafNode()
{
}

/*virtual*/ PointCloud::LeafNode::~LeafNode()
{
}

/*virtual*/ void PointCloud::LeafNode::ToPointArray(std::vector<Vector3>& givenPointArray) const
{
	for (const Vector3& point : this->pointArray)
		givenPointArray.push_back(point);
}

/*virtual*/ void PointCloud::LeafNode::FromPointArray(const std::vector<Vector3>& givenPointArray, int maxPointsPerLeaf)
{
	this->pointArray = givenPointArray;
}

/*virtual*/ const Vector3* PointCloud::LeafNode::FindNearestPoint(const Vector3& givenPoint, double& distance) const
{
	distance = DBL_MAX;
	const Vector3* closestPoint = nullptr;

	for (const Vector3& point : this->pointArray)
	{
		double otherDistance = (point - givenPoint).Length();
		if (otherDistance < distance)
		{
			distance = otherDistance;
			closestPoint = &point;
		}
	}

	return closestPoint;
}
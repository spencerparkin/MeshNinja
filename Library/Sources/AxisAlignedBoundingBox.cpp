#include "AxisAlignedBoundingBox.h"

using namespace MeshNinja;

AxisAlignedBoundingBox::AxisAlignedBoundingBox()
{
}

AxisAlignedBoundingBox::AxisAlignedBoundingBox(const Vector& point)
{
	this->min = point;
	this->max = point;
}

AxisAlignedBoundingBox::AxisAlignedBoundingBox(const Vector& min, const Vector& max)
{
	this->min = min;
	this->max = max;
}

AxisAlignedBoundingBox::AxisAlignedBoundingBox(const AxisAlignedBoundingBox& aabb)
{
	this->min = aabb.min;
	this->max = aabb.max;
}

/*virtual*/ AxisAlignedBoundingBox::~AxisAlignedBoundingBox()
{
}

bool AxisAlignedBoundingBox::IsValid() const
{
	if (this->min.x > this->max.x)
		return false;

	if (this->min.y > this->max.y)
		return false;

	if (this->min.z > this->max.z)
		return false;

	return true;
}

double AxisAlignedBoundingBox::Width() const
{
	return max.x - min.x;
}

double AxisAlignedBoundingBox::Height() const
{
	return max.y - min.y;
}

double AxisAlignedBoundingBox::Depth() const
{
	return max.z - min.z;
}

double AxisAlignedBoundingBox::Volume() const
{
	return this->Width() * this->Height() * this->Depth();
}

Vector AxisAlignedBoundingBox::Center() const
{
	return (this->min + this->max) / 2.0;
}

bool AxisAlignedBoundingBox::ContainsPoint(const Vector& point, double eps /*= MESH_NINJA_EPS*/) const
{
	return (
		this->min.x - eps <= point.x && point.x <= this->max.x + eps &&
		this->min.y - eps <= point.y && point.y <= this->max.y + eps &&
		this->min.z - eps <= point.z && point.z <= this->max.z + eps);
}

bool AxisAlignedBoundingBox::ContainsInteriorPoint(const Vector& point, double eps /*= MESH_NINJA_EPS*/) const
{
	return (
		this->min.x + eps <= point.x && point.x <= this->max.x - eps &&
		this->min.y + eps <= point.y && point.y <= this->max.y - eps &&
		this->min.z + eps <= point.z && point.z <= this->max.z - eps);
}

bool AxisAlignedBoundingBox::ContainsPointOnBoundary(const Vector& point, double eps /*= MESH_NINJA_EPS*/) const
{
	return this->ContainsPoint(point) && !this->ContainsInteriorPoint(point, eps);
}

void AxisAlignedBoundingBox::ExpandToIncludePoint(const Vector& point)
{
	this->min.x = MESH_NINJA_MIN(this->min.x, point.x);
	this->min.y = MESH_NINJA_MIN(this->min.y, point.y);
	this->min.z = MESH_NINJA_MIN(this->min.z, point.z);

	this->max.x = MESH_NINJA_MAX(this->max.x, point.x);
	this->max.y = MESH_NINJA_MAX(this->max.y, point.y);
	this->max.z = MESH_NINJA_MAX(this->max.z, point.z);
}

bool AxisAlignedBoundingBox::Intersect(const AxisAlignedBoundingBox& aabbA, const AxisAlignedBoundingBox& aabbB)
{
	// TODO: Write this.
	return false;
}

bool AxisAlignedBoundingBox::Merge(const AxisAlignedBoundingBox& aabbA, const AxisAlignedBoundingBox& aabbB)
{
	this->min = aabbA.min;
	this->max = aabbA.max;

	this->ExpandToIncludePoint(aabbB.min);
	this->ExpandToIncludePoint(aabbB.max);

	return true;
}

bool AxisAlignedBoundingBox::OverlapsWith(const AxisAlignedBoundingBox& aabb) const
{
	AxisAlignedBoundingBox intersection;
	return intersection.Intersect(*this, aabb);
}

void AxisAlignedBoundingBox::SplitReasonably(AxisAlignedBoundingBox& aabbA, AxisAlignedBoundingBox& aabbB) const
{
	double width = this->Width();
	double height = this->Height();
	double depth = this->Depth();

	double maxDimension = MESH_NINJA_MAX(width, MESH_NINJA_MAX(height, depth));

	Vector center = this->Center();

	aabbA.min = this->min;
	aabbA.max = this->max;
	aabbB.min = this->min;
	aabbB.max = this->max;

	if (width == maxDimension)
	{
		aabbA.max.x = center.x;
		aabbB.min.x = center.x;
	}
	else if (height == maxDimension)
	{
		aabbA.max.y = center.y;
		aabbB.min.y = center.y;
	}
	else if (depth == maxDimension)
	{
		aabbA.max.z = center.z;
		aabbB.min.z = center.z;
	}
}
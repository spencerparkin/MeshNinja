#pragma once

#include "Vector.h"
#include "AxisAlignedBoundingBox.h"

namespace MeshNinja
{
	class Ray;

	class MESH_NINJA_API BoundingBoxTree
	{
	public:
		BoundingBoxTree();
		virtual ~BoundingBoxTree();

		class Object;

		void Clear();
		bool Insert(Object* object);
		void Rebuild(const std::vector<Object*>& objectArray);
		void ForOverlappingObjects(const AxisAlignedBoundingBox& aabb, std::function<bool(Object*)> callback);
		void ForHitObjects(const Ray& ray, std::function<bool(Object*, double)> callback);
		bool GetBoundingBox(AxisAlignedBoundingBox& box) const;
		Object* FindClosestHit(const Ray& ray, double* beta = nullptr);

		class MESH_NINJA_API Object
		{
		public:
			Object();
			virtual ~Object();

			virtual AxisAlignedBoundingBox GetBoundingBox() const = 0;
			virtual bool IsHitByRay(const Ray& ray, double& alpha) const = 0;
		};

		template<typename T>
		class TemplateObject : public Object
		{
		public:
			TemplateObject()
			{
			}

			virtual ~TemplateObject()
			{
			}

			virtual AxisAlignedBoundingBox GetBoundingBox() const override
			{
				return this->thing.GetBoundingBox();
			}

		protected:
			T thing;
		};

	protected:

		class Node
		{
		public:
			Node(const AxisAlignedBoundingBox& aabb);
			virtual ~Node();

			bool Insert(Object* object);

			std::vector<Object*> objectArray;
			std::vector<Node*> childArray;
			AxisAlignedBoundingBox aabb;
		};

		Node* rootNode;
	};
}
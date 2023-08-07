#pragma once

#include "Common.h"
#include "Plane.h"

namespace MeshNinja
{
	class MESH_NINJA_API PointCloud
	{
	public:
		PointCloud();
		virtual ~PointCloud();

		void Clear();
		void ToPointArray(std::vector<Vector3>& pointArray) const;
		void FromPointArray(const std::vector<Vector3>& pointArray, int maxPointsPerLeaf = 10);

		const Vector3* FindNearestPoint(const Vector3& givenPoint, double& distance) const;

		class Node
		{
		public:
			Node();
			virtual ~Node();

			virtual void ToPointArray(std::vector<Vector3>& givenPointArray) const = 0;
			virtual void FromPointArray(const std::vector<Vector3>& givenPointArray, int maxPointsPerLeaf) = 0;
			virtual const Vector3* FindNearestPoint(const Vector3& givenPoint, double& distance) const = 0;
		};

		class InternalNode : public Node
		{
		public:
			InternalNode();
			virtual ~InternalNode();

			virtual void ToPointArray(std::vector<Vector3>& givenPointArray) const override;
			virtual void FromPointArray(const std::vector<Vector3>& givenPointArray, int maxPointsPerLeaf) override;
			virtual const Vector3* FindNearestPoint(const Vector3& givenPoint, double& distance) const override;

			Plane partitioningPlane;
			Node* backSpace;
			Node* frontSpace;
		};

		class LeafNode : public Node
		{
		public:
			LeafNode();
			virtual ~LeafNode();

			virtual void ToPointArray(std::vector<Vector3>& givenPointArray) const override;
			virtual void FromPointArray(const std::vector<Vector3>& givenPointArray, int maxPointsPerLeaf) override;
			virtual const Vector3* FindNearestPoint(const Vector3& givenPoint, double& distance) const override;

			std::vector<Vector3> pointArray;
		};

	protected:

		Node* rootSpace;
	};
}
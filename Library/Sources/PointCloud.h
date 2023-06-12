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
		void ToPointArray(std::vector<Vector>& pointArray) const;
		void FromPointArray(const std::vector<Vector>& pointArray, int maxPointsPerLeaf = 10);

		const Vector* FindNearestPoint(const Vector& givenPoint, double& distance) const;

		class Node
		{
		public:
			Node();
			virtual ~Node();

			virtual void ToPointArray(std::vector<Vector>& givenPointArray) const = 0;
			virtual void FromPointArray(const std::vector<Vector>& givenPointArray, int maxPointsPerLeaf) = 0;
			virtual const Vector* FindNearestPoint(const Vector& givenPoint, double& distance) const = 0;
		};

		class InternalNode : public Node
		{
		public:
			InternalNode();
			virtual ~InternalNode();

			virtual void ToPointArray(std::vector<Vector>& givenPointArray) const override;
			virtual void FromPointArray(const std::vector<Vector>& givenPointArray, int maxPointsPerLeaf) override;
			virtual const Vector* FindNearestPoint(const Vector& givenPoint, double& distance) const override;

			Plane partitioningPlane;
			Node* backSpace;
			Node* frontSpace;
		};

		class LeafNode : public Node
		{
		public:
			LeafNode();
			virtual ~LeafNode();

			virtual void ToPointArray(std::vector<Vector>& givenPointArray) const override;
			virtual void FromPointArray(const std::vector<Vector>& givenPointArray, int maxPointsPerLeaf) override;
			virtual const Vector* FindNearestPoint(const Vector& givenPoint, double& distance) const override;

			std::vector<Vector> pointArray;
		};

	protected:

		Node* rootSpace;
	};
}
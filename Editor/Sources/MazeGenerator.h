#pragma once

#include "Math/Vector.h"
#include "ConvexPolygonMesh.h"

class MazeGenerator
{
public:
	MazeGenerator();
	virtual ~MazeGenerator();

	void Clear();
	bool GenerateGridMaze(int width, int height, int depth, double scale);
	bool GenerateMazeMeshes(std::list<MeshNinja::ConvexPolygonMesh*>& meshList, double radius, bool unionize) const;
	bool WriteJsonNavigationFile(const std::string& filePath) const;

private:

	bool GenerateMaze();
	int RandomInt(int min, int max) const;
	void RandomPerm(int size, std::vector<int>& perm) const;
	bool GenerateTunnelMesh(MeshNinja::ConvexPolygonMesh* mesh, const MeshNinja::Vector& pointA, const MeshNinja::Vector& pointB, int sides, double radius) const;

	class Node
	{
	public:
		Node();
		virtual ~Node();

		bool ConnectedTo(const Node* node) const;
		Node* FindRoot() const;
		bool ConnectWith(Node* node);

		std::vector<Node*> adjacencyArray;
		std::vector<Node*> connectionArray;
		mutable Node* parentNode;
		MeshNinja::Vector location;
		mutable int i;
	};

	std::vector<Node*> nodeArray;
};
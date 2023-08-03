#include "MazeGenerator.h"
#include "MeshGraph.h"
#include "MeshSetOperation.h"
#include "JSON/JsonValue.h"

//------------------------------- MazeGenerator -------------------------------

MazeGenerator::MazeGenerator()
{
}

/*virtual*/ MazeGenerator::~MazeGenerator()
{
	this->Clear();
}

void MazeGenerator::Clear()
{
	for (Node* node : this->nodeArray)
		delete node;

	this->nodeArray.clear();
}

bool MazeGenerator::GenerateGridMaze(int width, int height, int depth, double scale)
{
	Node**** matrix = new Node***[width];
	for (int i = 0; i < width; i++)
	{
		matrix[i] = new Node**[height];
		for (int j = 0; j < height; j++)
		{
			matrix[i][j] = new Node*[depth];
			for (int k = 0; k < depth; k++)
			{
				Node* node = new Node();
				node->location = MeshNinja::Vector(double(i) - double(width - 1) / 2.0, double(j) - double(height - 1) / 2.0, double(k) - double(depth - 1) / 2.0) * scale;
				matrix[i][j][k] = node;
				this->nodeArray.push_back(node);
			}
		}
	}

	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			for (int k = 0; k < depth; k++)
			{
				Node* node = matrix[i][j][k];

				if (i > 0)
					node->adjacencyArray.push_back(matrix[i - 1][j][k]);
				if (i < width - 1)
					node->adjacencyArray.push_back(matrix[i + 1][j][k]);
				if (j > 0)
					node->adjacencyArray.push_back(matrix[i][j - 1][k]);
				if (j < height - 1)
					node->adjacencyArray.push_back(matrix[i][j + 1][k]);
				if (k > 0)
					node->adjacencyArray.push_back(matrix[i][j][k - 1]);
				if (k < depth - 1)
					node->adjacencyArray.push_back(matrix[i][j][k + 1]);
			}
		}
	}

	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
			delete[] matrix[i][j];

		delete[] matrix[i];
	}

	delete[] matrix;

	return this->GenerateMaze();
}

bool MazeGenerator::GenerateMaze()
{
	// Randomly populate the queue.
	std::list<Node*> nodeQueue;
	std::vector<int> perm;
	this->RandomPerm(this->nodeArray.size(), perm);
	for (int i : perm)
		nodeQueue.push_back(this->nodeArray[i]);

	// Until the queue is empty...
	while (nodeQueue.size() > 0)
	{
		// Pull the next node off the queue.
		std::list<Node*>::iterator iter = nodeQueue.begin();
		Node* node = *iter;
		nodeQueue.erase(iter);

		// Try to connect the node up with an adjacency.  Try them all in a random order.
		this->RandomPerm(node->adjacencyArray.size(), perm);
		for (int i : perm)
		{
			Node* adjacentNode = node->adjacencyArray[i];
			if (node->ConnectWith(adjacentNode))
			{
				node->connectionArray.push_back(adjacentNode);
				adjacentNode->connectionArray.push_back(node);
				break;
			}
		}
	}

	return true;
}

int MazeGenerator::RandomInt(int min, int max) const
{
	double t = double(rand()) / double(RAND_MAX);
	int r = min + int(t * double(max - min));
	r = MESH_NINJA_CLAMP(r, min, max);
	return r;
}

void MazeGenerator::RandomPerm(int size, std::vector<int>& perm) const
{
	perm.clear();
	for (int i = 0; i < size; i++)
		perm.push_back(i);

	for (int i = 0; i < size - 1; i++)
	{
		int j = this->RandomInt(i, size - 1);
		if (i != j)
		{
			int k = perm[i];
			perm[i] = perm[j];
			perm[j] = k;
		}
	}
}

bool MazeGenerator::GenerateMazeMeshes(std::list<MeshNinja::ConvexPolygonMesh*>& meshList, double radius, bool unionize) const
{
	meshList.clear();

	int i = 0;
	for (const Node* node : this->nodeArray)
	{
		node->i = i++;

		MeshNinja::ConvexPolygonMesh* mesh = new MeshNinja::ConvexPolygonMesh();
		meshList.push_back(mesh);

		MeshNinja::ConvexPolygonMesh::Polyhedron polyhedron;

		switch (this->RandomInt(0, 4))
		{
			case 0: polyhedron = MeshNinja::ConvexPolygonMesh::Polyhedron::HEXADRON; break;
			case 1: polyhedron = MeshNinja::ConvexPolygonMesh::Polyhedron::CUBOCTAHEDRON; break;
			case 2: polyhedron = MeshNinja::ConvexPolygonMesh::Polyhedron::DODECAHEDRON; break;
			case 3: polyhedron = MeshNinja::ConvexPolygonMesh::Polyhedron::ICOSAHEDRON; break;
			case 4: polyhedron = MeshNinja::ConvexPolygonMesh::Polyhedron::OCTAHEDRON; break;
		}

		if (!mesh->GeneratePolyhedron(polyhedron))
			return false;

		mesh->CenterAndScale(radius);

		MeshNinja::Transform transform;
		transform.SetIdentity();
		transform.translation = node->location;

		mesh->ApplyTransform(transform);
	}

	std::set<MeshNinja::MeshGraph::VertexPair<false>> nodePairSet;

	for (const Node* nodeA : this->nodeArray)
	{
		for (const Node* nodeB : nodeA->connectionArray)
		{
			MeshNinja::MeshGraph::VertexPair<false> nodePair{ nodeA->i, nodeB->i };
			if (nodePairSet.find(nodePair) == nodePairSet.end())
			{
				nodePairSet.insert(nodePair);

				MeshNinja::ConvexPolygonMesh* mesh = new MeshNinja::ConvexPolygonMesh();
				meshList.push_back(mesh);

				MeshNinja::Vector vector = nodeB->location - nodeA->location;
				vector.Normalize();

				MeshNinja::Vector pointA = nodeA->location + vector * radius / 2.0;
				MeshNinja::Vector pointB = nodeB->location - vector * radius / 2.0;

				if (!this->GenerateTunnelMesh(mesh, pointA, pointB, this->RandomInt(4, 8), radius / 3.0))
					return false;
			}
		}
	}
	
	if (unionize)
	{
		std::vector<MeshNinja::ConvexPolygon> nodePolygonArray;
		std::vector<MeshNinja::ConvexPolygon> tunnelPolygonArray;
		int i = 0;
		for (MeshNinja::ConvexPolygonMesh* mesh : meshList)
		{
			if (i++ < (signed)this->nodeArray.size())
				mesh->ToConvexPolygonArray(nodePolygonArray, true);
			else
				mesh->ToConvexPolygonArray(tunnelPolygonArray, true);
			delete mesh;
		}

		meshList.clear();

		MeshNinja::ConvexPolygonMesh nodesMesh(nodePolygonArray);
		MeshNinja::ConvexPolygonMesh tunnelsMesh(tunnelPolygonArray);

		MeshNinja::MeshUnion meshUnion;

		MeshNinja::ConvexPolygonMesh* mazeMesh = new MeshNinja::ConvexPolygonMesh();
		
		if (!meshUnion.Perform(nodesMesh, tunnelsMesh, *mazeMesh))
		{
			delete mazeMesh;
			return false;
		}

		mazeMesh->ReverseAllPolygons();
		meshList.push_back(mazeMesh);
	}

	return true;
}

bool MazeGenerator::GenerateTunnelMesh(MeshNinja::ConvexPolygonMesh* mesh, const MeshNinja::Vector& pointA, const MeshNinja::Vector& pointB, int sides, double radius) const
{
	MeshNinja::Vector zAxis = pointB - pointA;
	zAxis.Normalize();
	MeshNinja::Vector yAxis;
	yAxis.MakeOrthogonalTo(zAxis);
	yAxis.Normalize();
	MeshNinja::Vector xAxis = yAxis.Cross(zAxis);

	std::vector<MeshNinja::Vector> vertexArray[2];

	for (int i = 0; i < sides; i++)
	{
		double angle = -2.0 * MESH_NINJA_PI * (double(i) / double(sides));
		MeshNinja::Vector vector = (xAxis * ::cos(angle) + yAxis * ::sin(angle)) * radius;
		vertexArray[0].push_back(pointA + vector);
		vertexArray[1].push_back(pointB + vector);
	}

	std::vector<MeshNinja::ConvexPolygon> polygonArray;
	for (int i = 0; i < sides; i++)
	{
		int j = (i + 1) % sides;
		MeshNinja::ConvexPolygon polygon;
		polygon.vertexArray->push_back(vertexArray[0][i]);
		polygon.vertexArray->push_back(vertexArray[1][i]);
		polygon.vertexArray->push_back(vertexArray[1][j]);
		polygon.vertexArray->push_back(vertexArray[0][j]);
		polygonArray.push_back(polygon);
	}

	for (int i = 0; i < 2; i++)
	{
		MeshNinja::ConvexPolygon cap;
		for (int j = 0; j < sides; j++)
		{
			int k = (i == 0) ? j : (sides - 1 - j);
			cap.vertexArray->push_back(vertexArray[i][k]);
		}

		polygonArray.push_back(cap);
	}

	mesh->FromConvexPolygonArray(polygonArray);
	return true;
}

bool MazeGenerator::WriteJsonNavigationFile(const std::string& filePath) const
{
	bool success = false;
	MeshNinja::JsonArray* jsonArray = nullptr;

	do
	{
		std::ofstream fileStream(filePath, std::ios::out);
		if (!fileStream.is_open())
			break;

		jsonArray = new MeshNinja::JsonArray();

		for (int i = 0; i < (signed)this->nodeArray.size(); i++)
			this->nodeArray[i]->i = i;

		for (int i = 0; i < (signed)this->nodeArray.size(); i++)
		{
			const Node* node = this->nodeArray[i];

			MeshNinja::JsonObject* jsonNode = new MeshNinja::JsonObject();
			jsonArray->PushValue(jsonNode);

			MeshNinja::JsonObject* jsonLocation = new MeshNinja::JsonObject();
			jsonNode->SetValue("location", jsonLocation);
			jsonLocation->SetValue("x", new MeshNinja::JsonFloat(node->location.x));
			jsonLocation->SetValue("y", new MeshNinja::JsonFloat(node->location.y));
			jsonLocation->SetValue("z", new MeshNinja::JsonFloat(node->location.z));
			
			MeshNinja::JsonArray* jsonConnections = new MeshNinja::JsonArray();
			jsonNode->SetValue("connections", jsonConnections);
			for (int j = 0; j < (signed)node->connectionArray.size(); j++)
				jsonConnections->PushValue(new MeshNinja::JsonInt(node->connectionArray[j]->i));
		}

		std::string jsonString;
		if (!jsonArray->PrintJson(jsonString))
			break;

		fileStream << jsonString;
		fileStream.close();

		success = true;
	} while (false);

	delete jsonArray;

	return success;
}

//------------------------------- MazeGenerator::Node -------------------------------

MazeGenerator::Node::Node()
{
	this->parentNode = nullptr;
	this->i = 0;
}

/*virtual*/ MazeGenerator::Node::~Node()
{
}

// Is this node and the given node connected directly or indirectly?
bool MazeGenerator::Node::ConnectedTo(const Node* node) const
{
	return this->FindRoot() == node->FindRoot();
}

MazeGenerator::Node* MazeGenerator::Node::FindRoot() const
{
	const Node* rootNode = this;
	while (rootNode->parentNode)
		rootNode = rootNode->parentNode;
	
	// This bit of code here is just an optimization and is not needed for correctness.
	const Node* node = this;
	while (node != rootNode)
	{
		const Node* nextNode = node->parentNode;
		node->parentNode = const_cast<Node*>(rootNode);
		node = nextNode;
	}

	return const_cast<Node*>(rootNode);
}

bool MazeGenerator::Node::ConnectWith(Node* node)
{
	if (this->ConnectedTo(node))
		return false;

	Node* rootNode = this->FindRoot();
	rootNode->parentNode = node->FindRoot();
	return true;
}
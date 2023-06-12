#include "ConvexPolygon.h"

using namespace MeshNinja;

ConvexPolygon::ConvexPolygon()
{
	this->vertexArray = new std::vector<Vector>();
}

ConvexPolygon::ConvexPolygon(const ConvexPolygon& polygon)
{
	this->vertexArray = new std::vector<Vector>();
	*this->vertexArray = *polygon.vertexArray;
}

/*virtual*/ ConvexPolygon::~ConvexPolygon()
{
	delete this->vertexArray;
}

void ConvexPolygon::Clear()
{
	this->vertexArray->clear();
}

void ConvexPolygon::Compress(double eps /*= MESH_NINJA_EPS*/)
{
	bool makeAnotherPass = true;
	while (makeAnotherPass)
	{
		makeAnotherPass = false;

		for (int i = 0; i < (signed)this->vertexArray->size(); i++)
		{
			const Vector& vertexA = (*this->vertexArray)[i];
			const Vector& vertexB = (*this->vertexArray)[(i + 1) % this->vertexArray->size()];
			const Vector& vertexC = (*this->vertexArray)[(i + 2) % this->vertexArray->size()];

			double triangleArea = (vertexB - vertexA).Cross(vertexC - vertexA).Length() / 2.0;

			if (triangleArea < eps)
			{
				this->vertexArray->erase(this->vertexArray->begin() + ((i + 1) % this->vertexArray->size()));
				makeAnotherPass = true;
				break;
			}
		}
	}
}
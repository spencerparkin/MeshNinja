#include "ConvexPolygon.h"

using namespace MeshNinja;

ConvexPolygon::ConvexPolygon()
{
	this->vertexArray = new std::vector<Vertex>();
}

/*virtual*/ ConvexPolygon::~ConvexPolygon()
{
	delete this->vertexArray;
}
#include "ConvexPolygon.h"

using namespace MeshNinja;

ConvexPolygon::ConvexPolygon()
{
	this->vertexArray = new std::vector<Vector>();
}

/*virtual*/ ConvexPolygon::~ConvexPolygon()
{
	delete this->vertexArray;
}
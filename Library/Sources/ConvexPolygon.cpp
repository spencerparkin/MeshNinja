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
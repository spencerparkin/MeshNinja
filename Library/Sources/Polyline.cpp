#include "Polyline.h"

using namespace MeshNinja;

Polyline::Polyline()
{
	this->vertexArray = new std::vector<Vector>();
}

Polyline::Polyline(const Polyline& polyline)
{
	this->vertexArray = new std::vector<Vector>();
	*this->vertexArray = *polyline.vertexArray;
}

Polyline::Polyline(const Vector& vertexA, const Vector& vertexB)
{
	this->vertexArray = new std::vector<Vector>();
	this->vertexArray->push_back(vertexA);
	this->vertexArray->push_back(vertexB);
}

/*virtual*/ Polyline::~Polyline()
{
	delete this->vertexArray;
}

const Vector& Polyline::GetFirstVertex() const
{
	return (*this->vertexArray)[0];
}

const Vector& Polyline::GetLastVertex() const
{
	return (*this->vertexArray)[this->vertexArray->size() - 1];
}

bool Polyline::IsLineLoop(double eps /*= MESH_NINJA_EPS*/) const
{
	if (this->vertexArray->size() == 0)
		return false;

	if (this->vertexArray->size() == 1)
		return true;

	return (this->GetLastVertex() - this->GetFirstVertex()).Length() < eps;
}

void Polyline::ReverseOrder()
{
	std::vector<Vector> reversedVertexArray;
	for (int i = this->vertexArray->size() - 1; i >= 0; i--)
		reversedVertexArray.push_back((*this->vertexArray)[i]);

	*this->vertexArray = reversedVertexArray;
}

bool Polyline::Merge(const Polyline& polylineA, const Polyline& polylineB, double eps /*= MESH_NINJA_EPS*/)
{
	if ((polylineA.GetLastVertex() - polylineB.GetFirstVertex()).Length() < eps)
	{
		this->Concatinate(polylineA, polylineB);
		return true;
	}
	else if ((polylineA.GetFirstVertex() - polylineB.GetLastVertex()).Length() < eps)
	{
		this->Concatinate(polylineB, polylineA);
		return true;
	}
	else if ((polylineA.GetLastVertex() - polylineB.GetLastVertex()).Length() < eps)
	{
		Polyline reversePolylineB(polylineB);
		reversePolylineB.ReverseOrder();
		this->Concatinate(polylineA, reversePolylineB);
		return true;
	}
	else if ((polylineA.GetFirstVertex() - polylineB.GetFirstVertex()).Length() < eps)
	{
		Polyline reversePolylineA(polylineA);
		reversePolylineA.ReverseOrder();
		this->Concatinate(reversePolylineA, polylineB);
		return true;
	}

	return false;
}

void Polyline::Concatinate(const Polyline& polylineA, const Polyline& polylineB)
{
	this->vertexArray->clear();

	for (const Vector& vertex : *polylineA.vertexArray)
		this->vertexArray->push_back(vertex);

	for (const Vector& vertex : *polylineB.vertexArray)
		this->vertexArray->push_back(vertex);
}
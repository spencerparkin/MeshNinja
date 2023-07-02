#include "Polyline.h"
#include "Plane.h"
#include "ConvexPolygonMesh.h"
#include "Ray.h"

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
		Polyline copyPolylineA(polylineA);
		copyPolylineA.vertexArray->pop_back();
		this->Concatinate(copyPolylineA, polylineB);
		return true;
	}
	else if ((polylineA.GetFirstVertex() - polylineB.GetLastVertex()).Length() < eps)
	{
		Polyline copyPolylineB(polylineB);
		copyPolylineB.vertexArray->pop_back();
		this->Concatinate(copyPolylineB, polylineA);
		return true;
	}
	else if ((polylineA.GetFirstVertex() - polylineB.GetFirstVertex()).Length() < eps && (polylineA.GetLastVertex() - polylineB.GetLastVertex()).Length() < eps)
	{
		if (polylineA.vertexArray->size() <= polylineB.vertexArray->size())
		{
			Polyline reversePolylineA(polylineA);
			reversePolylineA.ReverseOrder();
			reversePolylineA.vertexArray->pop_back();
			this->Concatinate(reversePolylineA, polylineB);
			return true;
		}
		else
		{
			Polyline reversePolylineB(polylineB);
			reversePolylineB.ReverseOrder();
			reversePolylineB.vertexArray->erase(reversePolylineB.vertexArray->begin());
			this->Concatinate(polylineA, reversePolylineB);
			return true;
		}
	}
	else if ((polylineA.GetFirstVertex() - polylineB.GetFirstVertex()).Length() < eps)
	{
		Polyline reversePolylineA(polylineA);
		reversePolylineA.ReverseOrder();
		reversePolylineA.vertexArray->pop_back();
		this->Concatinate(reversePolylineA, polylineB);
		return true;
	}
	else if ((polylineA.GetLastVertex() - polylineB.GetLastVertex()).Length() < eps)
	{
		Polyline reversePolylineB(polylineB);
		reversePolylineB.ReverseOrder();
		reversePolylineB.vertexArray->erase(reversePolylineB.vertexArray->begin());
		this->Concatinate(polylineA, reversePolylineB);
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

bool Polyline::GenerateTubeMesh(ConvexPolygonMesh& tubeMesh, double radius, int numSides) const
{
	if (numSides < 2 || this->vertexArray->size() < 2)
		return false;

	std::vector<ConvexPolygon> polygonArray;
	int numVertices = this->vertexArray->size();
	for (int i = 0; i < numVertices - 1; i++)
	{
		const Vector& vertexA = (*this->vertexArray)[i];
		const Vector& vertexB = (*this->vertexArray)[i + 1];

		Vector vectorA, vectorB;

		if (i > 0)
			vectorA = (*this->vertexArray)[i - 1] - (*this->vertexArray)[i];
		else if (this->IsLineLoop())
			vectorA = (*this->vertexArray)[numVertices - 2] - (*this->vertexArray)[numVertices - 1];
		else
			vectorA = vertexA - vertexB;

		if (i < numVertices - 2)
			vectorB = (*this->vertexArray)[i + 2] - (*this->vertexArray)[i + 1];
		else if (this->IsLineLoop())
			vectorB = (*this->vertexArray)[1] - (*this->vertexArray)[0];
		else
			vectorB = vertexB - vertexA;

		vectorA.Normalize();
		vectorB.Normalize();

		Vector axisVectorA(vertexA - vertexB);
		Vector axisVectorB(vertexB - vertexA);

		axisVectorA.Normalize();
		axisVectorB.Normalize();

		Vector capNormalA(axisVectorA + vectorA);
		Vector capNormalB(axisVectorB + vectorB);

		capNormalA.Normalize();
		capNormalB.Normalize();

		Plane capPlaneA(vertexA, capNormalA);
		Plane capPlaneB(vertexB, capNormalB);

		Vector origin = (vertexA + vertexB) / 2.0;

		Vector zAxis = vertexB - vertexA;
		zAxis.Normalize();
		Vector yAxis;
		yAxis.MakeOrthogonalTo(zAxis);
		yAxis.Normalize();
		Vector xAxis = yAxis.Cross(zAxis);

		std::vector<Vector> pointArrayA, pointArrayB;
		for (int j = 0; j < numSides; j++)
		{
			double angle = (double(j) / double(numSides)) * 2.0 * M_PI;
			Vector circlePoint;
			circlePoint = origin + (xAxis * cos(angle) + yAxis * sin(angle)) * radius;

			double alpha = 0.0;
			Ray rayA(circlePoint, axisVectorA);
			rayA.CastAgainst(capPlaneA, alpha);
			pointArrayA.push_back(rayA.Lerp(alpha));

			double beta = 0.0;
			Ray rayB(circlePoint, axisVectorB);
			rayB.CastAgainst(capPlaneB, beta);
			pointArrayB.push_back(rayB.Lerp(beta));
		}

		for (int j = 0; j < numSides; j++)
		{
			int k = (j + 1) % numSides;
			ConvexPolygon polygon;
			polygon.vertexArray->push_back(pointArrayA[j]);
			polygon.vertexArray->push_back(pointArrayB[j]);
			polygon.vertexArray->push_back(pointArrayB[k]);
			polygon.vertexArray->push_back(pointArrayA[k]);
			polygonArray.push_back(polygon);
		}
	}
	
	tubeMesh.FromConvexPolygonArray(polygonArray);
	return true;
}
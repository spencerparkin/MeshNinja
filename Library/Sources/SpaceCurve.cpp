#include "SpaceCurve.h"
#include "ConvexPolygonMesh.h"

using namespace MeshNinja;

//------------------------------------ SpaceCurve ------------------------------------

SpaceCurve::SpaceCurve()
{
}

/*virtual*/ SpaceCurve::~SpaceCurve()
{
}

/*virtual*/ double SpaceCurve::AdvanceByCurveLength(double t, double length, double eps /*= MESH_NINJA_EPS*/) const
{
	Vector3 startingPoint = this->Evaluate(t);
	double dt = length / 2.0;
	while (true)
	{
		t += dt;
		Vector3 endingPoint = this->Evaluate(t);
		double currentLength = (endingPoint - startingPoint).Length();
		if (::fabs(currentLength - length) < eps)
			break;

		if (currentLength > length)
		{
			t -= dt;
			dt *= 0.5;
		}
		else if (t > 1.0)
		{
			t = 1.0;
			break;
		}
	}

	return t;
}

/*virtual*/ Vector3 SpaceCurve::EvaluateDerivative(double t) const
{
	double dt = MESH_NINJA_EPS;
	return (this->Evaluate(t + dt) - this->Evaluate(t - dt)) / (2.0 * dt);
}

/*virtual*/ Vector3 SpaceCurve::EvaluateSecondDerivative(double t) const
{
	double dt = MESH_NINJA_EPS;
	return (this->EvaluateDerivative(t + dt) - this->EvaluateDerivative(t - dt)) / (2.0 * dt);
}

double SpaceCurve::CalculateLength() const
{
	// A more accurate algorithm here would have an adaptive step size.
	double length = 0.0;
	double dt = 0.001;
	Vector3 pointA = this->Evaluate(0.0);
	for (double t = dt; t <= 1.0; t += dt)
	{
		Vector3 pointB = this->Evaluate(t);
		length += (pointB - pointA).Length();
		pointA = pointB;
	}

	return length;
}

void SpaceCurve::CalculateFrame(double t, Vector3& tangent, Vector3& normal, Vector3& binormal) const
{
	tangent = this->EvaluateDerivative(t);
	normal = this->EvaluateSecondDerivative(t);

	double angle = tangent.AngleBetweenThisAnd(normal);
	if (::fabs(angle - MESH_NINJA_PI / 2.0) >= MESH_NINJA_EPS)
		normal.MakeOrthogonalTo(tangent);	// This is a hack.  I'm not sure exactly why it happens.

	binormal = tangent.Cross(normal);
}

void SpaceCurve::GenerateTubeMesh(ConvexPolygonMesh& mesh, double stepLength, int sides, std::function<double(double)> radiusFunction) const
{
	std::vector<std::vector<Vector3>> vertexData;

	double t = 0.0;
	while (t <= 1.0)
	{
		double radius = radiusFunction(t);	// Should probably have passed in 'lengthCovered', not 't'.

		Vector3 curvePoint = this->Evaluate(t);
		Vector3 xAxis, yAxis, zAxis;
		this->CalculateFrame(t, zAxis, yAxis, xAxis);
		xAxis.Normalize();
		yAxis.Normalize();

		vertexData.push_back(std::vector<Vector3>());
		std::vector<Vector3>& vertexArray = vertexData.back();

		for (int i = 0; i < sides; i++)
		{
			double angle = -(double(i) / double(sides)) * MESH_NINJA_TWO_PI;
			Vector3 vertex = curvePoint + xAxis * radius * ::cos(angle) + yAxis * radius * ::sin(angle);
			vertexArray.push_back(vertex);
		}

		if (t == 1.0)
			break;

		t = this->AdvanceByCurveLength(t, stepLength);
	}

	std::vector<ConvexPolygon> polygonArray;

	for (int i = 0; i < (signed)vertexData.size() - 1; i++)
	{
		std::vector<Vector3>& vertexArrayA = vertexData[i];
		std::vector<Vector3>& vertexArrayB = vertexData[i + 1];

		auto calculateLength = [&vertexArrayA, &vertexArrayB, &sides](int shift) -> double
		{
			double length = 0.0;
			for (int j = 0; j < sides; j++)
			{
				length += (vertexArrayB[(j + shift) % sides] - vertexArrayA[j]).Length();
			}
			return length;
		};

		int shift = 0;
		double minimumLength = FLT_MAX;
		for (int j = 0; j < sides; j++)
		{
			double length = calculateLength(j);
			if (length < minimumLength)
			{
				minimumLength = length;
				shift = j;
			}
		}

		for (int j = 0; j < sides; j++)
		{
			ConvexPolygon polygon;

			polygon.vertexArray->push_back(vertexArrayA[j]);
			polygon.vertexArray->push_back(vertexArrayA[(j + 1) % sides]);
			polygon.vertexArray->push_back(vertexArrayB[(j + 1 + shift) % sides]);
			polygon.vertexArray->push_back(vertexArrayB[(j + shift) % sides]);

			polygonArray.push_back(polygon);
		}
	}

	mesh.FromConvexPolygonArray(polygonArray);
}

//------------------------------------ SpaceCurve::ControlPointCurve ------------------------------------

ControlPointCurve::ControlPointCurve()
{
	this->controlPointArray = new std::vector<ControlPoint>();
}

/*virtual*/ ControlPointCurve::~ControlPointCurve()
{
	delete this->controlPointArray;
}
	
//------------------------------------ SpaceCurve::CompositeBezierCurve ------------------------------------

CompositeBezierCurve::CompositeBezierCurve()
{
}

/*virtual*/ CompositeBezierCurve::~CompositeBezierCurve()
{
}

bool CompositeBezierCurve::CalculateBezierData(BezierData& data, double t) const
{
	if (this->controlPointArray->size() < 2)
		return false;

	t = MESH_NINJA_CLAMP(t, 0.0, 1.0);

	double alpha = double(this->controlPointArray->size() - 1) * t;
	double beta = ::floor(alpha);

	data.t = alpha - beta;

	int i = int(beta);
	if (i == (signed)this->controlPointArray->size() - 1)
	{
		i--;
		data.t = 1.0;
	}

	MESH_NINJA_ASSERT(0 <= i && i + 1 < (signed)this->controlPointArray->size());

	const ControlPoint& pointA = (*this)[i];
	const ControlPoint& pointB = (*this)[i + 1];

	data.pointA = pointA.point;
	data.pointB = pointA.point + pointA.tangent;
	data.pointC = pointB.point - pointB.tangent;
	data.pointD = pointB.point;

	return true;
}

/*virtual*/ Vector3 CompositeBezierCurve::Evaluate(double t) const
{
	Vector3 result(0.0, 0.0, 0.0);

	BezierData data;
	if (this->CalculateBezierData(data, t))
	{
		result += MESH_NINJA_CUBED(1.0 - data.t) * data.pointA;
		result += 3.0 * MESH_NINJA_SQUARED(1.0 - data.t) * data.t * data.pointB;
		result += 3.0 * (1.0 - data.t) * MESH_NINJA_SQUARED(data.t) * data.pointC;
		result += MESH_NINJA_CUBED(data.t) * data.pointD;
	}

	return result;
}

/*virtual*/ Vector3 CompositeBezierCurve::EvaluateDerivative(double t) const
{
	Vector3 result(0.0, 0.0, 0.0);

	BezierData data;
	if (this->CalculateBezierData(data, t))
	{
		result += -3.0 * MESH_NINJA_SQUARED(data.t - 1.0) * data.pointA;
		result += (3.0 - 12.0 * data.t + 9.0 * MESH_NINJA_SQUARED(data.t)) * data.pointB;
		result += (6.0 * data.t - 9.0 * MESH_NINJA_SQUARED(data.t)) * data.pointC;
		result += 3.0 * MESH_NINJA_SQUARED(data.t) * data.pointD;
	}

	return result;
}

/*virtual*/ Vector3 CompositeBezierCurve::EvaluateSecondDerivative(double t) const
{
	Vector3 result(0.0, 0.0, 0.0);

	BezierData data;
	if (this->CalculateBezierData(data, t))
	{
		result += 6.0 * (1.0 - data.t) * data.pointA;
		result += (-12.0 + 18.0 * data.t) * data.pointB;
		result += (6.0 - 18.0 * data.t) * data.pointC;
		result += 6.0 * data.t * data.pointD;
	}

	return result;
}
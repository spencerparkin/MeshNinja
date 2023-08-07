#pragma once

#include "Math/Vector.h"

namespace MeshNinja
{
	class ConvexPolygonMesh;

	// These are always parameterized from 0 to 1.
	class MESH_NINJA_API SpaceCurve
	{
	public:
		SpaceCurve();
		virtual ~SpaceCurve();

		virtual Vector Evaluate(double t) const = 0;
		virtual Vector EvaluateDerivative(double t) const;
		virtual Vector EvaluateSecondDerivative(double t) const;
		virtual double AdvanceByCurveLength(double t, double length, double eps = MESH_NINJA_EPS) const;

		double CalculateLength() const;
		void CalculateFrame(double t, Vector& tangent, Vector& normal, Vector& binormal) const;
		void GenerateTubeMesh(ConvexPolygonMesh& mesh, double stepLength, int sides, std::function<double(double)> radiusFunction) const;
	};

	class MESH_NINJA_API ControlPointCurve : public SpaceCurve
	{
	public:
		ControlPointCurve();
		virtual ~ControlPointCurve();

		struct ControlPoint
		{
			Vector point;
			Vector tangent;
		};

		const ControlPoint& operator[](int i) const
		{
			return (*this->controlPointArray)[i];
		}

		std::vector<ControlPoint>* controlPointArray;
	};

	class MESH_NINJA_API CompositeBezierCurve : public ControlPointCurve
	{
	public:
		CompositeBezierCurve();
		virtual ~CompositeBezierCurve();

		virtual Vector Evaluate(double t) const override;
		virtual Vector EvaluateDerivative(double t) const override;
		virtual Vector EvaluateSecondDerivative(double t) const override;

		struct BezierData
		{
			Vector pointA, pointB, pointC, pointD;
			double t;
		};

		bool CalculateBezierData(BezierData& data, double t) const;
	};
}
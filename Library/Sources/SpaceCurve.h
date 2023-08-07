#pragma once

#include "Math/Vector3.h"

namespace MeshNinja
{
	class ConvexPolygonMesh;

	// These are always parameterized from 0 to 1.
	class MESH_NINJA_API SpaceCurve
	{
	public:
		SpaceCurve();
		virtual ~SpaceCurve();

		virtual Vector3 Evaluate(double t) const = 0;
		virtual Vector3 EvaluateDerivative(double t) const;
		virtual Vector3 EvaluateSecondDerivative(double t) const;
		virtual double AdvanceByCurveLength(double t, double length, double eps = MESH_NINJA_EPS) const;

		double CalculateLength() const;
		void CalculateFrame(double t, Vector3& tangent, Vector3& normal, Vector3& binormal) const;
		void GenerateTubeMesh(ConvexPolygonMesh& mesh, double stepLength, int sides, std::function<double(double)> radiusFunction) const;
	};

	class MESH_NINJA_API ControlPointCurve : public SpaceCurve
	{
	public:
		ControlPointCurve();
		virtual ~ControlPointCurve();

		struct ControlPoint
		{
			Vector3 point;
			Vector3 tangent;
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

		virtual Vector3 Evaluate(double t) const override;
		virtual Vector3 EvaluateDerivative(double t) const override;
		virtual Vector3 EvaluateSecondDerivative(double t) const override;

		struct BezierData
		{
			Vector3 pointA, pointB, pointC, pointD;
			double t;
		};

		bool CalculateBezierData(BezierData& data, double t) const;
	};
}
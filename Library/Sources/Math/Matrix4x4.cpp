#include "Matrix4x4.h"
#include "Matrix3x3.h"
#include "Vector.h"
#include "Quaternion.h"
#include <math.h>

using namespace MeshNinja;

Matrix4x4::Matrix4x4()
{
	this->Identity();
}

Matrix4x4::Matrix4x4(const Matrix4x4& matrix)
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			this->ele[i][j] = matrix.ele[i][j];
}

Matrix4x4::Matrix4x4(const Matrix3x3& matrix)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (i < 3 && j < 3)
				this->ele[i][j] = matrix.ele[i][j];
			else
				this->ele[i][j] = (i == j) ? 1.0 : 0.0;
		}
	}
}

Matrix4x4::Matrix4x4(const Matrix3x3& matrix, const Vector& translation)
{
	this->ele[0][0] = matrix.ele[0][0];
	this->ele[1][0] = matrix.ele[1][0];
	this->ele[2][0] = matrix.ele[2][0];

	this->ele[0][1] = matrix.ele[0][1];
	this->ele[1][1] = matrix.ele[1][1];
	this->ele[2][1] = matrix.ele[2][1];

	this->ele[0][2] = matrix.ele[0][2];
	this->ele[1][2] = matrix.ele[1][2];
	this->ele[2][2] = matrix.ele[2][2];

	this->ele[0][3] = translation.x;
	this->ele[1][3] = translation.y;
	this->ele[2][3] = translation.z;

	this->ele[3][0] = 0.0;
	this->ele[3][1] = 0.0;
	this->ele[3][2] = 0.0;

	this->ele[3][3] = 1.0;
}

/*virtual*/ Matrix4x4::~Matrix4x4()
{
}

void Matrix4x4::operator=(const Matrix4x4& matrix)
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			this->ele[i][j] = matrix.ele[i][j];
}

void Matrix4x4::operator=(const Matrix3x3& matrix)
{
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			this->ele[i][j] = matrix.ele[i][j];
}

void Matrix4x4::Identity()
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			this->ele[i][j] = (i == j) ? 1.0 : 0.0;
}

bool Matrix4x4::SetCol(int col, const Vector& vector)
{
	if (col < 0 || col > 3)
		return false;

	this->ele[0][col] = vector.x;
	this->ele[1][col] = vector.y;
	this->ele[2][col] = vector.z;
	return true;
}

bool Matrix4x4::GetCol(int col, Vector& vector) const
{
	if (col < 0 || col > 3)
		return false;

	vector.x = this->ele[0][col];
	vector.y = this->ele[1][col];
	vector.z = this->ele[2][col];
	return true;
}

void Matrix4x4::GetAxes(Vector& xAxis, Vector& yAxis, Vector& zAxis) const
{
	this->GetCol(0, xAxis);
	this->GetCol(1, yAxis);
	this->GetCol(2, zAxis);
}

void Matrix4x4::SetAxes(const Vector& xAxis, const Vector& yAxis, const Vector& zAxis)
{
	this->SetCol(0, xAxis);
	this->SetCol(1, yAxis);
	this->SetCol(2, zAxis);
}

void Matrix4x4::TransformVector(const Vector& vector, Vector& vectorTransformed) const
{
	vectorTransformed.SetComponents(
		vector.x * this->ele[0][0] +
		vector.y * this->ele[0][1] +
		vector.z * this->ele[0][2],
		vector.x * this->ele[1][0] +
		vector.y * this->ele[1][1] +
		vector.z * this->ele[1][2],
		vector.x * this->ele[2][0] +
		vector.y * this->ele[2][1] +
		vector.z * this->ele[2][2]);
}

void Matrix4x4::TransformPoint(const Vector& point, Vector& pointTransformed) const
{
	pointTransformed.SetComponents(
		point.x * this->ele[0][0] +
		point.y * this->ele[0][1] +
		point.z * this->ele[0][2] +
		this->ele[0][3],
		point.x * this->ele[1][0] +
		point.y * this->ele[1][1] +
		point.z * this->ele[1][2] +
		this->ele[1][3],
		point.x * this->ele[2][0] +
		point.y * this->ele[2][1] +
		point.z * this->ele[2][2] +
		this->ele[2][3]);

	double w = 
		point.x * this->ele[3][0] +
		point.y * this->ele[3][1] +
		point.z * this->ele[3][2] +
		this->ele[3][3];

	if (w != 1.0)
	{
		pointTransformed.x /= w;
		pointTransformed.y /= w;
		pointTransformed.z /= w;
	}
}

/*
void Matrix4x4::TransformVector(const Vector4& vector, Vector4& vectorTransformed) const
{
	vectorTransformed.SetComponents(
		this->ele[0][0] * vector.x +
		this->ele[0][1] * vector.y +
		this->ele[0][2] * vector.z +
		this->ele[0][3] * vector.w,
		this->ele[1][0] * vector.x +
		this->ele[1][1] * vector.y +
		this->ele[1][2] * vector.z +
		this->ele[1][3] * vector.w,
		this->ele[2][0] * vector.x +
		this->ele[2][1] * vector.y +
		this->ele[2][2] * vector.z +
		this->ele[2][3] * vector.w,
		this->ele[3][0] * vector.x +
		this->ele[3][1] * vector.y +
		this->ele[3][2] * vector.z +
		this->ele[3][3] * vector.w);
}
*/

void Matrix4x4::Multiply(const Matrix4x4& leftMatrix, const Matrix4x4& rightMatrix)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			this->ele[i][j] =
				leftMatrix.ele[i][0] * rightMatrix.ele[0][j] +
				leftMatrix.ele[i][1] * rightMatrix.ele[1][j] +
				leftMatrix.ele[i][2] * rightMatrix.ele[2][j] +
				leftMatrix.ele[i][3] * rightMatrix.ele[3][j];
		}
	}
}

bool Matrix4x4::Divide(const Matrix4x4& leftMatrix, const Matrix4x4& rightMatrix)
{
	Matrix4x4 rightMatrixInverted;
	if (!rightMatrixInverted.Invert(rightMatrix))
		return false;

	this->Multiply(leftMatrix, rightMatrixInverted);
	return true;
}

bool Matrix4x4::Invert(const Matrix4x4& matrix)
{
	double det = matrix.Determinant();
	if (det == 0.0)
		return false;

	this->ele[0][0] = (matrix.ele[1][1] * (matrix.ele[2][2] * matrix.ele[3][3] - matrix.ele[3][2] * matrix.ele[2][3]) - matrix.ele[1][2] * (matrix.ele[2][1] * matrix.ele[3][3] - matrix.ele[3][1] * matrix.ele[2][3]) + matrix.ele[1][3] * (matrix.ele[2][1] * matrix.ele[3][2] - matrix.ele[3][1] * matrix.ele[2][2])) / det;
	this->ele[0][1] = -(matrix.ele[0][1] * (matrix.ele[2][2] * matrix.ele[3][3] - matrix.ele[3][2] * matrix.ele[2][3]) - matrix.ele[0][2] * (matrix.ele[2][1] * matrix.ele[3][3] - matrix.ele[3][1] * matrix.ele[2][3]) + matrix.ele[0][3] * (matrix.ele[2][1] * matrix.ele[3][2] - matrix.ele[3][1] * matrix.ele[2][2])) / det;
	this->ele[0][2] = (matrix.ele[0][1] * (matrix.ele[1][2] * matrix.ele[3][3] - matrix.ele[3][2] * matrix.ele[1][3]) - matrix.ele[0][2] * (matrix.ele[1][1] * matrix.ele[3][3] - matrix.ele[3][1] * matrix.ele[1][3]) + matrix.ele[0][3] * (matrix.ele[1][1] * matrix.ele[3][2] - matrix.ele[3][1] * matrix.ele[1][2])) / det;
	this->ele[0][3] = -(matrix.ele[0][1] * (matrix.ele[1][2] * matrix.ele[2][3] - matrix.ele[2][2] * matrix.ele[1][3]) - matrix.ele[0][2] * (matrix.ele[1][1] * matrix.ele[2][3] - matrix.ele[2][1] * matrix.ele[1][3]) + matrix.ele[0][3] * (matrix.ele[1][1] * matrix.ele[2][2] - matrix.ele[2][1] * matrix.ele[1][2])) / det;
	this->ele[1][0] = -(matrix.ele[1][0] * (matrix.ele[2][2] * matrix.ele[3][3] - matrix.ele[3][2] * matrix.ele[2][3]) - matrix.ele[1][2] * (matrix.ele[2][0] * matrix.ele[3][3] - matrix.ele[3][0] * matrix.ele[2][3]) + matrix.ele[1][3] * (matrix.ele[2][0] * matrix.ele[3][2] - matrix.ele[3][0] * matrix.ele[2][2])) / det;
	this->ele[1][1] = (matrix.ele[0][0] * (matrix.ele[2][2] * matrix.ele[3][3] - matrix.ele[3][2] * matrix.ele[2][3]) - matrix.ele[0][2] * (matrix.ele[2][0] * matrix.ele[3][3] - matrix.ele[3][0] * matrix.ele[2][3]) + matrix.ele[0][3] * (matrix.ele[2][0] * matrix.ele[3][2] - matrix.ele[3][0] * matrix.ele[2][2])) / det;
	this->ele[1][2] = -(matrix.ele[0][0] * (matrix.ele[1][2] * matrix.ele[3][3] - matrix.ele[3][2] * matrix.ele[1][3]) - matrix.ele[0][2] * (matrix.ele[1][0] * matrix.ele[3][3] - matrix.ele[3][0] * matrix.ele[1][3]) + matrix.ele[0][3] * (matrix.ele[1][0] * matrix.ele[3][2] - matrix.ele[3][0] * matrix.ele[1][2])) / det;
	this->ele[1][3] = (matrix.ele[0][0] * (matrix.ele[1][2] * matrix.ele[2][3] - matrix.ele[2][2] * matrix.ele[1][3]) - matrix.ele[0][2] * (matrix.ele[1][0] * matrix.ele[2][3] - matrix.ele[2][0] * matrix.ele[1][3]) + matrix.ele[0][3] * (matrix.ele[1][0] * matrix.ele[2][2] - matrix.ele[2][0] * matrix.ele[1][2])) / det;
	this->ele[2][0] = (matrix.ele[1][0] * (matrix.ele[2][1] * matrix.ele[3][3] - matrix.ele[3][1] * matrix.ele[2][3]) - matrix.ele[1][1] * (matrix.ele[2][0] * matrix.ele[3][3] - matrix.ele[3][0] * matrix.ele[2][3]) + matrix.ele[1][3] * (matrix.ele[2][0] * matrix.ele[3][1] - matrix.ele[3][0] * matrix.ele[2][1])) / det;
	this->ele[2][1] = -(matrix.ele[0][0] * (matrix.ele[2][1] * matrix.ele[3][3] - matrix.ele[3][1] * matrix.ele[2][3]) - matrix.ele[0][1] * (matrix.ele[2][0] * matrix.ele[3][3] - matrix.ele[3][0] * matrix.ele[2][3]) + matrix.ele[0][3] * (matrix.ele[2][0] * matrix.ele[3][1] - matrix.ele[3][0] * matrix.ele[2][1])) / det;
	this->ele[2][2] = (matrix.ele[0][0] * (matrix.ele[1][1] * matrix.ele[3][3] - matrix.ele[3][1] * matrix.ele[1][3]) - matrix.ele[0][1] * (matrix.ele[1][0] * matrix.ele[3][3] - matrix.ele[3][0] * matrix.ele[1][3]) + matrix.ele[0][3] * (matrix.ele[1][0] * matrix.ele[3][1] - matrix.ele[3][0] * matrix.ele[1][1])) / det;
	this->ele[2][3] = -(matrix.ele[0][0] * (matrix.ele[1][1] * matrix.ele[2][3] - matrix.ele[2][1] * matrix.ele[1][3]) - matrix.ele[0][1] * (matrix.ele[1][0] * matrix.ele[2][3] - matrix.ele[2][0] * matrix.ele[1][3]) + matrix.ele[0][3] * (matrix.ele[1][0] * matrix.ele[2][1] - matrix.ele[2][0] * matrix.ele[1][1])) / det;
	this->ele[3][0] = -(matrix.ele[1][0] * (matrix.ele[2][1] * matrix.ele[3][2] - matrix.ele[3][1] * matrix.ele[2][2]) - matrix.ele[1][1] * (matrix.ele[2][0] * matrix.ele[3][2] - matrix.ele[3][0] * matrix.ele[2][2]) + matrix.ele[1][2] * (matrix.ele[2][0] * matrix.ele[3][1] - matrix.ele[3][0] * matrix.ele[2][1])) / det;
	this->ele[3][1] = (matrix.ele[0][0] * (matrix.ele[2][1] * matrix.ele[3][2] - matrix.ele[3][1] * matrix.ele[2][2]) - matrix.ele[0][1] * (matrix.ele[2][0] * matrix.ele[3][2] - matrix.ele[3][0] * matrix.ele[2][2]) + matrix.ele[0][2] * (matrix.ele[2][0] * matrix.ele[3][1] - matrix.ele[3][0] * matrix.ele[2][1])) / det;
	this->ele[3][2] = -(matrix.ele[0][0] * (matrix.ele[1][1] * matrix.ele[3][2] - matrix.ele[3][1] * matrix.ele[1][2]) - matrix.ele[0][1] * (matrix.ele[1][0] * matrix.ele[3][2] - matrix.ele[3][0] * matrix.ele[1][2]) + matrix.ele[0][2] * (matrix.ele[1][0] * matrix.ele[3][1] - matrix.ele[3][0] * matrix.ele[1][1])) / det;
	this->ele[3][3] = (matrix.ele[0][0] * (matrix.ele[1][1] * matrix.ele[2][2] - matrix.ele[2][1] * matrix.ele[1][2]) - matrix.ele[0][1] * (matrix.ele[1][0] * matrix.ele[2][2] - matrix.ele[2][0] * matrix.ele[1][2]) + matrix.ele[0][2] * (matrix.ele[1][0] * matrix.ele[2][1] - matrix.ele[2][0] * matrix.ele[1][1])) / det;

	return true;
}

void Matrix4x4::Transpose(const Matrix4x4& matrix)
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			this->ele[i][j] = matrix.ele[j][i];
}

double Matrix4x4::Determinant() const
{
	double det =
		this->ele[0][0] * (
			this->ele[1][1] * (
				this->ele[2][2] * this->ele[3][3] - this->ele[3][2] * this->ele[2][3]
			) - this->ele[1][2] * (
				this->ele[2][1] * this->ele[3][3] - this->ele[3][1] * this->ele[2][3]
			) + this->ele[1][3] * (
				this->ele[2][1] * this->ele[3][2] - this->ele[3][1] * this->ele[2][2]
			)
		) - this->ele[0][1] * (
			this->ele[1][0] * (
				this->ele[2][2] * this->ele[3][3] - this->ele[3][2] * this->ele[2][3]
			) - this->ele[1][2] * (
				this->ele[2][0] * this->ele[3][3] - this->ele[3][0] * this->ele[2][3]
			) + this->ele[1][3] * (
				this->ele[2][0] * this->ele[3][2] - this->ele[3][0] * this->ele[2][2]
			)
		) + this->ele[0][2] * (
			this->ele[1][0] * (
				this->ele[2][1] * this->ele[3][3] - this->ele[3][1] * this->ele[2][3]
			) - this->ele[1][1] * (
				this->ele[2][0] * this->ele[3][3] - this->ele[3][0] * this->ele[2][3]
			) + this->ele[1][3] * (
				this->ele[2][0] * this->ele[3][1] - this->ele[3][0] * this->ele[2][1]
			)
		) - this->ele[0][3] * (
			this->ele[1][0] * (
				this->ele[2][1] * this->ele[3][2] - this->ele[3][1] * this->ele[2][2]
			) - this->ele[1][1] * (
				this->ele[2][0] * this->ele[3][2] - this->ele[3][0] * this->ele[2][2]
			) + this->ele[1][2] * (
				this->ele[2][0] * this->ele[3][1] - this->ele[3][0] * this->ele[2][1]
			)
		);

	return det;
}

void Matrix4x4::SetTranslation(const Vector& translation)
{
	this->ele[0][3] = translation.x;
	this->ele[1][3] = translation.y;
	this->ele[2][3] = translation.z;
}

void Matrix4x4::SetScale(const Vector& scale)
{
	this->SetCol(0, Vector(scale.x, 0.0, 0.0));
	this->SetCol(1, Vector(0.0, scale.y, 0.0));
	this->SetCol(2, Vector(0.0, 0.0, scale.z));
}

void Matrix4x4::SetUniformScale(double scale)
{
	this->SetCol(0, Vector(scale, 0.0, 0.0));
	this->SetCol(1, Vector(0.0, scale, 0.0));
	this->SetCol(2, Vector(0.0, 0.0, scale));
}

void Matrix4x4::SetRotation(const Vector& axis, double angle)
{
	Vector xAxis(1.0, 0.0, 0.0);
	Vector yAxis(0.0, 1.0, 0.0);
	Vector zAxis(0.0, 0.0, 1.0);

	xAxis.RotateAbout(axis, angle);
	yAxis.RotateAbout(axis, angle);
	zAxis.RotateAbout(axis, angle);

	this->SetCol(0, xAxis);
	this->SetCol(1, yAxis);
	this->SetCol(2, zAxis);
}

void Matrix4x4::RigidBodyMotion(const Vector& axis, double angle, const Vector& delta)
{
	this->Identity();
	this->SetRotation(axis, angle);
	this->SetTranslation(delta);
}

void Matrix4x4::RigidBodyMotion(const Quaternion& quat, const Vector& delta)
{
	Vector axis;
	double angle;
	quat.GetToAxisAngle(axis, angle);
	this->RigidBodyMotion(axis, angle, delta);
}

void Matrix4x4::Projection(double hfovi, double vfovi, double near, double far)
{
	this->Identity();
	this->ele[0][0] = 1.0 / tan(hfovi / 2.0);
	this->ele[1][1] = 1.0 / tan(vfovi / 2.0);
	this->ele[2][2] = -far / (far - near);
	this->ele[3][3] = 0.0;
	this->ele[2][3] = far * near / (far - near);
	this->ele[3][2] = -1.0;
}

bool Matrix4x4::OrthonormalizeOrientation()
{
	Vector xAxis, yAxis, zAxis;
	this->GetAxes(xAxis, yAxis, zAxis);

	if (!xAxis.Normalize())
		return false;

	if (!yAxis.RejectFrom(xAxis))
		return false;

	if (!yAxis.Normalize())
		return false;

	zAxis.Cross(xAxis, yAxis);
	zAxis.Normalize();

	this->SetAxes(xAxis, yAxis, zAxis);
	return true;
}

namespace Frumpy
{
	Matrix4x4 operator*(const Matrix4x4& leftMatrix, const Matrix4x4& rightMatrix)
	{
		Matrix4x4 product;
		product.Multiply(leftMatrix, rightMatrix);
		return product;
	}
}
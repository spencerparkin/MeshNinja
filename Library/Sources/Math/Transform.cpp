#include "Transform.h"
#include "Matrix4x4.h"
#include "../ConvexPolygon.h"

using namespace MeshNinja;

Transform::Transform()
{
	this->SetIdentity();
}

/*virtual*/ Transform::~Transform()
{
}

void Transform::SetIdentity()
{
	this->matrix.SetIdentity();
	this->translation = Vector3(0.0, 0.0, 0.0);
}

bool Transform::FromMatrix4x4(const Matrix4x4& matrix)
{
	if (matrix.ele[3][0] != 0.0 || matrix.ele[3][1] != 0.0 || matrix.ele[3][2] != 0.0 || matrix.ele[3][3] != 1.0)
		return false;

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			this->matrix.ele[i][j] = matrix.ele[i][j];

	this->translation.SetComponents(
		matrix.ele[0][3],
		matrix.ele[1][3],
		matrix.ele[2][3]);

	return true;
}

void Transform::ToMatrix4x4(Matrix4x4& matrix) const
{
	matrix.Identity();

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			matrix.ele[i][j] = this->matrix.ele[i][j];

	matrix.ele[0][3] = this->translation.x;
	matrix.ele[1][3] = this->translation.y;
	matrix.ele[2][3] = this->translation.z;
}

Vector3 Transform::TransformVector(const Vector3& vector) const
{
	Vector3 result;
	this->matrix.MultiplyRight(vector, result);
	return result;
}

Vector3 Transform::TransformPosition(const Vector3& position) const
{
	Vector3 result;
	this->matrix.MultiplyRight(position, result);
	result += this->translation;
	return result;
}

void Transform::TransformPolygon(ConvexPolygon& polygon) const
{
	for (Vector3& vertex : *polygon.vertexArray)
		vertex = this->TransformPosition(vertex);
}

bool Transform::SetInverse(const Transform& transform)
{
	if (!transform.matrix.GetInverse(this->matrix))
		return false;

	this->matrix.MultiplyRight(transform.translation, this->translation);
	this->translation *= -1.0;
	return true;
}

bool Transform::GetInverse(Transform& transform) const
{
	return transform.SetInverse(*this);
}

Transform& Transform::Multiply(const Transform& transformA, const Transform& transformB)
{
	this->matrix.SetProduct(transformA.matrix, transformB.matrix);
	transformA.matrix.MultiplyRight(transformB.translation, this->translation);
	this->translation += transformA.translation; 
	return *this;
}

Vector3 Transform::operator*(const Vector3& vector)
{
	return this->TransformVector(vector);
}

namespace MeshNinja
{
	Transform operator*(const Transform& transformA, const Transform& transformB)
	{
		Transform product;
		product.Multiply(transformA, transformB);
		return product;
	}
}
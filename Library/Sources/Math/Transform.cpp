#include "Transform.h"
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
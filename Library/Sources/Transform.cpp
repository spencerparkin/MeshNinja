#include "Transform.h"
#include "ConvexPolygon.h"

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
	this->translation = Vector(0.0, 0.0, 0.0);
}

Vector Transform::TransformVector(const Vector& vector) const
{
	Vector result;
	this->matrix.MultiplyRight(vector, result);
	return result;
}

Vector Transform::TransformPosition(const Vector& position) const
{
	Vector result;
	this->matrix.MultiplyRight(position, result);
	result += this->translation;
	return result;
}

void Transform::TransformPolygon(ConvexPolygon& polygon) const
{
	for (Vector& vertex : *polygon.vertexArray)
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

Vector Transform::operator*(const Vector& vector)
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
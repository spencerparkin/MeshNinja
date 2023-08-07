#pragma once

#include "Math/Vector3.h"

namespace MeshNinja
{
	class ConvexPolygonMesh;
	class Ray;
	class AxisAlignedBoundingBox;
	class AlgebraicSurface;

	class MESH_NINJA_API MeshFitter
	{
	public:
		MeshFitter();
		virtual ~MeshFitter();

		// We should be able to fit a mesh to an object that inherits and impliments this interface.
		class MESH_NINJA_API FittableObject
		{
		public:
			FittableObject();
			virtual ~FittableObject();

			virtual Ray CalcInitialContactRay() const = 0;
			virtual Vector3 CalculateSurfaceNormalAt(const Vector3& surfacePoint) const = 0;
			virtual bool RayCast(const Ray& ray, double& alpha, double eps = MESH_NINJA_EPS) const = 0;
		};

		// Fitting a mesh to an algebraic surface is useful for being able to visualize/render the surface.
		class MESH_NINJA_API FittableAlgebraicSurface : public FittableObject
		{
		public:
			FittableAlgebraicSurface(const AlgebraicSurface* surface);
			virtual ~FittableAlgebraicSurface();

			virtual bool RayCast(const Ray& ray, double& alpha, double eps = MESH_NINJA_EPS) const override;
			virtual Vector3 CalculateSurfaceNormalAt(const Vector3& surfacePoint) const override;
			virtual Ray CalcInitialContactRay() const override;

			const AlgebraicSurface* surface;
		};

		// Fitting a mesh to a mesh is one way to down-res the mesh.  Up-ressing would probably be better
		// done using polygon subdivision, but down-ressing the mesh is a more difficult problem.
		class MESH_NINJA_API FittableMesh : public FittableObject
		{
		public:
			FittableMesh(const ConvexPolygonMesh* mesh);
			virtual ~FittableMesh();

			virtual bool RayCast(const Ray& ray, double& alpha, double eps = MESH_NINJA_EPS) const override;
			virtual Vector3 CalculateSurfaceNormalAt(const Vector3& surfacePoint) const override;
			virtual Ray CalcInitialContactRay() const override;

			const ConvexPolygonMesh* mesh;
		};

		bool GenerateMesh(ConvexPolygonMesh& mesh, const AxisAlignedBoundingBox& boundingBox, double approximateEdgeLength, const FittableObject& object);
	};
}
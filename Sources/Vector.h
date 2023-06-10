#pragma once

#include "Common.h"

namespace MeshNinja
{
	class MESH_NINJA_API Vector
	{
	public:
		Vector();
		virtual ~Vector();

		double x, y, z;
	};
}
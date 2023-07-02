#pragma once

#if defined MESH_NINJA_IMPORT
#	define MESH_NINJA_API			__declspec(dllimport)
#elif defined MESH_NINJA_EXPORT
#	define MESH_NINJA_API			__declspec(dllexport)
#else
#	define MESH_NINJA_API
#endif

#define MESH_NINJA_EPS				1e-3 //1e-4
#define MESH_NINJA_MIN(a, b)		((a) < (b) ? (a) : (b))
#define MESH_NINJA_MAX(a, b)		((a) > (b) ? (a) : (b))
#define MESH_NINJA_SIGN(a)			((a) < 0.0 ? -1.0 : 1.0)

#include <vector>
#include <list>
#include <map>
#include <set>
#include <string>
#include <format>
#include <fstream>
#include <sstream>
#include <math.h>
#include <assert.h>
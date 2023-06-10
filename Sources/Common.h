#pragma once

#if defined MESH_NINJA_IMPORT
#	define MESH_NINJA_API			__declspec(dllimport)
#elif defined MESH_NINJA_EXPORT
#	define MESH_NINJA_API			__declspec(dllexport)
#else
#	define MESH_NINJA_API
#endif

#include <vector>
#include <list>
#include <map>
#include <set>
#include <string>
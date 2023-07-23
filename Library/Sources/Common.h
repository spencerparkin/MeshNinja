#pragma once

#if defined MESH_NINJA_IMPORT
#	define MESH_NINJA_API			__declspec(dllimport)
#elif defined MESH_NINJA_EXPORT
#	define MESH_NINJA_API			__declspec(dllexport)
#else
#	define MESH_NINJA_API
#endif

#define MESH_NINJA_EPS				1e-4	// Tighter?
#define MESH_NINJA_PHI				1.618033988749
#define MESH_NINJA_PI				3.1415926536
#define MESH_NINJA_TWO_PI			(2.0 * MESH_NINJA_PI)
#define MESH_NINJA_MIN(a, b)		((a) < (b) ? (a) : (b))
#define MESH_NINJA_MAX(a, b)		((a) > (b) ? (a) : (b))
#define MESH_NINJA_SIGN(a)			((a) < 0.0 ? -1.0 : 1.0)
#define MESH_NINJA_CLAMP(x, a, b)	MESH_NINJA_MIN(MESH_NINJA_MAX(x, a), b)
#define MESH_NINJA_SQUARED(x)		((x) * (x))
#define MESH_NINJA_CUBED(x)			((x) * (x) * (x))
#define MESH_NINJA_SWAP(a, b) \
									do { \
										a ^= b; \
										b ^= a; \
										a ^= b; \
									} while(0)

#if defined MESH_NINJA_DEBUG
#	define MESH_NINJA_ASSERT(cond)		do { \
											assert(cond); \
										} while(0)
#else
#	define MESH_NINJA_ASSERT(cond)
#endif

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
#include <functional>

// TODO: Make PCH?
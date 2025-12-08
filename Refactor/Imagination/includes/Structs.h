#pragma once

#define GATEWARE_ENABLE_MATH //enable all math libraries
#include "Gateware/Gateware.h"
using GMatrix = GW::MATH::GMatrix;
using mat4 = GW::MATH::GMATRIXF;
using vec4 = GW::MATH::GVECTORF;
using GVector2D = GW::MATH2D::GVector2D;
using mat3 = GW::MATH2D::GMATRIX3F;
using vec3 = GW::MATH2D::GVECTOR3F;
using vec2 = GW::MATH2D::GVECTOR2F;

struct SceneData
{
	mat4 view, proj;
};

struct ModelData
{
	mat4 world;
};
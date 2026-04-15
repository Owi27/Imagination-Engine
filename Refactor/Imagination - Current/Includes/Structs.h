#pragma once

struct GBufferUBO
{
	mat4 world = GW::MATH::GIdentityMatrixF, view, proj;
	float deltaTime;
};

struct LightingUBO
{

};

struct Material
{

};
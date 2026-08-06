#include "pch.hpp"
#include "ImgnMath.h"

namespace Imgn
{
	float Math::Dot(vec3 pLVec, vec3 pRVec)
	{
		return (pLVec[0] * pRVec[0]) + (pLVec[1] * pRVec[1]) + (pLVec[2] * pRVec[2]);
	}

	float Math::Length(vec3 pVec)
	{
		return std::sqrt(std::pow(pVec[0], 2) + std::pow(pVec[1], 2) + std::pow(pVec[2], 2));
	}

	vec3 Math::Normalize(vec3 pVec)
	{
		float length = Length(pVec);

		return vec3
		{
			pVec[0] / length,
			pVec[1] / length,
			pVec[2] / length
		};
	}

	mat4 Math::LookAtLH(vec3 pEye, vec3 pAt, vec3 pUp)
	{
		vec3 forward = Normalize(pAt - pEye), right = Normalize(pUp * forward), up = forward * right;

		return mat4
		{
			right[0], up[0], forward[0], 0.f,
			right[1], up[1], forward[1], 0.f,
			right[2], up[2], forward[2], 0.f,
			-Dot(right, pEye), -Dot(up, pEye), -Dot(forward, pEye), 1.f,
		};
	}

	mat4 Math::PerspectiveVKLH(float pFOV, float pAspect, float pNear, float pFar)
	{
		float yScale = 1.f / std::tanf(pFOV * .5f);
		float z = pFar / (pFar - pNear);

		return mat4
		{
			yScale / pAspect, 0.f, 0.f, 0.f,
			0.f, -yScale, 0.f, 0.f,
			0.f, 0.f, z, 1.f,
			0.f, 0.f, -pNear * z, 0.f,
		};
	}
}
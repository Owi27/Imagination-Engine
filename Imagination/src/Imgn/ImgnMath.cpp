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

	mat4 Math::Inverse(mat4 pMat)
	{
		float a0 = pMat[0] * pMat[5] - pMat[1] * pMat[4];
		float a1 = pMat[0] * pMat[6] - pMat[2] * pMat[4];
		float a2 = pMat[0] * pMat[7] - pMat[3] * pMat[4];
		float a3 = pMat[1] * pMat[6] - pMat[2] * pMat[5];
		float a4 = pMat[1] * pMat[7] - pMat[3] * pMat[5];
		float a5 = pMat[2] * pMat[7] - pMat[3] * pMat[6];
		float b0 = pMat[8] * pMat[13] - pMat[9] * pMat[12];
		float b1 = pMat[8] * pMat[14] - pMat[10] * pMat[12];
		float b2 = pMat[8] * pMat[15] - pMat[11] * pMat[12];
		float b3 = pMat[9] * pMat[14] - pMat[10] * pMat[13];
		float b4 = pMat[9] * pMat[15] - pMat[11] * pMat[13];
		float b5 = pMat[10] * pMat[15] - pMat[11] * pMat[14];
		float det = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;

		return mat4
		{
			pMat[5] * b5 - pMat[6] * b4 + pMat[7] * b3, -pMat[1] * b5 + pMat[2] * b4 - pMat[3] * b3, pMat[13] * a5 - pMat[14] * a4 + pMat[15] * a3, -pMat[9] * a5 + pMat[10] * a4 - pMat[11] * a3,
			-pMat[4] * b5 + pMat[6] * b2 - pMat[7] * b1, pMat[0] * b5 - pMat[2] * b2 + pMat[3] * b1, -pMat[12] * a5 + pMat[14] * a2 - pMat[15] * a1, pMat[8] * a5 - pMat[10] * a2 + pMat[11] * a1,
			pMat[4] * b4 - pMat[5] * b2 + pMat[7] * b0, -pMat[0] * b4 + pMat[1] * b2 - pMat[3] * b0, pMat[12] * a4 - pMat[13] * a2 + pMat[15] * a0, -pMat[8] * a4 + pMat[9] * a2 - pMat[11] * a0,
			-pMat[4] * b3 + pMat[5] * b1 - pMat[6] * b0, pMat[0] * b3 - pMat[1] * b1 + pMat[2] * b0, -pMat[12] * a3 + pMat[13] * a1 - pMat[14] * a0, pMat[8] * a3 - pMat[9] * a1 + pMat[10] * a0,
		} * (1.f / det);
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

	mat4 Math::Rotate(mat4 pMat, vec3 pAxis, float pRadian, bool pGlobal)
	{
		float c = cos(pRadian);
		float s = sin(pRadian);
		mat4 rotation = identity;

		if (pAxis[0] > 0.f) //x
		{
			rotation[5] = c;
			rotation[6] = s;
			rotation[9] = -s;
			rotation[10] = c;

			if (pGlobal)
			{
				mat4 out;
				vec4 translation = { pMat[12], pMat[13], pMat[14], pMat[15] };
				out = pMat * rotation;

				for (size_t i = 0; i < 4; i++) out[i + 12] = translation[i];
				return out;
			}

			return rotation * pMat;
		}

		if (pAxis[1] > 0.f) //y
		{
			rotation[0] = c;
			rotation[2] = -s;
			rotation[8] = s;
			rotation[10] = c;

			if (pGlobal)
			{
				mat4 out;
				vec4 translation = { pMat[12], pMat[13], pMat[14], pMat[15] };
				out = pMat * rotation;

				for (size_t i = 0; i < 4; i++) out[i + 12] = translation[i];
				return out;
			}

			return rotation * pMat;
		}

		if (pAxis[2] > 0.f) //z
		{
			rotation[0] = c;
			rotation[1] = s;
			rotation[4] = -s;
			rotation[5] = c;

			if (pGlobal)
			{
				mat4 out;
				vec4 translation = { pMat[12], pMat[13], pMat[14], pMat[15] };
				out = pMat * rotation;

				for (size_t i = 0; i < 4; i++) out[i + 12] = translation[i];
				return out;
			}

			return rotation * pMat;
		}

		return rotation; // no axis, just return identity
	}

	mat4 Math::PerspectiveVKLH(float pFOV, float pAspect, float pNear, float pFar)
	{
		float yScale = 1.f / std::tanf(pFOV * .5f);
		float z = pNear / (pNear - pFar);

		return mat4
		{
			yScale / pAspect, 0.f, 0.f, 0.f,
			0.f, -yScale, 0.f, 0.f,
			0.f, 0.f, z, 1.f,
			0.f, 0.f, -pFar * z, 0.f,
		};
	}

	mat4 Math::Orthographic(float pRight, float pLeft, float pTop, float pBottom, float pNear, float pFar)
	{
		return mat4
		{
			2.f / (pRight - pLeft), 0.f, 0.f, -(pRight + pLeft) / (pRight - pLeft),
			0.f, 2.f / (pTop - pBottom), 0.f, -(pTop + pBottom) / (pTop - pBottom),
			0.f, 0.f, 1.f / (pFar - pNear), -pNear / (pFar - pNear),
			0.f, 0.f, 0.f, 1.f
		};
	}

	mat4 Math::Translate(mat4 pMat, vec3 pVec, bool pGlobal)
	{
		mat4 translation = identity;
		translation[12] = pVec[0];
		translation[13] = pVec[1];
		translation[14] = pVec[2];

		return pGlobal ? pMat * translation : translation * pMat;
	}
}
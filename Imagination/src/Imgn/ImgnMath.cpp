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

	float Math::Dot(quat pLQuat, quat pRQuat)
	{
		return pLQuat[0] * pRQuat[0] + pLQuat[1] * pRQuat[1] + pLQuat[2] * pRQuat[2] + pLQuat[3] * pRQuat[3];
	}

	float Math::Length(quat pQuat)
	{
		return std::sqrt(Dot(pQuat, pQuat));
	}

	quat Math::Normalize(quat pQuat)
	{
		float length = Length(pQuat);

		if (WithinStandardDeviation(length, 0.f)) return { 0.f, 0.f, 0.f, 1.f };

		float inverseLength = 1.f / length;

		return { pQuat[0] * inverseLength, pQuat[1] * inverseLength, pQuat[2] * inverseLength, pQuat[3] * inverseLength };
	}

	quat Math::Multiply(quat pLQuat, quat pRQuat)
	{
		return { pLQuat[3] * pRQuat[0] + pLQuat[0] * pRQuat[3] + pLQuat[1] * pRQuat[2] - pLQuat[2] * pRQuat[1], pLQuat[3] * pRQuat[1] - pLQuat[0] * pRQuat[2] + pLQuat[1] * pRQuat[3] + pLQuat[2] * pRQuat[0], pLQuat[3] * pRQuat[2] + pLQuat[0] * pRQuat[1] - pLQuat[1] * pRQuat[0] + pLQuat[2] * pRQuat[3], pLQuat[3] * pRQuat[3] - pLQuat[0] * pRQuat[0] - pLQuat[1] * pRQuat[1] - pLQuat[2] * pRQuat[2] };
	}

	quat Math::QuatFromEuler(vec3 pEuler)
	{
		float halfX = Radians(pEuler[0]) * .5f;
		float halfY = Radians(pEuler[1]) * .5f;
		float halfZ = Radians(pEuler[2]) * .5f;

		quat rotationX = { std::sin(halfX), 0.f, 0.f, std::cos(halfX) }, rotationY = { 0.f, std::sin(halfY), 0.f, std::cos(halfY) }, rotationZ = { 0.f, 0.f, std::sin(halfZ), std::cos(halfZ) };

		return Normalize(Multiply(Multiply(rotationX, rotationY), rotationZ));
	}

	vec3 Math::EulerFromQuat(quat pQuat)
	{
		mat4 rotation = RotationMatrix(Normalize(pQuat));

		return { Degrees(std::atan2(-rotation[9], rotation[10])), Degrees(std::atan2(rotation[8], std::sqrt(rotation[0] * rotation[0] + rotation[4] * rotation[4]))), Degrees(std::atan2(-rotation[4], rotation[0])) };
	}

	mat4 Math::RotationMatrix(quat pQuat)
	{
		float xx = pQuat[0] * pQuat[0];
		float yy = pQuat[1] * pQuat[1];
		float zz = pQuat[2] * pQuat[2];

		float xy = pQuat[0] * pQuat[1];
		float xz = pQuat[0] * pQuat[2];
		float yz = pQuat[1] * pQuat[2];

		float wx = pQuat[3] * pQuat[0];
		float wy = pQuat[3] * pQuat[1];
		float wz = pQuat[3] * pQuat[2];

		return { 1.f - 2.f * (yy + zz), 2.f * (xy + wz), 2.f * (xz - wy), 0.f, 2.f * (xy - wz), 1.f - 2.f * (xx + zz), 2.f * (yz + wx), 0.f, 2.f * (xz + wy), 2.f * (yz - wx), 1.f - 2.f * (xx + yy), 0.f, 0.f, 0.f, 0.f, 1.f };
	}

	quat Math::QuatFromRotationMatrix(mat4 pRotation)
	{
		quat rotation;

		float trace = pRotation[0] + pRotation[5] + pRotation[10];

		if (trace > 0.f)
		{
			float s = std::sqrt(trace + 1.f) * 2.f;

			rotation[3] = .25f * s;
			rotation[0] = (pRotation[6] - pRotation[9]) / s;
			rotation[1] = (pRotation[8] - pRotation[2]) / s;
			rotation[2] = (pRotation[1] - pRotation[4]) / s;
		}
		else if (pRotation[0] > pRotation[5] && pRotation[0] > pRotation[10])
		{
			float s = std::sqrt(1.f + pRotation[0] - pRotation[5] - pRotation[10]) * 2.f;

			rotation[3] = (pRotation[6] - pRotation[9]) / s;
			rotation[0] = .25f * s;
			rotation[1] = (pRotation[1] + pRotation[4]) / s;
			rotation[2] = (pRotation[2] + pRotation[8]) / s;
		}
		else if (pRotation[5] > pRotation[10])
		{
			float s = std::sqrt(1.f + pRotation[5] - pRotation[0] - pRotation[10]) * 2.f;

			rotation[3] = (pRotation[8] - pRotation[2]) / s;
			rotation[0] = (pRotation[1] + pRotation[4]) / s;
			rotation[1] = .25f * s;
			rotation[2] = (pRotation[6] + pRotation[9]) / s;
		}
		else
		{
			float s = std::sqrt(1.f + pRotation[10] - pRotation[0] - pRotation[5]) * 2.f;

			rotation[3] = (pRotation[1] - pRotation[4]) / s;
			rotation[0] = (pRotation[2] + pRotation[8]) / s;
			rotation[1] = (pRotation[6] + pRotation[9]) / s;
			rotation[2] = .25f * s;
		}

		return Normalize(rotation);
	}

	vec3 Math::Rotate(vec3 pVec, quat pRotation)
	{
		mat4 rotation = RotationMatrix(pRotation);

		return { pVec[0] * rotation[0] + pVec[1] * rotation[4] + pVec[2] * rotation[8], pVec[0] * rotation[1] + pVec[1] * rotation[5] + pVec[2] * rotation[9], pVec[0] * rotation[2] + pVec[1] * rotation[6] + pVec[2] * rotation[10] };
	}

	mat4 Math::Rotate(mat4 pMat, quat pRotation, bool pGlobal)
	{
		mat4 rotation = RotationMatrix(pRotation);

		if (pGlobal)
		{
			vec4 translation = { pMat[12], pMat[13], pMat[14], pMat[15] };
			mat4 out = pMat * rotation;

			for (size_t i = 0; i < 4; i++) out[i + 12] = translation[i];

			return out;
		}

		return rotation * pMat;
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

	mat4 Math::Transpose(mat4 pMat)
	{
		return mat4
		{
			pMat[0], pMat[4], pMat[8], pMat[12],
			pMat[1], pMat[5], pMat[9], pMat[13],
			pMat[2], pMat[6], pMat[10], pMat[14],
			pMat[3], pMat[7], pMat[11], pMat[15]
		};
	}

	float Math::Determinant(mat4 pMat)
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

		return a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;
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

	mat4 Math::Scale(mat4 pMat, vec3 pScale, bool pGlobal)
	{
		mat4 scale
		{
			pScale[0], 0.f, 0.f, 0.f,
			0.f, pScale[1], 0.f, 0.f,
			0.f, 0.f, pScale[2], 0.f,
			0.f, 0.f, 0.f, 1.f
		};

		if (pGlobal)
		{
			mat4 out;
			vec4 translation = { pMat[12], pMat[13], pMat[14], pMat[15] };

			out = pMat * scale;

			for (size_t i = 0; i < 4; i++) out[i + 12] = translation[i];

			return out;
		}


		return scale * pMat;
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

	void Math::Decompose(mat4 pMat, vec4& pTranslation, quat& pRotation, vec4& pScale)
	{
		//translate
		pTranslation = { pMat[12], pMat[13], pMat[14], 0 };

		//rotation
		float det = Determinant(pMat);
		float sx = sqrt(pMat[0] * pMat[0] + pMat[4] * pMat[4] + pMat[8] * pMat[8]);
		float sy = sqrt(pMat[1] * pMat[1] + pMat[5] * pMat[5] + pMat[9] * pMat[9]);
		float sz = sqrt(pMat[2] * pMat[2] + pMat[6] * pMat[6] + pMat[10] * pMat[10]);

		if (WithinStandardDeviation(det, 0.f)) return;
		if (det < 0) sx = -sx;

		mat4 rotation = pMat;
		rotation[0] /= sx;
		rotation[4] /= sx;
		rotation[8] /= sx;
		rotation[1] /= sy;
		rotation[5] /= sy;
		rotation[9] /= sy;
		rotation[2] /= sz;
		rotation[6] /= sz;
		rotation[10] /= sz;

		pRotation = QuatFromRotationMatrix(rotation);

		//scale
		pScale[0] = sqrt(pMat[0] * pMat[0] + pMat[4] * pMat[4] + pMat[8] * pMat[8]);
		pScale[1] = sqrt(pMat[1] * pMat[1] + pMat[5] * pMat[5] + pMat[9] * pMat[9]);
		pScale[2] = sqrt(pMat[2] * pMat[2] + pMat[6] * pMat[6] + pMat[10] * pMat[10]);
		pScale[3] = 0;
		if (det < 0) pScale[0] = -pScale[0];
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
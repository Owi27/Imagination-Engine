#pragma once

namespace Imgn
{
	class IMGN_API Math
	{
	public:
		Math() = delete;

		/// <summary>
		/// identity matrix
		/// </summary>
		static constexpr mat4 identity =
		{
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f,
		};

		static float Radians(float pDegrees) { return pDegrees * .01745329251f; }
		static float Degrees(float pRadians) { return pRadians * 57.2957795131f; }
		/* VEC2*/
		/* VEC3*/
		
		static float Dot(vec3 pLVec, vec3 pRVec);
		/// <summary>
		/// length/magnitude of a vector
		/// </summary>
		/// <param name="pVec">vector</param>
		/// <returns></returns>
		static float Length(vec3 pVec);
		static vec3 Normalize(vec3 pVec);


		/* VEC4*/

		/* MAT4 */
		static mat4 Inverse(mat4 pMat);
		static mat4 LookAtLH(vec3 pEye, vec3 pAt, vec3 pUp);
		static mat4 Rotate(mat4 pMat, vec3 pAxis, float pRadian, bool pGlobal = false);
		static mat4 Translate(mat4 pMat, vec3 pVec, bool pGlobal = false);
		static mat4 PerspectiveVKLH(float pFOV, float pAspect, float pNear, float pFar);
		static mat4 Orthographic(float pRight, float pLeft, float pTop, float pBottom, float pNear = -1.f, float pFar = 1.f);

	};

	/// <summary>
	/// vec3 add
	/// </summary>
	/// <param name="lhs"></param>
	/// <param name="rhs"></param>
	/// <returns></returns>
	inline vec3 operator+(const vec3& lhs, const vec3& rhs)
	{
		return vec3
		{
			lhs[0] + rhs[0],
			lhs[1] + rhs[1],
			lhs[2] + rhs[2]
		};
	}
	
	/// <summary>
	/// vec3 subtract
	/// </summary>
	/// <param name="lhs"></param>
	/// <param name="rhs"></param>
	/// <returns></returns>
	inline vec3 operator-(const vec3& lhs, const vec3& rhs)
	{
		return vec3
		{
			lhs[0] - rhs[0],
			lhs[1] - rhs[1],
			lhs[2] - rhs[2]
		};
	}

	/// <summary>
	/// vec3 cross product
	/// </summary>
	/// <param name="lhs"></param>
	/// <param name="rhs"></param>
	/// <returns></returns>
	inline vec3 operator*(const vec3& lhs, const vec3& rhs)
	{
		return vec3
		{
			(lhs[1] * rhs[2]) - (lhs[2] * rhs[1]),
			(lhs[2] * rhs[0]) - (lhs[0] * rhs[2]),
			(lhs[0] * rhs[1]) - (lhs[1] * rhs[0])
		};
	}

	inline mat4 operator*(const mat4& lhs, const float rhs)
	{
		mat4 out;
		
		for (size_t i = 0; i < 16; i++) out[i] = lhs[i] * rhs;

		return out;
	}
	
	inline mat4 operator*(const mat4& lhs, const mat4& rhs)
	{
		return mat4
		{
			lhs[0] * rhs[0] + lhs[1] * rhs[4] + lhs[2] * rhs[8] + lhs[3] * rhs[12], lhs[0] * rhs[1] + lhs[1] * rhs[5] + lhs[2] * rhs[9] + lhs[3] * rhs[13], lhs[0] * rhs[2] + lhs[1] * rhs[6] + lhs[2] * rhs[10] + lhs[3] * rhs[14], lhs[0] * rhs[3] + lhs[1] * rhs[7] + lhs[2] * rhs[11] + lhs[3] * rhs[15],
			lhs[4] * rhs[0] + lhs[5] * rhs[4] + lhs[6] * rhs[8] + lhs[7] * rhs[12], lhs[4] * rhs[1] + lhs[5] * rhs[5] + lhs[6] * rhs[9] + lhs[7] * rhs[13], lhs[4] * rhs[2] + lhs[5] * rhs[6] + lhs[6] * rhs[10] + lhs[7] * rhs[14], lhs[4] * rhs[3] + lhs[5] * rhs[7] + lhs[6] * rhs[11] + lhs[7] * rhs[15],
			lhs[8] * rhs[0] + lhs[9] * rhs[4] + lhs[10] * rhs[8] + lhs[11] * rhs[12], lhs[8] * rhs[1] + lhs[9] * rhs[5] + lhs[10] * rhs[9] + lhs[11] * rhs[13], lhs[8] * rhs[2] + lhs[9] * rhs[6] + lhs[10] * rhs[10] + lhs[11] * rhs[14], lhs[8] * rhs[3] + lhs[9] * rhs[7] + lhs[10] * rhs[11] + lhs[11] * rhs[15],
			lhs[12] * rhs[0] + lhs[13] * rhs[4] + lhs[14] * rhs[8] + lhs[15] * rhs[12], lhs[12] * rhs[1] + lhs[13] * rhs[5] + lhs[14] * rhs[9] + lhs[15] * rhs[13], lhs[12] * rhs[2] + lhs[13] * rhs[6] + lhs[14] * rhs[10] + lhs[15] * rhs[14], lhs[12] * rhs[3] + lhs[13] * rhs[7] + lhs[14] * rhs[11] + lhs[15] * rhs[15]
		};
	}
}
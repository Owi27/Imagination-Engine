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

		/// <summary>
		/// left handed look at
		/// </summary>
		/// <param name="pEye">Camera Pos</param>
		/// <param name="pAt">What to look at</param>
		/// <param name="pUp">Up axis</param>
		/// <returns>left handed look at mat4</returns>
		static mat4 LookAtLH(vec3 pEye, vec3 pAt, vec3 pUp);
		static mat4 PerspectiveVKLH(float pFOV, float pAspect, float pNear, float pFar);

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
}
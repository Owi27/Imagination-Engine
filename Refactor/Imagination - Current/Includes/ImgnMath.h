#include "pch.h"

namespace Math
{
	template<typename T>
	struct vec2
	{
		union
		{
			struct
			{
				T x, y;
			};

			T data[2];
		};

		constexpr vec2<T>& operator+=(const vec2<T>& rhs)
		{
			for (int i = 0; i < 2; i++) data[i] += rhs.data[i];

			return *this;
		}

		constexpr vec2<T>& operator-=(const vec2<T>& rhs)
		{
			for (int i = 0; i < 2; i++) data[i] -= rhs.data[i];

			return *this;
		}

		constexpr T Length() { return std::sqrt(std::pow(x, 2) + std::pow(y, 2)); }
		constexpr T Normalize() { return std::sqrt(std::pow(x, 2) + std::pow(y, 2)); }
		//inline constexpr vec2<T> Distance(vec2<T>& pOther) { return }
	};

	template<typename T>
	constexpr vec2<T> operator+(vec2<T> lhs, vec2<T> rhs) { return lhs += rhs; }

	template<typename T>
	constexpr vec2<T> operator-(vec2<T> lhs, vec2<T> rhs) { return lhs -= rhs; }

	template<typename T>
	constexpr T Distance(vec2<T> lhs, vec2<T> rhs) { rhs -= lhs; return rhs.Normalize(); }

	template<typename T>
	constexpr T Dot(vec2<T> lhs, vec2<T> rhs) { return (lhs.x * rhs.x) + (lhs.y * rhs.y); }

	template<typename T>
	constexpr T Angle(vec2<T> lhs, vec2<T> rhs) { return std::pow(std::cos(Dot(lhs, rhs) / (lhs.Normalize() * rhs.Normalize())), -1); }

	template<typename T>
	constexpr T Cross(vec2<T> lhs, vec2<T> rhs) { return std::pow(std::cos(Dot(lhs, rhs) / (lhs.Normalize() * rhs.Normalize())), -1); }



	template<typename T>
	struct vec3
	{
		union
		{
			struct
			{
				T x, y, z;
			};

			T data[3];
		};

		constexpr vec3<T>& operator+=(const vec3<T>& rhs)
		{
			for (int i = 0; i < 3; i++) data[i] += rhs.data[i];

			return *this;
		}

		constexpr vec3<T>& operator-=(const vec3<T>& rhs)
		{
			for (int i = 0; i < 3; i++) data[i] -= rhs.data[i];

			return *this;
		}


		inline constexpr vec2<T> xy() const { return vec2<T>{.x = this->x, .y = this->y}; }

		inline constexpr T Length() { return std::sqrt((x * x) + (y * y) + (z * z)); }
		inline constexpr vec3<T> Normalize() { auto length = Length(); return vec3<T>{.x = x / length, .y = y / length, .z = z / length }; }
	};

	template<typename T>
	constexpr T Dot(vec3<T> lhs, vec3<T> rhs) { return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z); }

	template<typename T>
	constexpr T Angle(vec3<T> lhs, vec3<T> rhs) { return std::acos(Dot(lhs, rhs) / (lhs.Length() * rhs.Length())); }

	template<typename T>
	constexpr vec3<T> Cross(vec3<T> lhs, vec3<T> rhs) { return vec3<T>{.x = (lhs.y * rhs.z) - (rhs.z * lhs.y), .y = (lhs.z * rhs.x) - (lhs.x * rhs.z), .z = (lhs.x * rhs.y) - (lhs.y = rhs.x) }; }


	template<typename T>
	struct vec4
	{
		union
		{
			struct
			{
				T x, y, z, w;
			};

			T data[4];
		};

		constexpr vec4<T>& operator+=(const vec4<T>& rhs)
		{
			for (int i = 0; i < 4; i++) data[i] += rhs.data[i];

			return *this;
		}

		constexpr vec4<T>& operator-=(const vec4<T>& rhs)
		{
			for (int i = 0; i < 4; i++) data[i] -= rhs.data[i];

			return *this;
		}

		inline constexpr vec2<T> xy() const { return vec2<T>{.x = this->x, .y = this->y}; }
		inline constexpr vec3<T> xyz() const { return vec3<T>{.x = this->x, .y = this->y, .z = this->z}; }

		inline constexpr T Length() { return std::sqrt((x * x) + (y * y) + (z * z) + (w * w)); }
		inline constexpr vec4<T> Normalize() { auto length = Length(); return vec4<T>{.x = x / length, .y = y / length, .z = z / length, .w = w / length }; }
	};

	template<typename T>
	constexpr vec4<T> Cross(vec4<T> lhs, vec4<T> rhs) { return vec4<T>{.x = (lhs.y * rhs.z) - (rhs.z * lhs.y), .y = (lhs.z * rhs.x) - (lhs.x * rhs.z), .z = (lhs.x * rhs.y) - (lhs.y = rhs.x), .w = lhs.w * rhs.w }; }

	template<typename T>
	struct mat2
	{
		union
		{
			struct
			{
				vec2<T> row1;
				vec2<T> row2;
			};

			T data[4];
		};

		constexpr mat2<T>& operator+=(const mat2<T>& rhs)
		{
			for (int i = 0; i < 4; i++) data[i] += rhs.data[i];

			return *this;
		}

		constexpr mat2<T>& operator-=(const mat2<T>& rhs)
		{
			for (int i = 0; i < 4; i++) data[i] -= rhs.data[i];

			return *this;
		}
	};

	template<typename T>
	struct mat3
	{
		union
		{
			struct
			{
				vec3<T> row1;
				vec3<T> row2;
				vec3<T> row3;
			};

			T data[9];
		};

		constexpr mat3<T>& operator+=(const mat3<T>& rhs)
		{
			for (int i = 0; i < 9; i++) data[i] += rhs.data[i];

			return *this;
		}

		constexpr mat3<T>& operator-=(const mat3<T>& rhs)
		{
			for (int i = 0; i < 9; i++) data[i] -= rhs.data[i];

			return *this;
		}
	};

	template<typename T>
	struct mat4
	{
		union
		{
			struct
			{
				vec4<T> row1;
				vec4<T> row2;
				vec4<T> row3;
				vec4<T> row4;
			};

			T data[16];
		};

		constexpr mat4<T>& operator+=(const mat4<T>& rhs)
		{
			for (int i = 0; i < 16; i++) data[i] += rhs.data[i];

			return *this;
		}

		constexpr mat4<T>& operator-=(const mat4<T>& rhs)
		{
			for (int i = 0; i < 16; i++) data[i] -= rhs.data[i];

			return *this;
		}

		constexpr mat4<T>& operator*=(const mat4<T>& rhs)
		{
			row1.x = row1.x * rhs.row1.x + row1.y * rhs.row2.x + row1.z * rhs.row3.x + row1.w * rhs.row4.x;
			row1.y = row1.x * rhs.row1.y + row1.y * rhs.row2.y + row1.z * rhs.row3.y + row1.w * rhs.row4.y;
			row1.z = row1.x * rhs.row1.z + row1.y * rhs.row2.z + row1.z * rhs.row3.z + row1.w * rhs.row4.z;
			row1.w = row1.x * rhs.row1.w + row1.y * rhs.row2.w + row1.z * rhs.row3.w + row1.w * rhs.row4.y;

			row2.x = row2.x * rhs.row1.x + row2.y * rhs.row2.x + row2.z * rhs.row3.x + row2.w * rhs.row4.x;
			row2.y = row2.x * rhs.row1.y + row2.y * rhs.row2.y + row2.z * rhs.row3.y + row2.w * rhs.row4.y;
			row2.z = row2.x * rhs.row1.z + row2.y * rhs.row2.z + row2.z * rhs.row3.z + row2.w * rhs.row4.z;
			row2.w = row2.x * rhs.row1.w + row2.y * rhs.row2.w + row2.z * rhs.row3.w + row2.w * rhs.row4.y;

			row3.x = row3.x * rhs.row1.x + row3.y * rhs.row2.x + row3.z * rhs.row3.x + row3.w * rhs.row4.x;
			row3.y = row3.x * rhs.row1.y + row3.y * rhs.row2.y + row3.z * rhs.row3.y + row3.w * rhs.row4.y;
			row3.z = row3.x * rhs.row1.z + row3.y * rhs.row2.z + row3.z * rhs.row3.z + row3.w * rhs.row4.z;
			row3.w = row3.x * rhs.row1.w + row3.y * rhs.row2.w + row3.z * rhs.row3.w + row3.w * rhs.row4.y;

			row4.x = row4.x * rhs.row1.x + row4.y * rhs.row2.x + row4.z * rhs.row3.x + row4.w * rhs.row4.x;
			row4.y = row4.x * rhs.row1.y + row4.y * rhs.row2.y + row4.z * rhs.row3.y + row4.w * rhs.row4.y;
			row4.z = row4.x * rhs.row1.z + row4.y * rhs.row2.z + row4.z * rhs.row3.z + row4.w * rhs.row4.z;
			row4.w = row4.x * rhs.row1.w + row4.y * rhs.row2.w + row4.z * rhs.row3.w + row4.w * rhs.row4.y;

			return *this;
		}
	};

	/* Functions */

	template<typename T>
	constexpr mat4<T> Identity()
	{
		mat4<T> out
		{
			.row1
			{
				.x = static_cast<T>(1)
			},
			.row2
			{
				.y = static_cast<T>(1)
			},
			.row3
			{
				.z = static_cast<T>(1)
			},
			.row4
			{
				.w = static_cast<T>(1)
			}
		};

		return out;
	}

	template<typename T>
	constexpr mat4<T> RotateG(mat4<T> pMat, float pRadian, vec3<T> pAxis)
	{
		mat4<T> out = Identity<T>();

		T c = std::cos(pRadian);
		T s = std::sin(pRadian);

		mat4<T> rot = Identity<T>();

		if (pAxis.x)
		{
			rot.data[5] = c;
			rot.data[6] = s;
			rot.data[9] = -s;
			rot.data[10] = c;
		}

		if (pAxis.y)
		{
			rot.data[0] = c;
			rot.data[2] = -s;
			rot.data[8] = s;
			rot.data[10] = c;
		}

		if (pAxis.z)
		{
			rot.data[0] = c;
			rot.data[1] = s;
			rot.data[4] = -s;
			rot.data[5] = c;
		}

		vec4<T> trans = pMat.row4;

		pMat *= rot;
		pMat.row4 = trans;

		return pMat;
	}

	template<typename T>
	constexpr mat4<T> LookAtL(vec4<T> pEye, vec4<T> pAt, vec4<T> pUp)
	{
		mat4<T> out;
		vec4<T> camR, camU;
		vec4<T> camF
		{
			.x = pAt.x - pEye.x,
			.y = pAt.y - pEye.y,
			.z = pAt.z - pEye.z,
		};

		vec4<T> nCamF = camF.Normalize();
		vec4<T> nCamR = Cross(pUp, nCamF).Normalize();

		camU = Cross(nCamF, nCamR);

		vec4<T> temp
		{
			.x = nCamR.x * pEye.x + nCamR.y * pEye.y + nCamR.z * pEye.z,
			.y = camU.x * pEye.x + camU.y * pEye.y + camU.z * pEye.z,
			.z = nCamF.x * pEye.x + nCamF.y * pEye.y + nCamF.z * pEye.z,
		};

		out.row1.x = nCamR.x;
		out.row2.x = nCamR.y;
		out.row3.x = nCamR.z;
		out.row4.x = -temp.x;

		out.row1.y = camU.x;
		out.row2.y = camU.y;
		out.row3.y = camU.z;
		out.row4.y = -temp.y;

		out.row1.z = nCamF.x;
		out.row2.z = nCamF.y;
		out.row3.z = nCamF.z;
		out.row4.z = -temp.z;

		return out;
	}

	template<typename T>
	constexpr mat4<T> Projection(float pFOV, float pAspect, float pNear, float pFar)
	{
		mat4<T> out = {};

		float yScale = 1.f / std::tanf(pFOV * .5f);
		float z = pFar / (pFar - pNear);

		out.row1.x = yScale / pAspect;
		out.row2.y = -yScale;
		out.row3.z = z;
		out.row3.w = 1.f;
		out.row4.z = -pNear * z;

		return out;
	}


	constexpr float Radians(float pDegree) { return pDegree * 0.01745329251f; }
	constexpr float Degrees(float pRadian) { return pRadian * 57.2957795131f; }

	//constexpr T Dot(vec4<T> lhs, vec4<T> rhs)
	//{
	//	return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z) + (lhs.w * rhs.w);
	//}
}


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
	};

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

		inline const vec2<T> xy() const { return vec2<T>{.x = this->x, .y = this->y}; }
	};

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

		inline const vec2<T> xy() const { return vec2<T>{.x = this->x, .y = this->y}; }
		inline const vec3<T> xyz() const { return vec3<T>{.x = this->x, .y = this->y, .z = this->z}; }
	};

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
	};



}
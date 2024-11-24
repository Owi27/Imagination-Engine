#pragma once
#include "pch.h"

mat4 operator*(mat4 const& m1, mat4 const& m2)
{
	mat4 r;
	GMatrix::MultiplyMatrixF(m1, m2, r);
	return r;
}

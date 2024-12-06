#pragma once

//overloaded operators
mat4 operator*(mat4 const& m1, mat4 const& m2)
{
    mat4 m;
    GMatrix::MultiplyMatrixF(m1, m2, m);

    return m;
}
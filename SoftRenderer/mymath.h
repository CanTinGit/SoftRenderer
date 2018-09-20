#pragma once

#include "geometry.h"

#define PI ((float) 3.141592654f)
#define PI2 ((float)6.283185307f)
#define PI_DIV_2 ((float)1.570796327f)
#define PI_DIV_4 ((float)0.785398163f)
#define PI_DIV_INV ((float)0.318309886f)

#define FIXP16_SHIFT 16
#define FIXP16_MAG 65536
#define FIXP16_DP_MASK 0x0000ffff
#define FIXP16_WP_MASK 0xffff0000
#define FIXP16_ROUND_UP 0x00008000

#define EPSILON_E4 (float)(1E-4)
#define EPSILON_E5 (float)(1E-5)
#define EPSILON_E6 (float)(1E-6)

/////////Matrix to Vector, Vector to Matrix//////////
Vector3f MatrixToVector(Matrix m)
{
	return Vector3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

Matrix VectorToMatrix(Vector3f v)
{
	Matrix result(4, 1);
	result[0][0] = v.x;
	result[1][0] = v.y;
	result[2][0] = v.z;
	result[3][0] = 1.0f;
	return result;
}


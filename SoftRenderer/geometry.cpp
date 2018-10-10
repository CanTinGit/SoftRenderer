#include "geometry.h"

template <> template <> Vector3D<int>::Vector3D(const Vector3D<float> &v)
{
	x = int(v.x + 0.5f);
	y = int(v.y + 0.5f);
	z = int(v.z + 0.5f);
}

template <> template <> Vector3D<float>::Vector3D(const Vector3D<int> &v)
{
	x = v.x;
	y = v.y;
	z = v.z;
}

template <> template <> Vector4D<int>::Vector4D(const Vector4D<float> &v)
{
	x = int(v.x + 0.5f);
	y = int(v.y + 0.5f);
	z = int(v.z + 0.5f);
}

template <> template <> Vector4D<float>::Vector4D(const Vector4D<int> &v)
{
	x = v.x;
	y = v.y;
	z = v.z;
}

Matrix::Matrix(int r, int c) {
	m = std::vector<std::vector<float> >(r, std::vector<float>(c, 0.f));
	rows = r;
	cols = c;
}

int Matrix::Cols() {
	return cols;
}

int Matrix::Rows() {
	return rows;
}

std::vector<float>& Matrix::operator[](const int i)
{
	assert(i >= 0 && i < rows);
	return m[i];
}

Matrix Matrix::operator*(const Matrix& a)
{
	assert(cols == a.rows);
	int col = a.cols;
	Matrix result(rows, a.cols);
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < a.cols; j++)
		{
			result.m[i][j] = 0.0f;
			for (int k = 0; k < cols; k++)
			{
				result.m[i][j] += m[i][k] * a.m[k][j];
			}
		}
	}
	return result;
}

Vector4f Matrix::operator*(const Vector4f& a)
{
	Vector4f v;
	v.x = a.x * m[0][0] + a.y * m[1][0] + a.z *m[2][0] + a.w * m[3][0];
	v.y = a.x * m[0][1] + a.y * m[1][1] + a.z *m[2][1] + a.w * m[3][1];
	v.z = a.x * m[0][2] + a.y * m[1][2] + a.z *m[2][2] + a.w * m[3][2];
	v.w = a.x * m[0][3] + a.y * m[1][3] + a.z *m[2][3] + a.w * m[3][3];
	return v;
}

Matrix Matrix::operator*(float s)
{
	Matrix result;
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			result[i][j] = m[i][j] * s;
		}
	}
	return result;
}

Matrix Matrix::Identity(int dimension) {
	Matrix E(dimension, dimension);
	for (int i = 0; i < dimension; i++)
	{
		for (int j = 0; j < dimension; j++)
		{
			E[i][j] = (i == j ? 1.0f : 0.0f);
		}
	}
	return E;
}

Matrix Matrix::Transpose()
{
	Matrix result(cols, rows);
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			result[j][i] = m[i][j];
		}
	}
	return result;
}

Matrix Matrix::Inverse()
{
	assert(rows == cols);
	Matrix result(rows, cols * 2);

	for (int i = 0; i < rows; i++)
		for (int j = 0; j < cols; j++)
			result[i][j] = m[i][j];

	for (int i = 0; i < rows; i++)
		result[i][i + cols] = 1;

	for (int i = 0; i < rows - 1; i++)
	{
		for (int j = result.cols - 1; j >= 0; j--)
			result[i][j] /= result[i][i];
		for (int k = i + 1; k < rows; k++)
		{
			float coeff = result[k][i];
			for (int j = 0; j < result.cols; j++)
				result[k][j] -= result[i][j] * coeff;
		}
	}

	for (int i = result.cols - 1; i >= rows - 1; i--)
		result[rows - 1][i] /= result[rows - 1][rows - 1];

	for (int i = rows - 1; i > 0; i--)
	{
		for (int j = i - 1; j >= 0; j--)
		{
			float coeff = result[j][i];
			for (int k = 0; k < result.cols; k++)
			{
				result[j][k] -= result[i][k] * coeff;
			}
		}
	}

	Matrix truncate(rows, cols);
	for (int i = 0; i < rows; i++)
		for (int j = 0; j < cols; j++)
			truncate[i][j] = result[i][j + cols];
	return truncate;
}

Matrix Matrix::ZeroMatrix(int dimension) {
	Matrix E(dimension, dimension);
	for (int i = 0; i < dimension; i++)
	{
		for (int j = 0; j < dimension; j++)
		{
			E[i][j] = 0.0f;
		}
	}
	return E;
}

Matrix Matrix::TranslateMatrix(float x, float y, float z, Matrix &E) {
	
	E[3][0] = x;
	E[3][1] = y;
	E[3][2] = z;
	return E;
}

Matrix Matrix::ScaleMatrix(float x, float y, float z) {
	Matrix E = Identity(4);
	E[0][0] = x;
	E[1][1] = y;
	E[2][2] = z;
	return E;
}

Matrix Matrix::RotateMatrix(float x, float y, float z, float theta) {
	Matrix E = Identity(4);
	//float qsin = (float)sin(theta);
	//float qcos = (float)cos(theta);
	//float one_qcos = 1 - qcos;
	//Vector4f vi(x, y, z, 1);
	//vi.normalize();
	//float X = vi.x, Y = vi.y, Z = vi.z;
	//E.m[0][0] = qcos + X * X*one_qcos;
	//E.m[1][0] = X * Y*one_qcos - Z * qsin;
	//E.m[2][0] = X * Z*one_qcos + Y * qsin;
	//E.m[3][0] = 0.0f;
	//E.m[0][1] = Y * X*one_qcos + Z * qsin;
	//E.m[1][1] = qcos + Y * Y*one_qcos;
	//E.m[2][1] = Y * Z*one_qcos - X * qsin;
	//E.m[3][1] = 0.0f;
	//E.m[0][2] = Z * X*one_qcos - Y * qsin;
	//E.m[1][2] = Z * Y*one_qcos + X * qsin;
	//E.m[2][2] = qcos + Z * Z*one_qcos;
	//E.m[3][2] = 0.0f;
	//E.m[0][3] = 0;
	//E.m[1][3] = 0;
	//E.m[2][3] = 0;
	//E.m[3][3] = 1.0f;
	float qsin = (float)sin(theta * 0.5f);
	float qcos = (float)cos(theta*0.5f);
	float one_qcos = 1 - qcos;
	Vector4f vi(x, y, z, 1);
	float w = qcos;
	vi.normalize();
	x = vi.x *qsin;
	y = vi.y*qsin;
	z = vi.z *qsin;
	E.m[0][0] = 1 - 2 * y*y - 2 * z*z;
	E.m[1][0] = 2 * x*y - 2 * w*z;
	E.m[2][0] = 2 * w*z + 2 * w*y;
	E.m[3][0] = 0.0f;
	E.m[0][1] = 2 * x*y + 2 * w*z;
	E.m[1][1] = 1 - 2 * x*x - 2 * z*z;
	E.m[2][1] = 2 * y*z - 2 * w*x;
	E.m[3][1] = 0.0f;
	E.m[0][2] = 2 * x*z - 2 * w*y;
	E.m[1][2] = 2 * y*z + 2 * w*x;
	E.m[2][2] = 1 - 2 * x*x - 2 * y*y;
	E.m[3][2] = 0.0f;
	E.m[0][3] = 0;
	E.m[1][3] = 0;
	E.m[2][3] = 0;
	E.m[3][3] = 1.0f;
	return E;
}

void Color::Set(UINT32 x, float s) {
	uint32 = x;
	argb[0] *= s;
	argb[1] *= s;
	argb[2] *= s;
}
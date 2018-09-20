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

Matrix::Matrix(int r,int c) {
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
	Matrix result(rows, cols*2);

	for (int i = 0; i < rows; i++)
		for (int j = 0; j < cols; j++)
			result[i][j] = m[i][j];

	for (int i = 0; i < rows; i++)
		result[i][i + cols] = 1;

	for (int i = 0; i < rows-1; i++)
	{
		for (int j = result.cols - 1; j >= 0; j--)
			result[i][j] /= result[i][i];
		for (int k = i+1; k < rows; k++)
		{
			float coeff = result[k][i];
			for (int j = 0; j < result.cols; j++)
				result[k][j] -= result[i][j] * coeff;
		}
	}

	for (int i = result.cols - 1; i >= rows - 1; i--)
		result[rows - 1][i] /= result[rows - 1][rows - 1];

	for (int i = rows-1; i > 0; i--)
	{
		for (int j = i-1; j >= 0; j--)
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


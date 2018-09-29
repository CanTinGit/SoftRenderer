///////////////////////////////////////////////////////////////////////////////////////////////////
//refer:https://github.com/ssloy/tinyrenderer/blob/f6fecb7ad493264ecd15e230411bfb1cca539a12/geometry.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cmath>
#include <iostream>
#include <vector>
#include <assert.h>

typedef unsigned char BYTE;
typedef unsigned int UINT32;

template <class t> class Vector4D;
//////////Vector////////////////
template <class t> class Vector2D {
public:
	t x, y;
	Vector2D(): x(0),y(0){}
	Vector2D(t _x,t _y): x(_x), y(_y){}
	inline Vector2D<t> operator +(const Vector2D<t> &v) const { return Vector2D<t>(x + v.x, y + v.y); }
	inline Vector2D<t> operator -(const Vector2D<t> &v) const { return Vector2D<t>(x - v.x, y - v.y); }
	inline Vector2D<t> operator *(float f)          const { return Vector2D<t>(x*f, y*f); }
	inline t       operator *(const Vector2D<t> &v) const { return x * v.x + y * v.y; }
	t& operator [](const int i) { if (i <= 0)return x; else return y; }
};

template <class t> struct Vector3D {
public:
	t x, y, z;
	Vector3D() : x(0), y(0), z(0) {}
	Vector3D(t _x, t _y, t _z) : x(_x), y(_y), z(_z) {}
	template <class u> Vector3D<t>(const Vector3D<u> &v);
	Vector3D<t>(const Vector3D<t> &v) : x(t()), y(t()), z(t()) { *this = v; }
	Vector3D<t> & operator =(const Vector3D<t> &v)
	{
		if (this != &v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
		}
		return *this;
	}
	Vector3D<t>& operator =(const Vector4D<t> &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	inline Vector3D<t> operator ^(const Vector3D<t> &v) const { return Vector3D<t>(y*v.z - z * v.y, z*v.x - x * v.z, x*v.y - y * v.x); }
	inline Vector3D<t> operator +(const Vector3D<t> &v) const { return Vector3D<t>(x + v.x, y + v.y, z + v.z); }
	inline Vector3D<t> operator -(const Vector3D<t> &v) const { return Vector3D<t>(x - v.x, y - v.y, z - v.z); }
	inline Vector3D<t> operator *(float f)          const { return Vector3D<t>(x*f, y*f, z*f); }
	inline t       operator *(const Vector3D<t> &v) const { return x * v.x + y * v.y + z * v.z; }

	t& operator [](const int i) { if (i <= 0)return x; else if (i == 1) return y; else return z; }
	//t       operator *(const Vec3<t> &v) const { return x * v.x + y * v.y + z * v.z; }
	float norm() const { return std::sqrt(x*x + y * y + z * z); }
	Vector3D<t> & normalize(t l = 1) { *this = (*this)*(l / norm()); return *this; }
	template <class > friend std::ostream& operator<<(std::ostream& s, Vector3D<t>& v);
};

template <class t> struct Vector4D {
public:
	t x, y, z,w;
	Vector4D() : x(0), y(0), z(0),w(1) {}
	Vector4D(t _x, t _y, t _z,t _w) : x(_x), y(_y), z(_z),w(_w) {}
	template <class u> Vector4D<t>(const Vector4D<u> &v);
	Vector4D<t>(const Vector4D<t> &v) : x(t()), y(t()), z(t()) { *this = v; }
	Vector4D<t> & operator =(const Vector4D<t> &v)
	{
		if (this != &v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
			w = v.w;
		}
		return *this;
	}

	Vector4D<t> & operator =(const Vector3D<t> &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		w = 1.0f;
		return *this;
	}

	inline Vector4D<t> operator ^(const Vector4D<t> &v) const { return Vector4D<t>(y*v.z - z * v.y, z*v.x - x * v.z, x*v.y - y * v.x,1); }
	inline Vector4D<t> operator +(const Vector4D<t> &v) const { return Vector4D<t>(x + v.x, y + v.y, z + v.z,1); }
	inline Vector4D<t> operator -(const Vector4D<t> &v) const { return Vector4D<t>(x - v.x, y - v.y, z - v.z,1); }
	inline Vector4D<t> operator *(float f)          const { return Vector4D<t>(x*f, y*f, z*f,1); }
	inline t       operator *(const Vector4D<t> &v) const { return x * v.x + y * v.y + z * v.z; }

	t& operator [](const int i) { if (i <= 0)return x; else if (i == 1) return y; else return z; }
	//t       operator *(const Vec3<t> &v) const { return x * v.x + y * v.y + z * v.z; }
	float norm() const { return std::sqrt(x*x + y * y + z * z); }
	Vector4D<t> & normalize(t l = 1)
	{
		if (norm() != 0.0f)
		{
			*this = (*this)*(l / norm());
		} 
		(*this).w = (t)1; 
		return *this;
	}
	template <class > friend std::ostream& operator<<(std::ostream& s, Vector4D<t>& v);
};

typedef Vector2D<float> Vector2f;
typedef Vector2D<int>   Vector2i;
typedef Vector3D<float> Vector3f;
typedef Vector3D<int>   Vector3i;
typedef Vector4D<float> Vector4f;
typedef Vector4D<int>   Vector4i;

template <> template <> Vector3D<int>::Vector3D(const Vector3D<float> &v);
template <> template <> Vector3D<float>::Vector3D(const Vector3D<int> &v);
template <> template <> Vector4D<int>::Vector4D(const Vector4D<float> &v);
template <> template <> Vector4D<float>::Vector4D(const Vector4D<int> &v);

template <class t> std::ostream& operator<<(std::ostream& s, Vector2D<t>& v) {
	s << "(" << v.x << ", " << v.y << ")\n";
	return s;
}

template <class t> std::ostream& operator<<(std::ostream& s, Vector3D<t>& v) {
	s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
	return s;
}

template <class t> std::ostream& operator<<(std::ostream& s, Vector4D<t>& v) {
	s << "(" << v.x << ", " << v.y << ", " << v.z <<", " << v.w <<")\n";
	return s;
}

//////////Matrix////////////////
const int Default_ALLOC = 4;
class Matrix
{
public:
	std::vector<std::vector<float>> m;
	int rows, cols;
	Matrix(int r=Default_ALLOC, int c = Default_ALLOC);
	inline int Rows();
	inline int Cols();

	static Matrix Identity(int dimensions);
	std::vector<float>& operator[](const int i);
	Matrix operator*(const Matrix& a);
	Vector4f operator*(const Vector4f& a);
	Matrix operator*(float s);
	Matrix Transpose();
	Matrix Inverse();
	
	static Matrix ZeroMatrix(int dimension);
	static Matrix TranslateMatrix(float x, float y, float z);
	static Matrix ScaleMatrix(float x, float y, float z);
	static Matrix RotateMatrix(float x, float y, float z, float theta);

};


class Color
{
public:
	union
	{
		BYTE argb[4]; //bgra 小端存储
		UINT32 uint32;
	};
	void Set(UINT32 x, float s);
	inline Color operator*(const Color &color) {
		Color result; result.argb[0] = argb[0] * color.argb[0]; result.argb[1] = argb[1] * color.argb[1];
		result.argb[2] = argb[2] * color.argb[2]; return result;
	}
};

class Vertex
{
public:
	Vector4f normal;                 //法向量
	Vector4f local;                  //局部坐标
	Vector4f coordinates;			 //投影后的坐标
	Vector4f worldCoordinates;		 //原世界坐标
	Vector4i color;
	float u, v;
};
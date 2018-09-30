#pragma once

#include "geometry.h"
#include "model.h"

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
//Vector3f MatrixToVector(Matrix m)
//{
//	return Vector3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
//}
//
//Matrix VectorToMatrix(Vector3f v)
//{
//	Matrix result(4, 1);
//	result[0][0] = v.x;
//	result[1][0] = v.y;
//	result[2][0] = v.z;
//	result[3][0] = 1.0f;
//	return result;
//}
//
//Vector3f Translate(Vector3f vertex, Vector3f model_center)
//{
//	Matrix modelToWorld = Matrix::Identity(4);
//	modelToWorld[3][0] = model_center.x;
//	modelToWorld[3][1] = model_center.y;
//	modelToWorld[3][2] = model_center.z;
//
//	Matrix temp = VectorToMatrix(vertex);
//	temp = temp * modelToWorld;
//	return MatrixToVector(temp);
//}
//
////相机坐标转换为投影坐标
//Vector3f TransformToViewPort(float width, float height, Vector3f vertex)
//{
//	Matrix viewport = Matrix::Identity(4);
//	viewport[3][3] = 0;
//	viewport[2][3] = 1;
//	float d = 0.5f * width * 1; //1表示tan(45°),实际应为tan(fov/2);
//	viewport[0][0] = d;
//	viewport[1][1] = d * (float)(width / height);
//
//	Matrix temp = VectorToMatrix(vertex);
//	temp = temp * viewport;
//	return MatrixToVector(temp);
//}
//
//
////由投影坐标转换为屏幕坐标
//Vector2f TransformToScreen(Vector3f viewPortPosition, float width, float height)
//{
//	float x = (viewPortPosition.x + 1) *(0.5f *width - 0.5f);
//	float y = (height - 1) - (viewPortPosition.y + 1) *(0.5f * height - 0.5f);
//	Vector2f v(x, y);
//	return v;
//}

//bool SphereTest(Model model, float far_z, float near_z)
//{
//	std::vector<Vector4f> testPoint;
//	testPoint.push_back(model.Position());
//	testPoint.push_back(Vector4f(model.Position().x, model.Position().y, model.Position().z + model.Max_Radius(),1.0f));
//	testPoint.push_back(Vector4f(model.Position().x, model.Position().y, model.Position().z - model.Max_Radius(),1.f));
//	//testPoint.push_back(Vector3f(model.Position().x, model.Position().y + model.Max_Radius(), model.Position().z ));
//	//testPoint.push_back(Vector3f(model.Position().x, model.Position().y - model.Max_Radius(), model.Position().z ));
//	testPoint.push_back(Vector4f(model.Position().x + model.Max_Radius(), model.Position().y, model.Position().z,1.f));
//	testPoint.push_back(Vector4f(model.Position().x - model.Max_Radius(), model.Position().y , model.Position().z,1.f));
//
//	for (int i = 0; i < testPoint.size(); i++)
//	{
//		if (!( (testPoint[i].z) >far_z || (testPoint[i].z< near_z) || (fabs(testPoint[i].x) < testPoint[i].z) || (fabs(testPoint[i].y) < testPoint[i].z)))
//		{
//			return true;
//		}
//	}
//	return false;
//}

float inline DegToRad(int angle)
{
	return (angle)*PI / 180.0f;
}

float inline DegToRad(float angle)
{
	return (angle)*PI / 180.0f;
}

float inline RadToDeg(int rads)
{
	return (float)rads*180.0f / PI;
}

float inline RadToDeg(float rads)
{
	return rads * 180.0f / PI;
}

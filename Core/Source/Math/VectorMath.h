//
// This is a system of classes that wrap DirectXMath in a more object-oriented and concise way.
// DirectXMath treats vectors like [1x4] matrices (rows) rather than [4x1] matrices (columns). 
// It treats matrices like they are transposed, so multiplication will be left associated.
// In this library, we will use right association: Vector [4x1] = Matrix4 [4x4] * Vector [4x1].
// Under the hood, it will use the methods provided by DirectXMath.
// There is no need to transpose matrix in a shader constant buffer. Just multiply matrix backwards.
// In the shader, call mul( matrix, vector ) rather than mul( vector, matrix ).
// In addtion, right-handed coordinate system are used.
// Note: currently only support for x64 platform. So, we can safely use XMVECTOR without any alignment concern.
//

#pragma once

#include "Scalar.h"
#include "Vector.h"
#include "Quaternion.h"
#include "Matrix3.h"
#include "Transform.h"
#include "Matrix4.h"

namespace Math {

// Template parameter T should be Scalar, Vector3 and Vector4.
template<class T> T Sqrt(T s) { return T(DirectX::XMVectorSqrt(s)); }
template<class T> T Recip(T s) { return T(DirectX::XMVectorReciprocal(s)); }
template<class T> T RecipSqrt(T s) { return T(DirectX::XMVectorReciprocalSqrt(s)); }
template<class T> T Floor(T s) { return T(DirectX::XMVectorFloor(s)); }
template<class T> T Ceiling(T s) { return T(DirectX::XMVectorCeiling(s)); }
template<class T> T Round(T s) { return T(DirectX::XMVectorRound(s)); }
template<class T> T Abs(T s) { return T(DirectX::XMVectorAbs(s)); }
template<class T> T Exp(T s) { return T(DirectX::XMVectorExp(s)); }
template<class T> T Pow(T b, T e) { return T(DirectX::XMVectorPow(b, e)); }
template<class T> T Log(T s) { return T(DirectX::XMVectorLog(s)); }
template<class T> T Sin(T s) { return T(DirectX::XMVectorSin(s)); }
template<class T> T Cos(T s) { return T(DirectX::XMVectorCos(s)); }
template<class T> T Tan(T s) { return T(DirectX::XMVectorTan(s)); }
template<class T> T ASin(T s) { return T(DirectX::XMVectorASin(s)); }
template<class T> T ACos(T s) { return T(DirectX::XMVectorACos(s)); }
template<class T> T ATan(T s) { return T(DirectX::XMVectorATan(s)); }
template<class T> T ATan2(T y, T x) { return T(DirectX::XMVectorATan2(y, x)); }
template<class T> T Lerp(T a, T b, T t) { return T(DirectX::XMVectorLerpV(a, b, t)); }
template<class T> T Max(T a, T b) { return T(DirectX::XMVectorMax(a, b)); }
template<class T> T Min(T a, T b) { return T(DirectX::XMVectorMin(a, b)); }
template<class T> T Clamp(T v, T a, T b) { return Min(Max(v, a), b); }
template<class T> BoolVector operator<  (T lhs, T rhs) { return DirectX::XMVectorLess(lhs, rhs); }
template<class T> BoolVector operator<= (T lhs, T rhs) { return DirectX::XMVectorLessOrEqual(lhs, rhs); }
template<class T> BoolVector operator>  (T lhs, T rhs) { return DirectX::XMVectorGreater(lhs, rhs); }
template<class T> BoolVector operator>= (T lhs, T rhs) { return DirectX::XMVectorGreaterOrEqual(lhs, rhs); }
template<class T> BoolVector operator== (T lhs, T rhs) { return DirectX::XMVectorEqual(lhs, rhs); }
template<class T> T Select(T lhs, T rhs, BoolVector mask) { return T(DirectX::XMVectorSelect(lhs, rhs, mask)); }

INLINE float Sqrt(float s) { return Sqrt(Scalar(s)); }
INLINE float Recip(float s) { return Recip(Scalar(s)); }
INLINE float RecipSqrt(float s) { return RecipSqrt(Scalar(s)); }
INLINE float Floor(float s) { return Floor(Scalar(s)); }
INLINE float Ceiling(float s) { return Ceiling(Scalar(s)); }
INLINE float Round(float s) { return Round(Scalar(s)); }
INLINE float Abs(float s) { return s < 0.0f ? -s : s; }
INLINE float Exp(float s) { return Exp(Scalar(s)); }
INLINE float Pow(float b, float e) { return Pow(Scalar(b), Scalar(e)); }
INLINE float Log(float s) { return Log(Scalar(s)); }
INLINE float Sin(float s) { return Sin(Scalar(s)); }
INLINE float Cos(float s) { return Cos(Scalar(s)); }
INLINE float Tan(float s) { return Tan(Scalar(s)); }
INLINE float ASin(float s) { return ASin(Scalar(s)); }
INLINE float ACos(float s) { return ACos(Scalar(s)); }
INLINE float ATan(float s) { return ATan(Scalar(s)); }
INLINE float ATan2(float y, float x) { return ATan2(Scalar(y), Scalar(x)); }
INLINE float Lerp(float a, float b, float t) { return a + (b - a) * t; }
INLINE float Max(float a, float b) { return a > b ? a : b; }
INLINE float Min(float a, float b) { return a < b ? a : b; }
INLINE float Clamp(float v, float a, float b) { return Min(Max(v, a), b); }

}	// namespace Math



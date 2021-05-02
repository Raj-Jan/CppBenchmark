#include "benchmark.h"

const float sqrt2 = sqrtf(2);

struct Vector
{
	float x, y, z;

	Vector operator-()
	{
		return { -x, -y, -z };
	}
	Vector operator+(Vector other)
	{
		return { x + other.x, y + other.y, z + other.z };
	}
	Vector operator-(Vector other)
	{
		return { x - other.x, y - other.y, z - other.z };
	}

	Vector operator*(float other)
	{
		return { x * other, y * other, z * other };
	}
	Vector operator/(float other)
	{
		return { x / other, y / other, z / other };
	}

	Vector operator^(Vector other)
	{
		return { y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x };
	}
	float  operator|(Vector other)
	{
		return x * other.x + y * other.y + z * other.z;
	}
};

struct Matrix_
{
	static Matrix_ Create(Vector axis, float angle);

	float m00, m01, m02;
	float m10, m11, m12;
	float m20, m21, m22;

	Matrix_ operator~()
	{
		return
		{
			m00, m10, m20,
			m01, m11, m21,
			m02, m12, m22
		};
	}
	Matrix_ operator*(Matrix_ other)
	{
		return
		{
			m00 * other.m00 + m01 * other.m10 + m02 * other.m20,
			m00 * other.m01 + m01 * other.m11 + m02 * other.m21,
			m00 * other.m02 + m01 * other.m12 + m02 * other.m22,

			m10 * other.m00 + m11 * other.m10 + m12 * other.m20,
			m10 * other.m01 + m11 * other.m11 + m12 * other.m21,
			m10 * other.m02 + m11 * other.m12 + m12 * other.m22,

			m20 * other.m00 + m21 * other.m10 + m22 * other.m20,
			m20 * other.m01 + m21 * other.m11 + m22 * other.m21,
			m20 * other.m02 + m21 * other.m12 + m22 * other.m22
		};
	}
	Vector  operator*(Vector other)
	{
		return
		{
			m00 * other.x + m01 * other.y + m02 * other.z,
			m10 * other.x + m11 * other.y + m12 * other.z,
			m20 * other.x + m21 * other.y + m22 * other.z,
		};
	}
};

struct Quaternion_
{
	static Quaternion_ Create(Vector axis, float angle);

	Vector im;
	float re;

	Matrix_ ToMatrix()
	{
		float x = sqrt2 * im.x;
		float y = sqrt2 * im.y;
		float z = sqrt2 * im.z;
		float w = sqrt2 * re;

		float xx = x * x;
		float yy = y * y;
		float zz = z * z;
		float xy = x * y;
		float xz = x * z;
		float yz = y * z;
		float wx = w * x;
		float wy = w * y;
		float wz = w * z;

		return
		{
			1 - yy - zz, xy - wz, xz + wy,
			xy + wz, 1 - xx - zz, yz - wx,
			xz - wy, yz + wx, 1 - yy - zz
		};
	}

	Quaternion_ operator~()
	{
		return { -im, re };
	}
	Quaternion_ operator*(Quaternion_ other)
	{
		return
		{
			other.im * re + im * other.re + (im ^ other.im),
			re * other.re - (im | other.im)
		};
	}
	Vector      operator*(Vector other)
	{
		Vector cross = (im ^ other);

		return other + cross * (2 * re) + (im ^ cross) * 2;
	}
};

Quaternion_ Quaternion_::Create(Vector axis, float angle)
{
	angle *= 0.5f;

	float cos = std::cosf(angle);
	float sin = std::sinf(angle);

	return { axis * sin, cos };
}


class Test1
{
	Quaternion_ q1 = Quaternion_::Create({ 0, 0, 1 }, 1);
	Quaternion_ q2 = Quaternion_::Create({ 0, 1, 0 }, 1);

public:
	void operator ()()
	{
		q1 = q1 * q2;
	}
};

class Test2 
{
	Matrix_ q1 = Quaternion_::Create({ 0, 0, 1 }, 1).ToMatrix();
	Matrix_ q2 = Quaternion_::Create({ 0, 1, 0 }, 1).ToMatrix();

public:
	void operator ()()
	{
		q1 = q1 * q2;
	}
};

int main()
{
	// test time of multiplication operator on quaternions and matrices
	BenchmarkAuto<Test1>().PrintNS();
	BenchmarkAuto<Test2>().PrintNS();
}
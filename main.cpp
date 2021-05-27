#include "benchmark.h"

const float sqrt2 = sqrtf(2);

struct Vector
{
	float x, y, z;

	Vector() { }
	Vector(float x, float y, float z) : x (x), y(y), z(z) { }

	Vector operator-() const
	{
		return { -x, -y, -z };
	}
	Vector operator+(const Vector other) const
	{
		return { x + other.x, y + other.y, z + other.z };
	}
	Vector operator-(const Vector other) const
	{
		return { x - other.x, y - other.y, z - other.z };
	}

	Vector operator*(float other) const
	{
		return { x * other, y * other, z * other };
	}
	Vector operator/(float other) const
	{
		return { x / other, y / other, z / other };
	}

	Vector operator^(const Vector other) const
	{
		return { y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x };
	}
	float  operator|(const Vector other) const
	{
		return x * other.x + y * other.y + z * other.z;
	}
};

struct Matrix_
{
	float m00, m01, m02;
	float m10, m11, m12;
	float m20, m21, m22;

	Matrix_ operator~() const
	{
		return
		{
			m00, m10, m20,
			m01, m11, m21,
			m02, m12, m22
		};
	}
	Matrix_ operator*(Matrix_ other) const
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
	Vector  operator*(Vector other) const
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

	Quaternion_ operator~() const
	{
		return { -im, re };
	}
	Quaternion_ operator*(Quaternion_ other) const
	{
		return
		{
			other.im * re + im * other.re + (im ^ other.im),
			re * other.re - (im | other.im)
		};
	}
	Vector      operator*(Vector other) const
	{
		Vector cross = (im ^ other);

		return other + cross * (2 * re) + (im ^ cross) * 2;
	}
};

struct AxisAngle
{
	Vector axisangle;

	AxisAngle(Vector axis, float angle = 1)
	{
		axisangle =  axis * angle;
	}

	Matrix_ ToMatrix()
	{
		float angle = sqrtf(axisangle | axisangle);
		Vector axis = axis / angle;

		return Quaternion_::Create(axis, angle).ToMatrix();
	}

	AxisAngle operator*(const AxisAngle& other) const
	{
		return { axisangle + other.axisangle + (axisangle ^ other.axisangle) / 2 };
	}
	Vector      operator*(Vector other) const
	{
		float angle = sqrtf(axisangle | axisangle);
		Vector axis = axis / angle;

		Quaternion_ q = Quaternion_::Create(axis, angle);

		return q * other;
	}
};

Quaternion_ Quaternion_::Create(Vector axis, float angle)
{
	angle *= 0.5f;

	float cos = std::cosf(angle);
	float sin = std::sinf(angle);

	return { axis * sin, cos };
}

struct Test1
{
	Quaternion_ q1 = Quaternion_::Create({ 0, 0, 1 }, 1);
	Quaternion_ q2 = Quaternion_::Create({ 0, 1, 0 }, 1);

public:
	void operator ()()
	{
		q1 = q1 * q2;
	}
};

struct Test2
{
	Matrix_ q1 = Quaternion_::Create({ 0, 0, 1 }, 1).ToMatrix();
	Matrix_ q2 = Quaternion_::Create({ 0, 1, 0 }, 1).ToMatrix();

public:
	void operator ()()
	{
		q1 = q1 * q2;
	}
};

struct Test3
{
	AxisAngle q1 = AxisAngle({ 0, 0, 1 }, 1);
	AxisAngle q2 = AxisAngle({ 0, 1, 0 }, 1);

public:
	void operator ()()
	{
		q1 = q1 * q2;
	}
};

#include <stdlib.h>

class Test_1
{
	int ptr = rand();
	int result;

public:
	void operator()()
	{
		if (ptr)
		{
			result++;
		}
		else
		{
			result--;
		}
	}
};

class Test_2
{
	uint64_t ptr = rand();
	int result;

public:
	void operator()()
	{
		if (ptr)
		{
			result++;
		}
		else
		{
			result--;
		}
	}
};

class Test_3
{
	void* ptr;

	int result;

public:
	Test_3()
	{
		int x = rand();

		ptr = reinterpret_cast<void*>(x);
	}

	void operator()()
	{
		if (ptr)
		{
			result++;
		}
		else
		{
			result--;
		}
	}
};

class ID
{
	static int key;

public:
	template<typename T> static int value()
	{
		const static int id = key++;

		return id;
	}
};

#include <unordered_map>
#include <vector>



class StackLeft final
{
	void* top;

public:
	StackLeft()
	{
		top = &top + 1;
	}
	~StackLeft() = default;

	template<typename T> T* const alloc()
	{
		T* const result = (T*)top;

		top = (char*)top + sizeof(T);

		return result;
	}
	template<typename T> void free(T* const obj)
	{
		top = (char*)top - sizeof(T);
	}

	size_t size()
	{
		return (char*)top - (char*)&top;
	}
};

struct Foo
{
	int x;

	Foo()
	{
		std::cout << x << " constructor Foo\n";
	}
	~Foo()
	{
		std::cout << x << " destructor Foo\n";
	}
};

struct Memory
{
	StackLeft allocator;
	char memory[1024];
};

Memory memory;

template<typename T>
struct Pointer
{
	T* const value = memory.allocator.alloc<T>();

public:
	Pointer()
	{
		new (value) T();
	}
	~Pointer()
	{
		value->~T();
		memory.allocator.free(value);
	}

	T* const operator ()() const
	{
		return value;
	}
};

struct Tet_0
{
	void operator()()
	{
		int* ptr = new int();
		delete ptr;
	}
};

struct Tet_1
{
	void operator()()
	{
		Pointer<int> ptr;
	}
};

struct Tet_2
{
	void operator()()
	{
		int* const ptr = memory.allocator.alloc<int>();

		new (ptr) int();

		memory.allocator.free(ptr);
	}
};

int foo(int x)
{
	return x++;
}

class Tut_1
{
	int result;
	int limit;

	std::unordered_map<int, int(*)(int)> map;

public:
	Tut_1()
	{
		int result = rand();
		int limit = rand();

		map.emplace(0, &foo);
	}

	void operator()()
	{
		if (limit == 0)
			result = map[0](result);
	}
};

class Tut_2
{
	int result;
	int limit;

public:
	Tut_2()
	{
		int result = rand();
		int limit = rand();
	}

	void operator()()
	{
		switch (limit)
		{
		case 0:
			result++; break;
		default:
			return;
		}
	}
};

int main()
{
    //test time of multiplication operator on quaternions and matrices
	//Benchmark<Test1>();
	//Benchmark<Test2>();
	//Benchmark<Test3>();

	//Benchmark<Test_1>();
	//Benchmark<Test_2>();
	//Benchmark<Test_3>();

	Benchmark<Tet_0>();
	Benchmark<Tet_1>();
	Benchmark<Tet_2>();

	//Benchmark<Tut_1>();
	//Benchmark<Tut_2>();
}
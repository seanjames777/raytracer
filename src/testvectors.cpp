#include "math/vector.h"
#include <cassert>

void testVectors() {
	// 2D float vector
	{
		float2 x = float2(2.0f, 3.0f);
		float2 y = float2(4.0f, 5.0f);
		float2 z = min(x, y);
		assert(z.x == 2.0f && z.y == 3.0f);
	}
	{
		float2 x = float2(2.0f, 3.0f);
		float2 y = float2(4.0f, 5.0f);
		float2 z = max(x, y);
		assert(z.x == 4.0f && z.y == 5.0f);
	}
	{
		float2 x = float2(2.0f, 3.0f);
		float2 y = float2(4.0f, 5.0f);
		float2 z = x + y;
		assert(z.x == 6.0f && z.y == 8.0f);
	}
	{
		float2 x = float2(2.0f, 3.0f);
		float2 y = float2(4.0f, 5.0f);
		float2 z = x - y;
		assert(z.x == -2.0f && z.y == -2.0f);
	}
	{
		float2 x = float2(2.0f, 3.0f);
		float2 z = -x;
		assert(z.x == -2.0f && z.y == -3.0f);
	}
	{
		float2 x = float2(2.0f, 3.0f);
		float2 y = float2(4.0f, 5.0f);
		float2 z = x * y;
		assert(z.x == 8.0f && z.y == 15.0f);
	}
	{
		float2 x = float2(2.0f, 3.0f);
		float2 y = float2(4.0f, 5.0f);
		float2 z = x / y;
		assert(z.x == (2.0f / 4.0f) && z.y == (3.0f / 5.0f));
	}
	{
		float2 x = float2(2.0f, 3.0f);
		float2 y = float2(4.0f, 5.0f);
		float z = dot(x, y);
		assert(z == (2.0f * 4.0f) + (3.0f * 5.0f));
	}
	{
		float2 x = float2(2.0f, 3.0f);
		float z = length(x);
		assert(z == sqrt((2.0f * 2.0f) + (3.0f * 3.0f)));
	}
	{
		float2 x = float2(2.0f, 3.0f);
		float2 z = normalize(x);

		float l = sqrt((2.0f * 2.0f) + (3.0f * 3.0f));
		assert(z.x == 2.0f / l && z.y == 3.0f / l);
	}
	{
		float2 x = float2(2.0f, 3.0f);
		float2 y = float2(4.0f, 5.0f);
		bmask2 z = x < y;
		assert(z[0] == 0xFFFFFFFF && z[1] == 0xFFFFFFFF);
	}
	{
		float2 x = float2(2.0f, 3.0f);
		float2 y = float2(4.0f, 5.0f);
		bmask2 z = x <= y;
		assert(z[0] == 0xFFFFFFFF && z[1] == 0xFFFFFFFF);
	}
	{
		float2 x = float2(2.0f, 3.0f);
		float2 y = float2(4.0f, 5.0f);
		bmask2 z = x == y;
		assert(z[0] == 0x00000000 && z[1] == 0x00000000);
	}
	{
		float2 x = float2(2.0f, 3.0f);
		float2 y = float2(4.0f, 5.0f);
		bmask2 z = x != y;
		assert(z[0] == 0xFFFFFFFF && z[1] == 0xFFFFFFFF);
	}
	{
		float2 x = float2(2.0f, 3.0f);
		float2 y = float2(4.0f, 5.0f);
		bmask2 z = x >= y;
		assert(z[0] == 0x00000000 && z[1] == 0x00000000);
	}
	{
		float2 x = float2(2.0f, 3.0f);
		float2 y = float2(4.0f, 5.0f);
		bmask2 z = x > y;
		assert(z[0] == 0x00000000 && z[1] == 0x00000000);
	}
	{
		float2 x = float2(2.0f, 3.0f);
		float2 y = shuffle<2, 0, 0, 0, 0>(x);
		float2 z = shuffle<2, 1, 1, 1, 1>(x);

		assert(y.x == 2.0f && y.y == 2.0f && z.x == 3.0f && z.y == 3.0f);
	}

	// 3D float vector
	{
		float3 x = float3(2.0f, 3.0f, 4.0f);
		float3 y = float3(5.0f, 6.0f, 7.0f);
		float3 z = min(x, y);
		assert(z.x == 2.0f && z.y == 3.0f && z.z == 4.0f);
	}
	{
		float3 x = float3(2.0f, 3.0f, 4.0f);
		float3 y = float3(5.0f, 6.0f, 7.0f);
		float3 z = max(x, y);
		assert(z.x == 5.0f && z.y == 6.0f && z.z == 7.0f);
	}
	{
		float3 x = float3(2.0f, 3.0f, 4.0f);
		float3 y = float3(5.0f, 6.0f, 7.0f);
		float3 z = x + y;
		assert(z.x == 7.0f && z.y == 9.0f && z.z == 11.0f);
	}
	{
		float3 x = float3(2.0f, 3.0f, 4.0f);
		float3 y = float3(5.0f, 6.0f, 7.0f);
		float3 z = x - y;
		assert(z.x == -3.0f && z.y == -3.0f && z.z == -3.0f);
	}
	{
		float3 x = float3(2.0f, 3.0f, 4.0f);
		float3 z = -x;
		assert(z.x == -2.0f && z.y == -3.0f && z.z == -4.0f);
	}
	{
		float3 x = float3(2.0f, 3.0f, 4.0f);
		float3 y = float3(5.0f, 6.0f, 7.0f);
		float3 z = x * y;
		assert(z.x == 10.0f && z.y == 18.0f && z.z == 28.0f);
	}
	{
		float3 x = float3(2.0f, 3.0f, 4.0f);
		float3 y = float3(5.0f, 6.0f, 7.0f);
		float3 z = x / y;
		assert(z.x == (2.0f / 5.0f) && z.y == (3.0f / 6.0f) && z.z == (4.0f / 7.0f));
	}
	{
		float3 x = float3(2.0f, 3.0f, 4.0f);
		float3 y = float3(5.0f, 6.0f, 7.0f);
		float z = dot(x, y);
		assert(z == (2.0f * 5.0f) + (3.0f * 6.0f) + (4.0f * 7.0f));
	}
	{
		float3 x = float3(2.0f, 3.0f, 4.0f);
		float z = length(x);
		assert(z == sqrt((2.0f * 2.0f) + (3.0f * 3.0f) + (4.0f * 4.0f)));
	}
	{
		float3 x = float3(2.0f, 3.0f, 4.0f);
		float3 z = normalize(x);

		float l = sqrt((2.0f * 2.0f) + (3.0f * 3.0f) + (4.0f * 4.0f));
		assert(z.x == 2.0f / l && z.y == 3.0f / l && z.z == 4.0f / l);
	}
	{
		float3 x = float3(2.0f, 3.0f, 4.0f);
		float3 y = float3(5.0f, 6.0f, 7.0f);
		bmask3 z = x < y;
		assert(z[0] == 0xFFFFFFFF && z[1] == 0xFFFFFFFF && z[2] == 0xFFFFFFFF);
	}
	{
		float3 x = float3(2.0f, 3.0f, 4.0f);
		float3 y = float3(5.0f, 6.0f, 7.0f);
		bmask3 z = x <= y;
		assert(z[0] == 0xFFFFFFFF && z[1] == 0xFFFFFFFF && z[2] == 0xFFFFFFFF);
	}
	{
		float3 x = float3(2.0f, 3.0f, 4.0f);
		float3 y = float3(5.0f, 6.0f, 7.0f);
		bmask3 z = x == y;
		assert(z[0] == 0x00000000 && z[1] == 0x00000000 && z[2] == 0x00000000);
	}
	{
		float3 x = float3(2.0f, 3.0f, 4.0f);
		float3 y = float3(5.0f, 6.0f, 7.0f);
		bmask3 z = x != y;
		assert(z[0] == 0xFFFFFFFF && z[1] == 0xFFFFFFFF && z[2] == 0xFFFFFFFF);
	}
	{
		float3 x = float3(2.0f, 3.0f, 4.0f);
		float3 y = float3(5.0f, 6.0f, 7.0f);
		bmask3 z = x >= y;
		assert(z[0] == 0x00000000 && z[1] == 0x00000000 && z[2] == 0x00000000);
	}
	{
		float3 x = float3(2.0f, 3.0f, 4.0f);
		float3 y = float3(5.0f, 6.0f, 7.0f);
		bmask3 z = x > y;
		assert(z[0] == 0x00000000 && z[1] == 0x00000000 && z[2] == 0x00000000);
	}

	// 4D float vector
	{
		float4 x = float4(2.0f, 3.0f, 4.0f, 5.0f);
		float4 y = float4(6.0f, 7.0f, 8.0f, 9.0f);
		float4 z = min(x, y);
		assert(z.x == 2.0f && z.y == 3.0f && z.z == 4.0f && z.w == 5.0f);
	}
	{
		float4 x = float4(2.0f, 3.0f, 4.0f, 5.0f);
		float4 y = float4(6.0f, 7.0f, 8.0f, 9.0f);
		float4 z = max(x, y);
		assert(z.x == 6.0f && z.y == 7.0f && z.z == 8.0f && z.w == 9.0f);
	}
	{
		float4 x = float4(2.0f, 3.0f, 4.0f, 5.0f);
		float4 y = float4(6.0f, 7.0f, 8.0f, 9.0f);
		float4 z = x + y;
		assert(z.x == 8.0f && z.y == 10.0f && z.z == 12.0f && z.w == 14.0f);
	}
	{
		float4 x = float4(2.0f, 3.0f, 4.0f, 5.0f);
		float4 y = float4(6.0f, 7.0f, 8.0f, 9.0f);
		float4 z = x - y;
		assert(z.x == -4.0f && z.y == -4.0f && z.z == -4.0f && z.w == -4.0f);
	}
	{
		float4 x = float4(2.0f, 3.0f, 4.0f, 5.0f);
		float4 z = -x;
		assert(z.x == -2.0f && z.y == -3.0f && z.z == -4.0f && z.w == -5.0f);
	}
	{
		float4 x = float4(2.0f, 3.0f, 4.0f, 5.0f);
		float4 y = float4(6.0f, 7.0f, 8.0f, 9.0f);
		float4 z = x * y;
		assert(z.x == 12.0f && z.y == 21.0f && z.z == 32.0f && z.w == 45.0f);
	}
	{
		float4 x = float4(2.0f, 3.0f, 4.0f, 5.0f);
		float4 y = float4(6.0f, 7.0f, 8.0f, 9.0f);
		float4 z = x / y;
		assert(z.x == (2.0f / 6.0f) && z.y == (3.0f / 7.0f) && z.z == (4.0f / 8.0f) && z.w == (5.0f / 9.0f));
	}
	{
		float4 x = float4(2.0f, 3.0f, 4.0f, 5.0f);
		float4 y = float4(6.0f, 7.0f, 8.0f, 9.0f);
		float z = dot(x, y);
		assert(z == (2.0f * 6.0f) + (3.0f * 7.0f) + (4.0f * 8.0f) + (5.0f * 9.0f));
	}
	{
		float4 x = float4(2.0f, 3.0f, 4.0f, 5.0f);
		float z = length(x);
		assert(z == sqrt((2.0f * 2.0f) + (3.0f * 3.0f) + (4.0f * 4.0f) + (5.0f * 5.0f)));
	}
	{
		float4 x = float4(2.0f, 3.0f, 4.0f, 5.0f);
		float4 z = normalize(x);

		float l = sqrt((2.0f * 2.0f) + (3.0f * 3.0f) + (4.0f * 4.0f) + (5.0f * 5.0f));
		assert(z.x == 2.0f / l && z.y == 3.0f / l && z.z == 4.0f / l && z.w == 5.0f / l);
	}
	{
		float4 x = float4(2.0f, 3.0f, 4.0f, 5.0f);
		float4 y = float4(6.0f, 7.0f, 8.0f, 9.0f);
		bmask4 z = x < y;
		assert(z[0] == 0xFFFFFFFF && z[1] == 0xFFFFFFFF && z[2] == 0xFFFFFFFF && z[3] == 0xFFFFFFFF);
	}
	{
		float4 x = float4(2.0f, 3.0f, 4.0f, 5.0f);
		float4 y = float4(6.0f, 7.0f, 8.0f, 9.0f);
		bmask4 z = x <= y;
		assert(z[0] == 0xFFFFFFFF && z[1] == 0xFFFFFFFF && z[2] == 0xFFFFFFFF && z[3] == 0xFFFFFFFF);
	}
	{
		float4 x = float4(2.0f, 3.0f, 4.0f, 5.0f);
		float4 y = float4(6.0f, 7.0f, 8.0f, 9.0f);
		bmask4 z = x == y;
		assert(z[0] == 0x00000000 && z[1] == 0x00000000 && z[2] == 0x00000000 && z[3] == 0x00000000);
	}
	{
		float4 x = float4(2.0f, 3.0f, 4.0f, 5.0f);
		float4 y = float4(6.0f, 7.0f, 8.0f, 9.0f);
		bmask4 z = x != y;
		assert(z[0] == 0xFFFFFFFF && z[1] == 0xFFFFFFFF && z[2] == 0xFFFFFFFF && z[3] == 0xFFFFFFFF);
	}
	{
		float4 x = float4(2.0f, 3.0f, 4.0f, 5.0f);
		float4 y = float4(6.0f, 7.0f, 8.0f, 9.0f);
		bmask4 z = x >= y;
		assert(z[0] == 0x00000000 && z[1] == 0x00000000 && z[2] == 0x00000000 && z[3] == 0x00000000);
	}
	{
		float4 x = float4(2.0f, 3.0f, 4.0f, 5.0f);
		float4 y = float4(6.0f, 7.0f, 8.0f, 9.0f);
		bmask4 z = x > y;
		assert(z[0] == 0x00000000 && z[1] == 0x00000000 && z[2] == 0x00000000 && z[3] == 0x00000000);
	}

	// 2D bitmask
	{
		bmask2 y = bmask2(0xFFFF0000);
		bmask2 z = ~y;

		assert(z[0] == 0x0000FFFF && z[1] == 0x0000FFFF);
	}
	{
		bmask2 x = bmask2(0x00000010);
		bmask2 y = bmask2(0x00000001);

		bmask2 z = x | y;

		assert(z[0] == 0x00000011 && z[1] == 0x00000011);
	}
	{
		bmask2 x = bmask2(0x00000110);
		bmask2 y = bmask2(0x00000011);

		bmask2 z = x & y;

		assert(z[0] == 0x00000010 && z[1] == 0x00000010);
	}
	{
		bmask2 x = bmask2(0x00000000, 0xFFFFFFFF);
		float2 y = float2(1.0f);
		float2 z = float2(2.0f);

		float2 w = blend(x, y, z);

		assert(w.x == 1.0f && w.y == 2.0f);
	}

	{
		bmask4 x = bmask4(0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF);
		int4 y = int4(1);
		int4 z = int4(2);

		int4 w = blend(x, y, z);

		assert(w.x == 1 && w.y == 2 && w.z == 1 && w.w == 2);
	}

	{
		bmask4 x = bmask4(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
		bmask4 y = bmask4(0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000);
		bmask4 z = bmask4(0x00000000, 0x00000000, 0x00000000, 0x00000000);

		assert(any(x));
		assert(any(y));
		assert(!any(z));
		assert(all(x));
		assert(!all(y));
		assert(!all(z));
	}
}
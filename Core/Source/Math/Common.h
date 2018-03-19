//
// Common functions for mathamatic utility.
//

#pragma once

#include <DirectXMath.h>
#include <intrin.h>

#define INLINE __forceinline

namespace Math
{
	template <typename T> INLINE T AlignUpWithMask(T value, size_t mask)
	{
		return (T)(((size_t)value + mask) & ~mask);
	}

	template <typename T> INLINE T AlignDownWithMask(T value, size_t mask)
	{
		return (T)((size_t)value & ~mask);
	}

	template <typename T> INLINE T AlignUp(T value, size_t alignment)
	{
		return AlignUpWithMask(value, alignment - 1);
	}

	template <typename T> INLINE T AlignDown(T value, size_t alignment)
	{
		return AlignDownWithMask(value, alignment - 1);
	}

	template <typename T> INLINE bool IsAligned(T value, size_t alignment)
	{
		return 0 == ((size_t)value & (alignment - 1));
	}

	template <typename T> INLINE T DivideByMultiple(T value, size_t alignment)
	{
		return (T)((value + alignment - 1) / alignment);
	}

	template <typename T> INLINE bool IsPowerOfTwo(T value)
	{
		return 0 == (value & (value - 1));
	}

	template <typename T> INLINE bool IsDivisible(T value, T divisor)
	{
		return (value / divisor) * divisor == value;
	}

	INLINE uint8_t Log2(uint64_t value)
	{
		unsigned long mssb; // most significant set bit
		unsigned long lssb; // least significant set bit

		// If perfect power of two (only one set bit), return index of bit.  Otherwise round up
		// fractional log by adding 1 to most signicant set bit's index.
		if (_BitScanReverse64(&mssb, value) > 0 && _BitScanForward64(&lssb, value) > 0)
			return uint8_t(mssb + (mssb == lssb ? 0 : 1));
		else
			return 0;
	}

	template <typename T> INLINE T AlignPowerOfTwo(T value)
	{
		return value == 0 ? 0 : 1 << Log2(value);
	}

	INLINE DirectX::XMVECTOR SplatZero()
	{
		return DirectX::XMVectorZero();
	}

#if !defined(_XM_NO_INTRINSICS_) && defined(_XM_SSE_INTRINSICS_)

	INLINE DirectX::XMVECTOR SplatOne(DirectX::XMVECTOR zero = SplatZero())
	{
		__m128i AllBits = _mm_castps_si128(_mm_cmpeq_ps(zero, zero));
		return _mm_castsi128_ps(_mm_slli_epi32(_mm_srli_epi32(AllBits, 25), 23));	// return 0x3F800000
		//return _mm_cvtepi32_ps(_mm_srli_epi32(SetAllBits(zero), 31));				// return (float)1;  (alternate method)
	}

#if defined(_XM_SSE4_INTRINSICS_)
	INLINE DirectX::XMVECTOR CreateXUnitVector(DirectX::XMVECTOR one = SplatOne())
	{
		return _mm_insert_ps(one, one, 0x0E);
	}
	INLINE DirectX::XMVECTOR CreateYUnitVector(DirectX::XMVECTOR one = SplatOne())
	{
		return _mm_insert_ps(one, one, 0x0D);
	}
	INLINE DirectX::XMVECTOR CreateZUnitVector(DirectX::XMVECTOR one = SplatOne())
	{
		return _mm_insert_ps(one, one, 0x0B);
	}
	INLINE DirectX::XMVECTOR CreateWUnitVector(DirectX::XMVECTOR one = SplatOne())
	{
		return _mm_insert_ps(one, one, 0x07);
	}
	INLINE DirectX::XMVECTOR SetWToZero(DirectX::FXMVECTOR vec)
	{
		return _mm_insert_ps(vec, vec, 0x08);
	}
	INLINE DirectX::XMVECTOR SetWToOne(DirectX::FXMVECTOR vec)
	{
		return _mm_blend_ps(vec, SplatOne(), 0x8);
	}
#else
	INLINE DirectX::XMVECTOR CreateXUnitVector(DirectX::XMVECTOR one = SplatOne())
	{
		return _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(one), 12));
	}
	INLINE DirectX::XMVECTOR CreateYUnitVector(DirectX::XMVECTOR one = SplatOne())
	{
		DirectX::XMVECTOR unitx = CreateXUnitVector(one);
		return _mm_castsi128_ps(_mm_slli_si128(_mm_castps_si128(unitx), 4));
	}
	INLINE DirectX::XMVECTOR CreateZUnitVector(DirectX::XMVECTOR one = SplatOne())
	{
		DirectX::XMVECTOR unitx = CreateXUnitVector(one);
		return _mm_castsi128_ps(_mm_slli_si128(_mm_castps_si128(unitx), 8));
	}
	INLINE DirectX::XMVECTOR CreateWUnitVector(DirectX::XMVECTOR one = SplatOne())
	{
		return _mm_castsi128_ps(_mm_slli_si128(_mm_castps_si128(one), 12));
	}
	INLINE DirectX::XMVECTOR SetWToZero(DirectX::FXMVECTOR vec)
	{
		__m128i MaskOffW = _mm_srli_si128(_mm_castps_si128(_mm_cmpeq_ps(vec, vec)), 4);
		return _mm_and_ps(vec, _mm_castsi128_ps(MaskOffW));
	}
	INLINE DirectX::XMVECTOR SetWToOne(DirectX::FXMVECTOR vec)
	{
		return _mm_movelh_ps(vec, _mm_unpackhi_ps(vec, SplatOne()));
	}
#endif

#else // !_XM_SSE_INTRINSICS_

	INLINE DirectX::XMVECTOR SplatOne() { return XMVectorSplatOne(); }
	INLINE DirectX::XMVECTOR CreateXUnitVector() { return g_XMIdentityR0; }
	INLINE DirectX::XMVECTOR CreateYUnitVector() { return g_XMIdentityR1; }
	INLINE DirectX::XMVECTOR CreateZUnitVector() { return g_XMIdentityR2; }
	INLINE DirectX::XMVECTOR CreateWUnitVector() { return g_XMIdentityR3; }
	INLINE DirectX::XMVECTOR SetWToZero(DirectX::FXMVECTOR vec) { return XMVectorAndInt(vec, g_XMMask3); }
	INLINE DirectX::XMVECTOR SetWToOne(DirectX::FXMVECTOR vec) { return XMVectorSelect(g_XMIdentityR3, vec, g_XMMask3); }

#endif

	enum class EZeroTag { kZero, kOrigin };
	enum class EIdentityTag { kOne, kIdentity };
	enum class EXUnitVector { kXUnitVector };
	enum class EYUnitVector { kYUnitVector };
	enum class EZUnitVector { kZUnitVector };
	enum class EWUnitVector { kWUnitVector };

}	// namespace Math
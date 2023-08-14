/* Copyright (c) <2003-2022> <Julio Jerez, Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef __ND_UTILS_H__
#define __ND_UTILS_H__

#include "ndCoreStdafx.h"
#include "ndTypes.h"
#include "ndStack.h"
#include "ndMemory.h"
#include "ndFixSizeArray.h"

// assume this function returns memory aligned to 16 bytes
#define ndAlloca(type, count) (type*) alloca (sizeof (type) * (count))

inline ndInt32 ndExp2 (ndInt32 x)
{
	ndInt32 exp = -1;
	for (; x; x >>= 1) 
	{
		exp ++;
	}
	return exp;
}

inline ndInt32 ndBitReversal(ndInt32 v, ndInt32 base)
{
	ndInt32 x = 0;
	ndInt32 power = ndExp2 (base) - 1;
	do 
	{
		x += (v & 1) << power;
		v >>= 1;
		power--;
	} while (v);
	ndAssert(x < base);
	return x;
}

template <class T>
inline T ndMod(T val, T mod)
{
	return T(fmod(T(val), T(mod)));
}

template <class T> 
inline T ndMin(T A, T B)
{
	return (A < B) ? A : B; 
}

template <class T> 
inline T ndMax(T A, T B)
{
	return (A > B) ? A : B; 
}

template <class T>
inline T ndClamp(T val, T min, T max)
{
	return ndMax (min, ndMin (max, val));
}

template <class T> 
inline void ndSwap(T& A, T& B)
{
	T tmp (A);
	A = B;
	B = tmp;
}	

template <class T>
inline T ndAbs(T A)
{
	// according to Intel this is better because is does not read after write
	return (A >= T(0)) ? A : -A;
}

template <class T>
inline T ndSign(T A)
{
	return (A >= T(0)) ? T(1) : T(-1);
}

template <class T> 
inline bool ndAreEqual(T A, T B, T tol)
{
	// deal with too small and de normal values.
	if ((ndAbs(A) < tol) && (ndAbs(B) < tol)) 
	{
		return true;
	}

	ndInt32 exp0;
	ndFloat64 mantissa0 = frexp(ndFloat64 (A), &exp0);
	
	ndInt32 exp1;
	ndFloat64 mantissa1 = frexp(ndFloat64(B), &exp1);
	if (ndAbs(exp0 - exp1) > 1)
	{
		return false;
	}
	if (exp0 != exp1)
	{
		if (exp0 > exp1)
		{
			mantissa0 *= ndFloat32(2.0f);
		}
		else
		{
			mantissa1 *= ndFloat32(2.0f);
		}
	}
	return ndAbs(mantissa0 - mantissa1) < tol;
}

/// add two angles in a periodic way
template <class T>
inline T ndAnglesAdd (T angleInRadiand1, T angleInRadiand0)
{
	T s1 = T(ndSin(angleInRadiand1));
	T c1 = T(ndCos(angleInRadiand1));
	T s0 = T(ndSin(angleInRadiand0));
	T c0 = T(ndCos(angleInRadiand0));

	T s = s1 * c0 + s0 * c1;
	T c = c1 * c0 - s0 * s1;
	return T(ndAtan2(s, c));
}

template <class T>
inline T ndAnglesSub(T angleInRadiand1, T angleInRadiand0)
{
	T s1 = T(ndSin(angleInRadiand1));
	T c1 = T(ndCos(angleInRadiand1));
	T s0 = T(ndSin(angleInRadiand0));
	T c0 = T(ndCos(angleInRadiand0));

	T s = s1 * c0 - s0 * c1;
	T c = c1 * c0 + s0 * s1;
	return T(ndAtan2(s, c));
}

template <class T>
inline void ndMemSet(T* const dst, const T& val, ndInt32 elements)
{
	T value(val);
	const ndInt32 n = elements & (-0x04);
	for (ndInt32 i = 0; i < n; i += 4)
	{
		dst[i + 0] = value;
		dst[i + 1] = value;
		dst[i + 2] = value;
		dst[i + 3] = value;
	}
	for (ndInt32 i = n; i < elements; ++i)
	{
		dst[i] = value;
	}
}

template <class T>
inline void ndMemCpy(T* const dst, const T* const src, ndInt32 elements)
{
	const ndInt32 n = elements & (-0x04);
	for (ndInt32 i = 0; i < n; i += 4)
	{
		dst[i + 0] = src[i + 0];
		dst[i + 1] = src[i + 1];
		dst[i + 2] = src[i + 2];
		dst[i + 3] = src[i + 3];
	}
	for (ndInt32 i = n; i < elements; ++i)
	{
		dst[i] = src[i];
	}
}

#ifndef _MSC_VER 
	#define _stricmp(x,y) strcasecmp(x,y)
#endif

inline void strtolwr(char* const string)
{
	for (char * cp = string; *cp; ++cp)
	{
		if ((*cp >= 'A') && (*cp <= 'Z'))
		{
			*cp += 'a' - 'A';
		}
	}
}

/// Returns the time in micro seconds since application started 
D_CORE_API ndUnsigned64 ndGetTimeInMicroseconds();

/// Round a 64 bit float to a 32 bit float by truncating the mantissa to 24 bits 
/// \param ndFloat64 val: 64 bit float 
/// \return a 64 bit double precision with a 32 bit mantissa
D_CORE_API ndFloat64 ndRoundToFloat(ndFloat64 val);

/// tell the operating system the library is done with this job, 
/// the OS is free to swith to another task if it needs to.
D_CORE_API void ndThreadYield();

/// removed all duplicate points from an array and place the location in the index array
D_CORE_API ndInt32 ndVertexListToIndexList(ndFloat64* const vertexList, ndInt32 strideInBytes, ndInt32 compareCount, ndInt32 vertexCount, ndInt32* const indexListOut, ndFloat64 tolerance = ndEpsilon);

/// removed all duplicate points from an array and place the location in the index array
template <class T>
ndInt32 ndVertexListToIndexList(T* const vertexList, ndInt32 strideInBytes, ndInt32 compareCount, ndInt32 vertexCount, ndInt32* const indexListOut, T tolerance = ndEpsilon)
{
	ndInt32 stride = ndInt32(strideInBytes / sizeof(T));
	ndStack<ndFloat64> pool(vertexCount * stride);

	ndFloat64* const data = &pool[0];
	for (ndInt32 i = 0; i < vertexCount; ++i)
	{
		ndFloat64* const dst = &data[i * stride];
		const T* const src = &vertexList[i * stride];
		for (ndInt32 j = 0; j < stride; ++j)
		{
			dst[j] = src[j];
		}
	}

	ndInt32 count = ndVertexListToIndexList(data, ndInt32(stride * sizeof(ndFloat64)), compareCount, vertexCount, indexListOut, ndFloat64(tolerance));
	for (ndInt32 i = 0; i < count; ++i)
	{
		const ndFloat64* const src = &data[i * stride];
		T* const dst = &vertexList[i * stride];
		for (ndInt32 j = 0; j < stride; ++j)
		{
			dst[j] = T(src[j]);
		}
	}

	return count;
}

/// Simple moving average class, useful for stuff like frame rate smoothing
template <ndInt32 size>
class ndMovingAverage: public ndFixSizeArray<ndReal, size>
{
	public:
	ndMovingAverage()
		:ndFixSizeArray<ndReal, size>()
		,m_average(ndReal(0.0f))
		,m_index(0)
	{
		Clear();
	}

	ndReal GetAverage() const
	{
		return m_average;
	}

	void Clear()
	{
		m_index = 0;
		m_average = ndReal(0.0f);
		ndFixSizeArray<ndReal, size>::SetCount(0);
		for (ndInt32 i = 0; i < size; ++i)
		{
			ndFixSizeArray<ndReal, size>::PushBack(ndReal(0.0f));
		}
	}

	ndReal Update(ndReal value)
	{
		(*this)[m_index] = value;
		ndInt32 index = (m_index + 1) % size;
		m_average += (value - (*this)[index]) / size;
		m_index = index;
		return GetAverage();
	}

	private:
	ndReal m_average;
	ndInt32 m_index;
};

/// Simple spin lock for synchronizing threads for very short period of time.
class ndSpinLock
{
	public:
	ndSpinLock()
	#ifndef D_USE_THREAD_EMULATION	
		:m_lock(0)
	#endif
	{
	}

	void Lock()
	{
		#ifndef D_USE_THREAD_EMULATION	
			ndInt32 exp = 1;
			for (ndUnsigned32 test = 0; !m_lock.compare_exchange_weak(test, 1); test = 0)
			{
				Delay(exp);
			}
		#endif
	}

	void Unlock()
	{
		#ifndef D_USE_THREAD_EMULATION	
			m_lock.store(0);
		#endif
	}

	#ifndef D_USE_THREAD_EMULATION	
		private:
		D_CORE_API void Delay(ndInt32& exp);
		ndAtomic<ndUnsigned32> m_lock;
	#endif
};

/// Simple scope based spin lock.
class ndScopeSpinLock
{
	public:
	ndScopeSpinLock(ndSpinLock& spinLock)
		:m_spinLock(spinLock)
	{
		m_spinLock.Lock();
	}

	~ndScopeSpinLock()
	{
		m_spinLock.Unlock();
	}

	ndSpinLock& m_spinLock;
};

/// Set cpu floating point exceptions, the original exception state is restored when the destructor is called.
class ndFloatExceptions
{
	public:
	//#if defined (WIN32) || defined(_WIN32)
	#if defined (_MSC_VER)
		#define D_FLOAT_EXCEPTIONS_MASK	(_EM_ZERODIVIDE | _EM_INVALID | _EM_DENORMAL)
	#else
		#define D_FLOAT_EXCEPTIONS_MASK	(FE_DIVBYZERO | FE_INVALID | FE_INEXACT)
	#endif

	D_CORE_API ndFloatExceptions(ndUnsigned32 mask = D_FLOAT_EXCEPTIONS_MASK);
	D_CORE_API ~ndFloatExceptions();

	private:
	ndUnsigned32 m_floatMask;
	ndUnsigned32 m_simdMask;
};


#endif


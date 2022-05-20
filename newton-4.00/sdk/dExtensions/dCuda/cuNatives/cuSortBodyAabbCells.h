/* Copyright (c) <2003-2021> <Julio Jerez, Newton Game Dynamics>
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

#ifndef __CU_SORT_H__
#define __CU_SORT_H__

#include <cuda.h>
#include <vector_types.h>
#include <cuda_runtime.h>
#include <ndNewtonStdafx.h>

class ndCudaContext;

// do not change this
#define D_AABB_GRID_CELL_BITS			10

class cuBodyAabbCell
{
	public:
	union
	{
		struct
		{
			union
			{
				struct
				{
					unsigned m_x : D_AABB_GRID_CELL_BITS;
					unsigned m_y : D_AABB_GRID_CELL_BITS;
					unsigned m_z : D_AABB_GRID_CELL_BITS;
				};
				unsigned m_key;
			};
			unsigned m_id;
		};
		long long m_value;
	};
};

void CudaBodyAabbCellSortBuffer(ndCudaContext* const context);


template <typename Predicate>
__global__ void XXXXXXX(Predicate GetKey)
{
	unsigned val = 0;
	val = GetKey(val);
}


#endif
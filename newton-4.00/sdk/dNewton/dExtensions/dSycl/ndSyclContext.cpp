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

#include <ndSyclStdafx.h>
#include "ndSyclUtils.h"
#include "ndSyclContext.h"
#include "ndSyclContextImpl.h"

using namespace sycl;


//void xxxxxx(ndThreadPool& pool)
//{
//	ndArray<int> xxxxx0;
//	ndArray<int> xxxxx1;
//
//	class ndSortCompactKey
//	{
//		public:
//		ndSortCompactKey(const void* const)
//		{
//		}
//
//		ndInt32 GetKey(const int body) const
//		{
//			return body & 0xff;
//		}
//	};
//
//	for (int i = 0; i < 1000; i++)
//	{
//		xxxxx0.PushBack(0xff & int(ndRandInt()));
//	}
//	ndCountingSort<int, ndSortCompactKey, 8>(pool, xxxxx0, xxxxx1, nullptr, nullptr);
//	ndCountingSort<int, ndSortCompactKey, 8>(pool, xxxxx0, xxxxx1, nullptr, nullptr);
//}

ndSyclContext::ndSyclContext(bool selectCpu)
	:m_impl(nullptr)
{
	EnumDevices(selectCpu);

	//xxxxxx(*this);
}

ndSyclContext::~ndSyclContext()
{
	if (m_impl)
	{
		delete m_impl;
	}
}

void* ndSyclContext::operator new (size_t size)
{
	return ndSyclMalloc(size);
}

void ndSyclContext::operator delete (void* ptr)
{
	ndSyclFree(ptr);
}

void ndSyclContext::SetMemoryAllocators(ndMemAllocCallback alloc, ndMemFreeCallback free)
{
	ndSyclSetMemoryAllocators(alloc, free);
}

bool ndSyclContext::IsValid() const 
{
	return m_impl ? true : false;
}

const char* ndSyclContext::GetStringId() const
{
	ndAssert(m_impl);
	return m_impl->GetStringId();
}

void ndSyclContext::EnumDevices(bool selectCpu)
{
	std::vector<device> devices;
	std::vector<platform> platforms(platform::get_platforms());
	for (int i = 0; i < platforms.size(); ++i)
	{
		platform& plat = platforms[i];
		std::vector<device> devList(plat.get_devices());
		for (int j = 0; j < devList.size(); ++j)
		{
			device& dev = devList[j];
			if (selectCpu)
			{
				if (dev.is_cpu())
				{
					devices.push_back(dev);
				}
			}
			else
			{
				if (dev.is_gpu())
				{
					devices.push_back(dev);
				}
			}
		}
	}

	if (devices.size())
	{
		device bestDevice = devices[0];
		for (int i = 0; i < devices.size(); ++i)
		{
			std::string platformName(devices[i].get_platform().get_info<info::platform::name>());
			for (int j = 0; j < platformName.size(); ++j)
			{
				platformName[j] = char(tolower(platformName[j]));
			}
			if (platformName.find("opencl") >= 0)
			{
				bestDevice = devices[i];
			}
		}

		m_impl = new ndSyclContextImpl(bestDevice);
	}
}

void ndSyclContext::Begin()
{
	m_impl->Begin();
}


#if 0
double ndSyclContext::GetGPUTime() const
{
	return IsValid() ? m_impl->GetTimeInSeconds() : 0.0;
}

ndCudaSpatialVector* ndSyclContext::GetTransformBuffer()
{
	return m_impl->GetTransformBuffer();
}


void ndSyclContext::End()
{
	m_impl->End();
}

void ndSyclContext::ResizeBuffers(int size)
{
	m_impl->ResizeBuffers(size);
}

void ndSyclContext::LoadBodyData(const ndCudaBodyProxy* const src, int size)
{
	m_impl->LoadBodyData(src, size);
}

void ndSyclContext::ValidateContextBuffers()
{
	m_impl->ValidateContextBuffers();
}

void ndSyclContext::InitBodyArray()
{
	m_impl->InitBodyArray();
}

void ndSyclContext::IntegrateBodies(float timestep)
{
	m_impl->IntegrateBodies(timestep);
}

void ndSyclContext::IntegrateUnconstrainedBodies(float timestep)
{
	m_impl->IntegrateUnconstrainedBodies(timestep);
}

void ndSyclContext::UpdateTransform()
{
	m_impl->UpdateTransform();
}
#endif
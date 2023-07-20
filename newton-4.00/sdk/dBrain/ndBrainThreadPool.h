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

#ifndef _ND_BRAIN_TREAD_POOL_H__
#define _ND_BRAIN_TREAD_POOL_H__

#include "ndBrainStdafx.h"

class ndBrainThreadPool: public ndClassAlloc
{
	public: 
	class ndWorker;

	ndBrainThreadPool();
	~ndBrainThreadPool();

	ndInt32 GetThreadCount() const;
	static ndInt32 GetMaxThreads();
	void SetThreadCount(ndInt32 count);

	template <typename Function>
	void ParallelExecute(const Function& ndFunction);

	private:
	ndFixSizeArray<ndWorker*, D_MAX_THREADS_COUNT> m_workers;
};

template <typename Function>
class ndBrainTaskImplement : public ndTask
{
	public:
	ndBrainTaskImplement(ndInt32 threadIndex, ndBrainThreadPool* const threadPool, const Function& ndFunction)
		:ndTask()
		,m_function(ndFunction)
		,m_threadPool(threadPool)
		,m_threadIndex(threadIndex)
		,m_threadCount(threadPool->GetThreadCount())
	{
	}

	~ndBrainTaskImplement()
	{
	}

	private:
	void Execute() const
	{
		m_function(m_threadIndex, m_threadCount);
	}

	Function m_function;
	ndBrainThreadPool* m_threadPool;
	const ndInt32 m_threadIndex;
	const ndInt32 m_threadCount;
	friend class ndBrainThreadPool;
};


template <typename Function>
void ndBrainThreadPool::ParallelExecute(const Function& callback)
{
	const ndInt32 threadCount = GetThreadCount();
	ndBrainTaskImplement<Function>* const jobsArray = ndAlloca(ndBrainTaskImplement<Function>, threadCount);

	for (ndInt32 i = 0; i < threadCount; ++i)
	{
		ndBrainTaskImplement<Function>* const job = &jobsArray[i];
		new (job) ndBrainTaskImplement<Function>(i, this, callback);
	}

	if (m_workers.GetCount() > 0)
	{
		#ifdef	D_USE_THREAD_EMULATION
		ndAssert(0);
		for (ndInt32 i = 0; i < threadCount; ++i)
		{
			ndTaskImplement<Function>* const job = &jobsArray[i];
			callback(job->m_threadIndex, job->m_threadCount);
		}
		#else
		ndAssert(0);
		//for (ndInt32 i = 0; i < m_count; ++i)
		//{
		//	ndBrainThreadPool<Function>* const job = &jobsArray[i];
		//	m_workers[i].m_task.store(job);
		//}
		//ndTaskImplement<Function>* const job = &jobsArray[m_count];
		//callback(job->m_threadIndex, job->m_threadCount);
		//
		//bool jobsInProgress = true;
		//do
		//{
		//	ndThreadYield();
		//	bool inProgess = false;
		//	for (ndInt32 i = 0; i < m_count; ++i)
		//	{
		//		inProgess = inProgess | (m_workers[i].m_task.load() != nullptr);
		//	}
		//	jobsInProgress = jobsInProgress & inProgess;
		//} while (jobsInProgress);
		#endif
	}
	else
	{
		ndBrainTaskImplement<Function>* const job = &jobsArray[0];
		callback(job->m_threadIndex, job->m_threadCount);
	}
}


#endif 


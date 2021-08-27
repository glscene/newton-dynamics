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

#ifndef __D_MODEL_H__
#define __D_MODEL_H__

#include "ndNewtonStdafx.h"
#include "ndModelList.h"

class ndMultiBodyVehicle;
class ndConstraintDebugCallback;

D_MSV_NEWTON_ALIGN_32
class ndModel: public dClassAlloc
{
	public:
	D_CLASS_REFLECTION(ndModel);
	ndModel();
	D_NEWTON_API ndModel(const dLoadSaveBase::dLoadDescriptor& desc);

	virtual ~ndModel ();
	
	virtual ndModel* GetAsModel();
	virtual void AddToWorld(ndWorld* const world);
	virtual void RemoveFromToWorld(ndWorld* const world);

	virtual ndMultiBodyVehicle* GetAsMultiBodyVehicle();

	virtual void Debug(ndConstraintDebugCallback& context) const;

	protected:
	virtual void Update(ndWorld* const world, dFloat32 timestep);
	virtual void PostUpdate(ndWorld* const world, dFloat32 timestep);
	virtual void PostTransformUpdate(ndWorld* const world, dFloat32 timestep);

	D_NEWTON_API virtual void Save(const dLoadSaveBase::dSaveDescriptor& desc) const;

	ndModelList::dNode* m_node;

	friend class ndWorld;
	friend class ndLoadSave;
} D_GCC_NEWTON_ALIGN_32;

inline ndModel::ndModel()
	:dClassAlloc()
	,m_node(nullptr)
{
}

inline ndModel::~ndModel()
{
	dAssert(!m_node);
}

inline ndModel* ndModel::GetAsModel()
{ 
	return this; 
}

inline ndMultiBodyVehicle* ndModel::GetAsMultiBodyVehicle()
{ 
	return nullptr; 
}

inline void ndModel::Debug(ndConstraintDebugCallback&) const
{
}

inline void ndModel::Update(ndWorld* const, dFloat32)
{
}

inline void ndModel::PostUpdate(ndWorld* const, dFloat32)
{
}

inline void ndModel::PostTransformUpdate(ndWorld* const, dFloat32)
{
}

inline void ndModel::AddToWorld(ndWorld* const)
{
}

inline void ndModel::RemoveFromToWorld(ndWorld* const)
{
}

#endif 



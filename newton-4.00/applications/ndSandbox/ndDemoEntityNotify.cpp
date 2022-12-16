/* Copyright (c) <2003-2022> <Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/

#include "ndSandboxStdafx.h"
#include "ndDemoMesh.h"
#include "ndDemoEntity.h"
#include "ndPhysicsWorld.h"
#include "ndDemoEntityNotify.h"

D_CLASS_REFLECTION_IMPLEMENT_LOADER(ndDemoEntityNotify)
D_CLASS_REFLECTION_IMPLEMENT_LOADER(ndBindingRagdollEntityNotify)

ndDemoEntityNotify::ndDemoEntityNotify(ndDemoEntityManager* const manager, ndDemoEntity* const entity, ndBodyKinematic* const parentBody, ndFloat32 gravity)
	:ndBodyNotify(ndVector (ndFloat32 (0.0f), gravity, ndFloat32(0.0f), ndFloat32(0.0f)))
	,m_entity(entity)
	,m_parentBody(parentBody)
	,m_manager(manager)
{
}

// member a fill in a post process pass
ndDemoEntityNotify::ndDemoEntityNotify(const ndLoadSaveBase::ndLoadDescriptor& desc)
	:ndBodyNotify(ndLoadSaveBase::ndLoadDescriptor(desc))
	,m_entity(nullptr)
	,m_parentBody(nullptr)
	,m_manager(nullptr)
{
//	const nd::TiXmlNode* const rootNode = desc.m_rootNode;
// remember to save member below
}

ndDemoEntityNotify::~ndDemoEntityNotify()
{
	if (m_entity && m_entity->m_rootNode)
	{
		m_manager->RemoveEntity(m_entity);
		delete m_entity;
	}
}

void ndDemoEntityNotify::Save(const ndLoadSaveBase::ndSaveDescriptor& desc) const
{
	nd::TiXmlElement* const childNode = new nd::TiXmlElement(ClassName());
	desc.m_rootNode->LinkEndChild(childNode);
	ndBodyNotify::Save(ndLoadSaveBase::ndSaveDescriptor(desc, childNode));
	xmlSaveParam(childNode, "comment", "string", "body notification for Newton 4.0 demos");

	// remember to save member below
	//ndDemoEntity* m_entity;
	//ndBodyDynamic* m_parentBody;
	//ndDemoEntityManager* m_manager;
}

void ndDemoEntityNotify::OnObjectPick() const
{
	ndTrace(("picked body id: %d\n", GetBody()->GetId()));
}

void ndDemoEntityNotify::OnApplyExternalForce(ndInt32, ndFloat32)
{
	ndBodyKinematic* const body = GetBody()->GetAsBodyKinematic();
	ndAssert(body);
	if (body->GetInvMass() > 0.0f)
	{
		ndVector massMatrix(body->GetMassMatrix());
		ndVector force(GetGravity().Scale(massMatrix.m_w));
		body->SetForce(force);
		body->SetTorque(ndVector::m_zero);
	}
}

void ndDemoEntityNotify::OnTransform(ndInt32, const ndMatrix& matrix)
{
	// apply this transformation matrix to the application user data.
	if (m_entity)
	{
		const ndBody* const body = GetBody();
		if (!m_parentBody)
		{
			const ndQuaternion rot(body->GetRotation());
			m_entity->SetMatrix(rot, matrix.m_posit);
		}
		else
		{
			const ndMatrix parentMatrix(m_parentBody->GetMatrix());
			const ndMatrix localMatrix(matrix * parentMatrix.Inverse());
			const ndQuaternion rot(localMatrix);
			m_entity->SetMatrix(rot, localMatrix.m_posit);
		}
	}

	OutsideWorldCheck(matrix);
}

void ndDemoEntityNotify::OutsideWorldCheck(const ndMatrix& matrix)
{
	// check world bounds
	if (matrix.m_posit.m_y < -100.0f)
	{
		ndBody* const body = GetBody();
		ndPhysicsWorld* const world = m_manager->GetWorld();
		world->RemoveBody(body);
	}
}

ndBindingRagdollEntityNotify::ndBindingRagdollEntityNotify(ndDemoEntityManager* const manager, ndDemoEntity* const entity, ndBodyDynamic* const parentBody, ndFloat32 capSpeed)
	:ndDemoEntityNotify(manager, entity, parentBody)
	,m_bindMatrix(ndGetIdentityMatrix())
	,m_capSpeed(capSpeed)
{
	ndDemoEntity* const parentEntity = m_parentBody ? (ndDemoEntity*)(parentBody->GetNotifyCallback()->GetUserData()) : nullptr;
	m_bindMatrix = entity->GetParent()->CalculateGlobalMatrix(parentEntity).Inverse();
}

ndBindingRagdollEntityNotify::ndBindingRagdollEntityNotify(const ndLoadSaveBase::ndLoadDescriptor& desc)
	:ndDemoEntityNotify(ndLoadSaveBase::ndLoadDescriptor(desc))
	,m_bindMatrix(nullptr)
	,m_capSpeed(0.0f)
{
	ndAssert(0);
}

void ndBindingRagdollEntityNotify::Save(const ndLoadSaveBase::ndSaveDescriptor& desc) const
{
	ndAssert(0);
	nd::TiXmlElement* const childNode = new nd::TiXmlElement(ClassName());
	desc.m_rootNode->LinkEndChild(childNode);
	ndBodyNotify::Save(ndLoadSaveBase::ndSaveDescriptor(desc, childNode));
}

void ndBindingRagdollEntityNotify::OnTransform(ndInt32, const ndMatrix& matrix)
{
	if (!m_parentBody)
	{
		const ndMatrix localMatrix(matrix * m_bindMatrix);
		const ndQuaternion rot(localMatrix);
		m_entity->SetMatrix(rot, localMatrix.m_posit);
	}
	else
	{
		const ndMatrix parentMatrix(m_parentBody->GetMatrix());
		const ndMatrix localMatrix(matrix * parentMatrix.Inverse() * m_bindMatrix);
		const ndQuaternion rot(localMatrix);
		m_entity->SetMatrix(rot, localMatrix.m_posit);
	}

	OutsideWorldCheck(matrix);
}

void ndBindingRagdollEntityNotify::OnApplyExternalForce(ndInt32 thread, ndFloat32 timestep)
{
	ndDemoEntityNotify::OnApplyExternalForce(thread, timestep);

	// Clamp huge angular and linear velocities generated by high speed collisions
	ndBodyKinematic* const body = GetBody()->GetAsBodyKinematic();
	ndAssert(body && body->GetInvMass() > 0.0f);

	// clamp execive velocites.
	ndVector omega(body->GetOmega());
	ndVector veloc(body->GetVelocity());
	ndFloat32 omega2(omega.DotProduct(omega).GetScalar());
	if (omega2 > m_capSpeed * m_capSpeed)
	{
		omega = omega.Normalize().Scale(m_capSpeed);
		body->SetOmega(omega);
	}

	ndFloat32 veloc2(veloc.DotProduct(veloc).GetScalar());
	if (veloc2 > m_capSpeed * m_capSpeed)
	{
		veloc = veloc.Normalize().Scale(m_capSpeed);
		body->SetVelocity(veloc);
	}
}


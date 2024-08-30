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
#include "ndSkyBox.h"
#include "ndDemoMesh.h"
#include "ndUIEntity.h"
#include "ndMeshLoader.h"
#include "ndDemoCamera.h"
#include "ndPhysicsUtils.h"
#include "ndPhysicsWorld.h"
#include "ndMakeStaticMap.h"
#include "ndDemoEntityNotify.h"
#include "ndDemoEntityManager.h"
#include "ndDemoInstanceEntity.h"

namespace ndSimpleRobot
{
	class ndDefinition
	{
		public:
		enum ndJointType
		{
			m_root,
			m_hinge,
			m_slider,
			m_effector
		};

		char m_boneName[32];
		ndJointType m_type;
		ndFloat32 m_mass;
		ndFloat32 m_minLimit;
		ndFloat32 m_maxLimit;
	};

	static ndDefinition jointsDefinition[] =
	{
		{ "base", ndDefinition::m_root, 100.0f, 0.0f, 0.0f},
		{ "base_rotator", ndDefinition::m_hinge, 50.0f, -1.0e10f, 1.0e10f},
		{ "arm_0", ndDefinition::m_hinge , 5.0f, -120.0f * ndDegreeToRad, 120.0f * ndDegreeToRad},
		{ "arm_1", ndDefinition::m_hinge , 5.0f, -90.0f * ndDegreeToRad, 60.0f * ndDegreeToRad},
		{ "arm_2", ndDefinition::m_hinge , 5.0f, -1.0e10f, 1.0e10f},
		{ "arm_3", ndDefinition::m_hinge , 3.0f, -1.0e10f, 1.0e10f},
		{ "arm_4", ndDefinition::m_hinge , 2.0f, -1.0e10f, 1.0e10f},
		{ "gripperLeft", ndDefinition::m_slider , 1.0f, -0.2f, 0.03f},
		{ "gripperRight", ndDefinition::m_slider , 1.0f, -0.2f, 0.03f},
		{ "effector", ndDefinition::m_effector , 0.0f, 0.0f, 0.0f},
	};

	#define ND_MIN_X_SPAND			ndReal (-1.5f)
	#define ND_MAX_X_SPAND			ndReal ( 1.5f)
	#define ND_MIN_Y_SPAND			ndReal (-2.2f)
	#define ND_MAX_Y_SPAND			ndReal ( 1.5f)

	class RobotModelNotify : public ndModelNotify
	{
		public:
		RobotModelNotify(ndModelArticulation* const robot, bool showDebug)
			:ndModelNotify()
			,m_world(nullptr)
			,m_x(0.0f)
			,m_y(0.0f)
			,m_azimuth(0.0f)
			,m_yaw(0.0f)
			,m_pitch(0.0f)
			,m_roll(0.0f)
			,m_gripperPosit(0.0f)
			,m_timestep(ndFloat32(0.0f))
			,m_showDebug(showDebug)
		{
			Init(robot);
		}

		RobotModelNotify(const RobotModelNotify& src)
			:ndModelNotify(src)
		{
			//Init(robot);
			ndAssert(0);
		}

		//RobotModelNotify(const RobotModelNotify& src)
		//	:ndModelNotify(src)
		//	,m_controller(src.m_controller)
		//{
		//	//Init(robot);
		//	ndAssert(0);
		//}

		~RobotModelNotify()
		{
		}

		ndModelNotify* Clone() const
		{
			return new RobotModelNotify(*this);
		}

		void Init(ndModelArticulation* const robot)
		{
			m_rootBody = robot->GetRoot()->m_body->GetAsBodyDynamic();
			m_leftGripper = (ndJointSlider*)robot->FindByName("leftGripper")->m_joint->GetAsBilateral();
			m_rightGripper = (ndJointSlider*)robot->FindByName("rightGripper")->m_joint->GetAsBilateral();
			m_effector = (ndIk6DofEffector*)robot->FindLoopByName("effector")->m_joint->GetAsBilateral();
			m_effectorOffset = m_effector->GetOffsetMatrix().m_posit;
		}

		void ResetModel()
		{
			ndTrace(("Reset Model\n"));
		}

		void Update(ndWorld* const world, ndFloat32 timestep)
		{
			m_world = world;
			m_timestep = timestep;

			m_leftGripper->SetOffsetPosit(-m_gripperPosit * 0.5f);
			m_rightGripper->SetOffsetPosit(-m_gripperPosit * 0.5f);

			// apply target position collected by control panel
			ndMatrix targetMatrix(
				ndRollMatrix(90.0f * ndDegreeToRad) *
				ndPitchMatrix(m_pitch) *
				ndYawMatrix(m_yaw) *
				ndRollMatrix(m_roll) *
				ndRollMatrix(-90.0f * ndDegreeToRad));

			ndVector localPosit(m_x, m_y, 0.0f, 0.0f);
			const ndMatrix aximuthMatrix(ndYawMatrix(m_azimuth));
			targetMatrix.m_posit = aximuthMatrix.TransformVector(m_effectorOffset + localPosit);
			m_effector->SetOffsetMatrix(targetMatrix);

			//xxxx0 = m_effector->GetEffectorMatrix();
		}

		void PostUpdate(ndWorld* const, ndFloat32)
		{
			//xxxx1 = m_effector->GetEffectorMatrix();
			//
			//ndFloat32 azimuth = 0.0f;
			//const ndVector posit(xxxx1.m_posit);
			//if ((posit.m_x * posit.m_x + posit.m_z * posit.m_z) > 1.0e-3f)
			//{
			//	azimuth = ndAtan2(-posit.m_z, posit.m_x);
			//}
			//const ndMatrix aximuthMatrix(ndYawMatrix(azimuth));
			//const ndVector currenPosit(aximuthMatrix.UnrotateVector(posit) - m_effectorOffset);
			//ndAssert(currenPosit.m_x >= -1.0e-2f);
			//const ndVector currenPosit1(aximuthMatrix.UnrotateVector(posit) - m_effectorOffset);

			ndMatrix matrix(m_effector->GetEffectorMatrix());
			ndMatrix rotation0(ndRollMatrix(-90.0f * ndDegreeToRad) * matrix * ndRollMatrix(90.0f * ndDegreeToRad));
			ndMatrix rotation1(ndPitchMatrix(m_pitch) * ndYawMatrix(m_yaw) * ndRollMatrix(m_roll));
			ndMatrix rotation2(ndPitchMatrix(m_pitch) * ndYawMatrix(m_yaw) * ndRollMatrix(m_roll));
		}

		void PostTransformUpdate(ndWorld* const, ndFloat32)
		{
		}

		//void Debug(ndConstraintDebugCallback& context) const
		void Debug(ndConstraintDebugCallback&) const
		{
			//ndTrace(("xxxxx\n"));
			//if (!m_showDebug)
			//{
			//	return;
			//}
			//
			//ndModelArticulation* const model = GetModel()->GetAsModelArticulation();
			//
			//ndFixSizeArray<const ndBodyKinematic*, 32> bodies;
			//
			//ndFloat32 totalMass = ndFloat32(0.0f);
			//ndVector centerOfMass(ndVector::m_zero);
			//for (ndModelArticulation::ndNode* node = model->GetRoot()->GetFirstIterator(); node; node = node->GetNextIterator())
			//{
			//	const ndBodyKinematic* const body = node->m_body->GetAsBodyKinematic();
			//	const ndMatrix matrix(body->GetMatrix());
			//	ndFloat32 mass = body->GetMassMatrix().m_w;
			//	totalMass += mass;
			//	centerOfMass += matrix.TransformVector(body->GetCentreOfMass()).Scale(mass);
			//	bodies.PushBack(body);
			//}
			//ndFloat32 invMass = 1.0f / totalMass;
			//centerOfMass = centerOfMass.Scale(invMass);
			//
			//ndVector comLineOfAction(centerOfMass);
			//comLineOfAction.m_y -= ndFloat32(0.5f);
			//context.DrawLine(centerOfMass, comLineOfAction, ndVector::m_zero);
			//
			//ndBodyKinematic* const rootBody = model->GetRoot()->m_body->GetAsBodyKinematic();
			//const ndVector upVector(rootBody->GetMatrix().m_up);
			//ndFixSizeArray<ndBigVector, 16> supportPoint;
			//for (ndInt32 i = 0; i < m_animPose.GetCount(); ++i)
			//{
			//	const ndAnimKeyframe& keyFrame = m_animPose[i];
			//	ndEffectorInfo* const info = (ndEffectorInfo*)keyFrame.m_userData;
			//	ndIkSwivelPositionEffector* const effector = (ndIkSwivelPositionEffector*)*info->m_effector;
			//	if (i == 0)
			//	{
			//		effector->DebugJoint(context);
			//	}
			//
			//	if (keyFrame.m_userParamFloat < 1.0f)
			//	{
			//		ndBodyKinematic* const body = effector->GetBody0();
			//		supportPoint.PushBack(body->GetMatrix().TransformVector(effector->GetLocalMatrix0().m_posit));
			//	}
			//}
			//
			//ndVector supportColor(0.0f, 1.0f, 1.0f, 1.0f);
			//if (supportPoint.GetCount() >= 3)
			//{
			//	ScaleSupportShape(supportPoint);
			//	ndFixSizeArray<ndVector, 16> desiredSupportPoint;
			//	for (ndInt32 i = 0; i < supportPoint.GetCount(); ++i)
			//	{
			//		desiredSupportPoint.PushBack(supportPoint[i]);
			//	}
			//
			//	ndMatrix rotation(ndPitchMatrix(90.0f * ndDegreeToRad));
			//	rotation.TransformTriplex(&desiredSupportPoint[0].m_x, sizeof(ndVector), &desiredSupportPoint[0].m_x, sizeof(ndVector), desiredSupportPoint.GetCount());
			//	ndInt32 supportCount = ndConvexHull2d(&desiredSupportPoint[0], desiredSupportPoint.GetCount());
			//	rotation.OrthoInverse().TransformTriplex(&desiredSupportPoint[0].m_x, sizeof(ndVector), &desiredSupportPoint[0].m_x, sizeof(ndVector), desiredSupportPoint.GetCount());
			//	ndVector p0(desiredSupportPoint[supportCount - 1]);
			//	ndBigVector bigPolygon[16];
			//	for (ndInt32 i = 0; i < supportCount; ++i)
			//	{
			//		bigPolygon[i] = desiredSupportPoint[i];
			//		context.DrawLine(desiredSupportPoint[i], p0, supportColor);
			//		p0 = desiredSupportPoint[i];
			//	}
			//
			//	ndBigVector p0Out;
			//	ndBigVector p1Out;
			//	ndBigVector ray_p0(centerOfMass);
			//	ndBigVector ray_p1(comLineOfAction);
			//	ndRayToPolygonDistance(ray_p0, ray_p1, bigPolygon, supportCount, p0Out, p1Out);
			//
			//	const ndVector centerOfPresure(p0Out);
			//	context.DrawPoint(centerOfPresure, ndVector(0.0f, 0.0f, 1.0f, 1.0f), 5);
			//
			//	ndVector zmp(CalculateZeroMomentPoint());
			//	ray_p0 = zmp;
			//	ray_p1 = zmp;
			//	ray_p1.m_y -= ndFloat32(0.5f);
			//	ndRayToPolygonDistance(ray_p0, ray_p1, bigPolygon, supportCount, p0Out, p1Out);
			//	const ndVector zmpSupport(p0Out);
			//	context.DrawPoint(zmpSupport, ndVector(1.0f, 0.0f, 0.0f, 1.0f), 5);
			//}
			//else if (supportPoint.GetCount() == 2)
			//{
			//	ndTrace(("xxxxxxxxxx\n"));
			//	context.DrawLine(supportPoint[0], supportPoint[1], supportColor);
			//	//ndBigVector p0Out;
			//	//ndBigVector p1Out;
			//	//ndBigVector ray_p0(comMatrix.m_posit);
			//	//ndBigVector ray_p1(comMatrix.m_posit);
			//	//ray_p1.m_y -= 1.0f;
			//	//
			//	//ndRayToRayDistance(ray_p0, ray_p1, contactPoints[0], contactPoints[1], p0Out, p1Out);
			//	//context.DrawPoint(p0Out, ndVector(1.0f, 0.0f, 0.0f, 1.0f), 3);
			//	//context.DrawPoint(p1Out, ndVector(0.0f, 1.0f, 0.0f, 1.0f), 3);
			//}
		}

		//ndMatrix xxxx0;
		//ndMatrix xxxx1;
		ndVector m_effectorOffset;
		ndBodyDynamic* m_rootBody;
		ndJointSlider* m_leftGripper;
		ndJointSlider* m_rightGripper;
		ndIk6DofEffector* m_effector;
		ndWorld* m_world;

		ndReal m_x;
		ndReal m_y;
		ndReal m_azimuth;
		ndReal m_yaw;
		ndReal m_pitch;
		ndReal m_roll;
		ndReal m_gripperPosit;
		ndFloat32 m_timestep;
		bool m_showDebug;

		friend class ndRobotUI;
	};

	class ndRobotUI : public ndUIEntity
	{
		public:
		ndRobotUI(ndDemoEntityManager* const scene, RobotModelNotify* const robot)
			:ndUIEntity(scene)
			,m_robot(robot)
		{
		}
	
		~ndRobotUI()
		{
		}
	
		virtual void RenderUI()
		{
		}
	
		virtual void RenderHelp()
		{
			ndVector color(1.0f, 1.0f, 0.0f, 0.0f);
			m_scene->Print(color, "Control panel");

			ndInt8 change = 0;
			change = change | ndInt8(ImGui::SliderFloat("x", &m_robot->m_x, ND_MIN_X_SPAND, ND_MAX_X_SPAND));
			change = change | ndInt8(ImGui::SliderFloat("y", &m_robot->m_y, ND_MIN_Y_SPAND, ND_MAX_Y_SPAND));
			change = change | ndInt8(ImGui::SliderFloat("azimuth", &m_robot->m_azimuth, -ndPi, ndPi));
			change = change | ndInt8(ImGui::SliderFloat("pitch", &m_robot->m_pitch, -ndPi, ndPi));
			change = change | ndInt8(ImGui::SliderFloat("yaw", &m_robot->m_yaw, -ndPi * 0.5f, ndPi * 0.5f));
			change = change | ndInt8(ImGui::SliderFloat("roll", &m_robot->m_roll, -ndPi, ndPi));
			change = change | ndInt8(ImGui::SliderFloat("gripper", &m_robot->m_gripperPosit, -0.2f, 0.4f));

			if (change)
			{
				m_robot->GetModel()->GetAsModelArticulation()->GetRoot()->m_body->GetAsBodyKinematic()->SetSleepState(false);
			}
		}
	
		RobotModelNotify* m_robot;
	};

	ndBodyDynamic* CreateBodyPart(ndDemoEntityManager* const scene, ndDemoEntity* const entityPart, ndFloat32 mass, ndBodyDynamic* const parentBone)
	{
		ndSharedPtr<ndShapeInstance> shapePtr(entityPart->CreateCollisionFromChildren());
		ndShapeInstance* const shape = *shapePtr;
		ndAssert(shape);

		// create the rigid body that will make this body
		ndMatrix matrix(entityPart->CalculateGlobalMatrix());

		ndBodyKinematic* const body = new ndBodyDynamic();
		body->SetMatrix(matrix);
		body->SetCollisionShape(*shape);
		body->SetMassMatrix(mass, *shape);
		body->SetNotifyCallback(new ndDemoEntityNotify(scene, entityPart, parentBone));
		return body->GetAsBodyDynamic();
	}

	void NormalizeMassRatios(ndModelArticulation* const model)
	{
		ndFloat32 totalVolume = 0.0f;
		ndFixSizeArray<ndBodyKinematic*, 256> bodyArray;

		ndFloat32 totalMass = 0.0f;
		for (ndModelArticulation::ndNode* node = model->GetRoot()->GetFirstIterator(); node; node = node->GetNextIterator())
		{
			ndBodyKinematic* const body = node->m_body->GetAsBodyKinematic();
			ndFloat32 volume = body->GetCollisionShape().GetVolume();
			totalMass += body->GetMassMatrix().m_w;
			totalVolume += volume;
			bodyArray.PushBack(body);
		}

		ndFloat32 density = totalMass / totalVolume;
		for (ndInt32 i = 0; i < bodyArray.GetCount(); ++i)
		{
			ndBodyKinematic* const body = bodyArray[i];
			ndFloat32 volume = body->GetCollisionShape().GetVolume();
			ndFloat32 mass = density * volume;

			body->SetMassMatrix(mass, body->GetCollisionShape());
			ndVector inertia(body->GetMassMatrix());
			ndFloat32 maxInertia = ndMax(ndMax(inertia.m_x, inertia.m_y), inertia.m_z);
			ndFloat32 minInertia = ndMin(ndMin(inertia.m_x, inertia.m_y), inertia.m_z);
			if (minInertia < maxInertia * 0.125f)
			{
				minInertia = maxInertia * 0.125f;
				for (ndInt32 j = 0; j < 3; ++j)
				{
					if (inertia[j] < minInertia)
					{
						inertia[j] = minInertia;
					}
				}
			}
			body->SetMassMatrix(inertia);
		}
	}

	ndModelArticulation* CreateModel(ndDemoEntityManager* const scene, ndDemoEntity* const modelMesh, const ndMatrix& location)
	{
		// make a clone of the mesh and add it to the scene
		ndModelArticulation* const model = new ndModelArticulation();

		ndWorld* const world = scene->GetWorld();
		ndDemoEntity* const entity = modelMesh->CreateClone();
		scene->AddEntity(entity);

		ndDemoEntity* const rootEntity = (ndDemoEntity*)entity->Find(jointsDefinition[0].m_boneName);
		ndMatrix matrix(rootEntity->CalculateGlobalMatrix() * location);

		// find the floor location 
		ndVector floor(FindFloor(*world, matrix.m_posit + ndVector(0.0f, 100.0f, 0.0f, 0.0f), 200.0f));
		matrix.m_posit.m_y = floor.m_y;
				
		rootEntity->ResetMatrix(matrix);

		// add the root body
		ndSharedPtr<ndBody> rootBody(CreateBodyPart(scene, rootEntity, jointsDefinition[0].m_mass, nullptr));

		rootBody->SetMatrix(rootEntity->CalculateGlobalMatrix());

		// add the root body to the model
		ndModelArticulation::ndNode* const modelNode = model->AddRootBody(rootBody);
		
		ndFixSizeArray<ndDemoEntity*, 32> childEntities;
		ndFixSizeArray<ndModelArticulation::ndNode*, 32> parentBones;
		for (ndDemoEntity* child = rootEntity->GetFirstChild(); child; child = child->GetNext())
		{
			childEntities.PushBack(child);
			parentBones.PushBack(modelNode);
		}
		
		const ndInt32 definitionCount = ndInt32(sizeof(jointsDefinition) / sizeof(jointsDefinition[0]));
		while (parentBones.GetCount())
		{
			ndDemoEntity* const childEntity = childEntities.Pop();
			ndModelArticulation::ndNode* parentBone = parentBones.Pop();

			const char* const name = childEntity->GetName().GetStr();
			for (ndInt32 i = 0; i < definitionCount; ++i)
			{
				const ndDefinition& definition = jointsDefinition[i];
				if (!strcmp(definition.m_boneName, name))
				{
					ndTrace(("name: %s\n", name));
					if (definition.m_type == ndDefinition::m_hinge)
					{
						ndSharedPtr<ndBody> childBody (CreateBodyPart(scene, childEntity, definition.m_mass, parentBone->m_body->GetAsBodyDynamic()));
						const ndMatrix pivotMatrix(childBody->GetMatrix());
						ndSharedPtr<ndJointBilateralConstraint> hinge (new ndJointHinge(pivotMatrix, childBody->GetAsBodyKinematic(), parentBone->m_body->GetAsBodyKinematic()));

						ndJointHinge* const hingeJoint = (ndJointHinge*)*hinge;
						hingeJoint->SetLimits(definition.m_minLimit, definition.m_maxLimit);
						if ((definition.m_minLimit > -1000.0f) && (definition.m_maxLimit < 1000.0f))
						{
							hingeJoint->SetLimitState(true);
						}

						parentBone = model->AddLimb(parentBone, childBody, hinge);
					}
					else if (definition.m_type == ndDefinition::m_slider)
					{
						ndSharedPtr<ndBody> childBody(CreateBodyPart(scene, childEntity, definition.m_mass, parentBone->m_body->GetAsBodyDynamic()));
						
						const ndMatrix pivotMatrix(childBody->GetMatrix());
						ndSharedPtr<ndJointBilateralConstraint> slider (new ndJointSlider(pivotMatrix, childBody->GetAsBodyKinematic(), parentBone->m_body->GetAsBodyKinematic()));

						ndJointSlider* const sliderJoint = (ndJointSlider*)*slider;
						sliderJoint->SetLimits(definition.m_minLimit, definition.m_maxLimit);
						sliderJoint->SetAsSpringDamper(0.001f, 2000.0f, 100.0f);
						parentBone = model->AddLimb(parentBone, childBody, slider);

						if (!strstr(definition.m_boneName, "Left"))
						{
							parentBone->m_name = "leftGripper";
						}
						else
						{
							parentBone->m_name = "rightGripper";
						}
					}
					else if (definition.m_type == ndDefinition::m_effector)
					{
						ndBodyDynamic* const childBody = parentBone->m_body->GetAsBodyDynamic();
						
						const ndMatrix pivotFrame(rootEntity->Find("referenceFrame")->CalculateGlobalMatrix());
						const ndMatrix effectorFrame(childEntity->CalculateGlobalMatrix());
						
						ndSharedPtr<ndJointBilateralConstraint> effector (new ndIk6DofEffector(effectorFrame, pivotFrame, childBody, modelNode->m_body->GetAsBodyKinematic()));
						
						ndIk6DofEffector* const effectorJoint = (ndIk6DofEffector*)*effector;
						ndFloat32 relaxation = 1.0e-4f;
						effectorJoint->EnableRotationAxis(ndIk6DofEffector::m_shortestPath);
						effectorJoint->SetLinearSpringDamper(relaxation, 10000.0f, 500.0f);
						effectorJoint->SetAngularSpringDamper(relaxation, 10000.0f, 500.0f);
						effectorJoint->SetMaxForce(10000.0f);
						effectorJoint->SetMaxTorque(10000.0f);
						
						// the effector is part of the rig
						model->AddCloseLoop(effector, "effector");
					}
					break;
				}
			}
		
			for (ndDemoEntity* child = childEntity->GetFirstChild(); child; child = child->GetNext())
			{
				childEntities.PushBack(child);
				parentBones.PushBack(parentBone);
			}
		}
		NormalizeMassRatios(model);
		return model;
	}

	void AddBackgroundScene(ndDemoEntityManager* const scene, const ndMatrix& matrix)
	{
		ndMatrix location(matrix);
		location.m_posit.m_x += 1.5f;
		location.m_posit.m_z += 1.5f;
		AddBox(scene, location, 2.0f, 0.3f, 0.4f, 0.7f);
		AddBox(scene, location, 1.0f, 0.3f, 0.4f, 0.7f);
		
		location = ndYawMatrix(90.0f * ndDegreeToRad) * location;
		location.m_posit.m_x += 1.0f;
		location.m_posit.m_z += 0.5f;
		AddBox(scene, location, 8.0f, 0.3f, 0.4f, 0.7f);
		AddBox(scene, location, 4.0f, 0.3f, 0.4f, 0.7f);
	}
}

using namespace ndSimpleRobot;
void ndSimpleIndustrialRobot (ndDemoEntityManager* const scene)
{
	// build a floor
	ndBodyKinematic* const floor = BuildFloorBox(scene, ndGetIdentityMatrix());
	
	ndVector origin1(0.0f, 0.0f, 0.0f, 1.0f);
	ndMeshLoader loader;
	ndSharedPtr<ndDemoEntity> modelMesh(loader.LoadEntity("robot.fbx", scene));
	ndMatrix matrix(ndYawMatrix(-90.0f * ndDegreeToRad));

	AddBackgroundScene(scene, matrix);

	auto SpawnModel = [scene, &modelMesh, floor](const ndMatrix& matrix)
	{
		ndWorld* const world = scene->GetWorld();
		ndModelArticulation* const model = CreateModel(scene, *modelMesh, matrix);
		model->SetNotifyCallback(new RobotModelNotify(model, true));
		model->AddToWorld(world);
		((RobotModelNotify*)*model->GetNotifyCallback())->ResetModel();

		ndSharedPtr<ndJointBilateralConstraint> fixJoint(new ndJointFix6dof(model->GetRoot()->m_body->GetMatrix(), model->GetRoot()->m_body->GetAsBodyKinematic(), floor));
		world->AddJoint(fixJoint);
		return model;
	};

	ndModelArticulation* const visualModel = SpawnModel(matrix);
	ndSharedPtr<ndUIEntity> robotUI(new ndRobotUI(scene, (RobotModelNotify*)*visualModel->GetNotifyCallback()));
	scene->Set2DDisplayRenderFunction(robotUI);

	matrix.m_posit.m_x -= 5.0f;
	matrix.m_posit.m_y += 2.0f;
	matrix.m_posit.m_z += 5.0f;
	ndQuaternion rotation(ndVector(0.0f, 1.0f, 0.0f, 0.0f), 45.0f * ndDegreeToRad);
	scene->SetCameraMatrix(rotation, matrix.m_posit);
}

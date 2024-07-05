#include "stdafx.h"
#include "ComponentTestScene.h"

#include "Prefabs/SpherePrefab.h"

void ComponentTestScene::Initialize()
{
	auto& physX = PxGetPhysics();
	auto pBouncyMaterial = physX.createMaterial(0.5f, 0.5f, 0.999999999999999999999f);

	//GroundPlane
	GameSceneExt::CreatePhysXGroundPlane(*this, pBouncyMaterial);

	//Shere Red(Group0)
	auto pSphere = new SpherePrefab(1, 10, XMFLOAT4{ Colors::Red });
	AddChild(pSphere);

	pSphere->GetTransform()->Translate(0, 30.f, 0);
	auto pRigidBody = pSphere->AddComponent(new RigidBodyComponent(false));
	pRigidBody->AddCollider(PxSphereGeometry{ 1.f }, *pBouncyMaterial);
	pRigidBody->SetCollisionGroup(CollisionGroup::Group0);
	pRigidBody->SetCollisionIgnoreGroups(CollisionGroup::Group1 | CollisionGroup::Group2);

	//Shere Gree(Group1)
	pSphere = new SpherePrefab(1, 10, XMFLOAT4{ Colors::Green });
	AddChild(pSphere);

	pSphere->GetTransform()->Translate(0.5f, 20.f, 0);
	pRigidBody = pSphere->AddComponent(new RigidBodyComponent(false));
	pRigidBody->AddCollider(PxSphereGeometry{ 1.f }, *pBouncyMaterial);
	pRigidBody->SetCollisionGroup(CollisionGroup::Group1);
	pRigidBody->SetConstraint(RigidBodyConstraint::TransX | RigidBodyConstraint::TransZ, false);

	//Shere blue(Group2)
	pSphere = new SpherePrefab(1, 10, XMFLOAT4{ Colors::Blue });
	AddChild(pSphere);

	pSphere->GetTransform()->Translate(0, 10.f, 0);
	pRigidBody = pSphere->AddComponent(new RigidBodyComponent(false));
	pRigidBody->AddCollider(PxSphereGeometry{ 1.f }, *pBouncyMaterial);
	pRigidBody->SetCollisionGroup(CollisionGroup::Group2);

	
}

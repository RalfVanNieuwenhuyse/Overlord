#include "stdafx.h"
#include "ModelTestScene.h"

//#include "Managers/MaterialManager.h"
#include "Materials/DiffuseMaterial.h"

void ModelTestScene::Initialize()
{
	auto& physX = PxGetPhysics();
	auto pBouncyMaterial = physX.createMaterial(0.0f, 0.0f, 1.f);

	DiffuseMaterial* pMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial>();
	pMaterial->SetDiffuseTexture(L"Textures/Chair_Dark.dds");
	m_pChair = new GameObject{};
	auto pModel = m_pChair->AddComponent(new ModelComponent(L"Meshes/Chair.ovm"));
	pModel->SetMaterial(pMaterial);

	AddChild(m_pChair);

	auto pConvexMesh = ContentManager::Load<PxConvexMesh>(L"Meshes/chair.ovpc");

	auto pRigidBody = m_pChair->AddComponent(new RigidBodyComponent(false));
	
	pRigidBody->AddCollider(PxConvexMeshGeometry{ pConvexMesh }, *pBouncyMaterial);
	pRigidBody->SetDensity(10.f);

	GameSceneExt::CreatePhysXGroundPlane(*this, pBouncyMaterial);

	m_pChair->GetTransform()->Translate(0, 5, 0);

}

void ModelTestScene::Update()
{
}

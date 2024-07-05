#include "stdafx.h"
#include "SpikyScene.h"
#include "Materials/SpikyMaterial.h"

void SpikyScene::Initialize()
{
	m_pMaterial = MaterialManager::Get()->CreateMaterial<SpikyMaterial>();
	
	m_pOctaSphere = new GameObject{};
	auto pModel = m_pOctaSphere->AddComponent(new ModelComponent(L"Meshes/OctaSphere.ovm"));
	pModel->SetMaterial(m_pMaterial);
	m_pOctaSphere->GetTransform()->Scale(10, 10, 10);
	AddChild(m_pOctaSphere);

	m_SceneContext.settings.drawGrid = false;
	m_SceneContext.settings.enableOnGUI = true;
}

void SpikyScene::Update()
{
}

void SpikyScene::OnGUI()
{
	m_pMaterial->DrawImGui();
}

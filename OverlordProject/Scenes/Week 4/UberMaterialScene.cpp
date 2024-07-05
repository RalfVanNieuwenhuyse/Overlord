#include "stdafx.h"
#include "UberMaterialScene.h"
#include "Prefabs/SpherePrefab.h"
#include "Materials/UberMaterial.h"

void UberMaterialScene::Initialize()
{
	m_pUberMaterial = MaterialManager::Get()->CreateMaterial<UberMaterial>();

	m_pSphere = new GameObject();
	m_pSphere->AddComponent(new ModelComponent(L"Meshes/Sphere.ovm"))->SetMaterial(m_pUberMaterial);
	m_pSphere->GetTransform()->Scale({ 10, 10, 10 });

	AddChild(m_pSphere);

	m_SceneContext.settings.drawGrid = false;
	m_SceneContext.settings.enableOnGUI = true;
}

void UberMaterialScene::Update()
{
	
	if (m_RotateObject)
	{
		const float rotationSpeed = 45.f; 
		const float maxRotation = 360.f; 
		float rotation = rotationSpeed * m_SceneContext.pGameTime->GetTotal();
		
		while (rotation >= maxRotation)
		{
			rotation -= maxRotation;
		}

		m_pSphere->GetTransform()->Rotate(0, rotation, 0);
	}

	if (m_SceneContext.pInput->IsKeyboardKey(InputState::pressed, 'C'))
	{
		Logger::LogInfo(L"Camera pos: x: {} y: {} z: {},\n camera rot x: {} ,y: {} ,z: {}",
			m_SceneContext.pCamera->GetTransform()->GetPosition().x,
			m_SceneContext.pCamera->GetTransform()->GetPosition().y,
			m_SceneContext.pCamera->GetTransform()->GetPosition().z,
			m_SceneContext.pCamera->GetTransform()->GetRotation().x,
			m_SceneContext.pCamera->GetTransform()->GetRotation().y,
			m_SceneContext.pCamera->GetTransform()->GetRotation().z);
	}
}

void UberMaterialScene::OnGUI()
{
	ImGui::Checkbox("Rotate Object", &m_RotateObject);
	m_pUberMaterial->DrawImGui();
}

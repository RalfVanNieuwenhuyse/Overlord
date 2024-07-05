#include "stdafx.h"
#include "SummonTestScene.h"
#include "Prefabs/CubePrefab.h"

void SummonTestScene::Initialize()
{
	m_SceneContext.settings.enableOnGUI = true;

}

void SummonTestScene::Update()
{
	if(m_pCube != nullptr)
	{
		auto pos = m_pCube->GetTransform()->GetPosition();
		m_pCube->GetTransform()->Translate(pos.x + 10.f * GetSceneContext().pGameTime->GetElapsed(), pos.y, pos.z);
	}
}

void SummonTestScene::Draw()
{
}

void SummonTestScene::OnGUI()
{
	if (ImGui::Button("summon"))
	{	
		if (m_pCube != nullptr)
		{
			RemoveChild(m_pCube, true);
		}

		m_pCube = new CubePrefab(10.f, 10.f, 10.f, XMFLOAT4{ Colors::Red });
		AddChild(m_pCube);		
	}
}

#include "stdafx.h"
#include "SoftwareSkinningScene_1.h"
#include "Prefabs/BoneObject.h"
#include "Materials/ColorMaterial.h"

void SoftwareSkinningScene_1::Initialize()
{
	m_SceneContext.settings.enableOnGUI = true;

	const auto pMaterial = MaterialManager::Get()->CreateMaterial<ColorMaterial>();
	pMaterial->SetColor({ XMFLOAT4{ Colors::SlateGray } });

	const auto pRoot = new GameObject();

	m_pBone0 = new BoneObject(pMaterial, 15.f);
	pRoot->AddChild(m_pBone0);

	m_pBone1 = new BoneObject(pMaterial, 15.f);
	m_pBone0->AddBone(m_pBone1);
	AddChild(pRoot);
}

void SoftwareSkinningScene_1::Update()
{
	if (m_RotateBones)
	{
		if (m_BoneRotation >= 45 || m_BoneRotation <= -45)
		{
			m_RotationSign *= -1;
		}

		m_BoneRotation += (45 * m_SceneContext.pGameTime->GetElapsed()) * m_RotationSign;
		m_Bone0Rotation = XMFLOAT3{ 0, 0, m_BoneRotation };
		m_Bone1Rotation = XMFLOAT3{ 0, 0,  -m_BoneRotation * 2 };
	}
	m_pBone0->GetTransform()->Rotate(m_Bone0Rotation);
	m_pBone1->GetTransform()->Rotate(m_Bone1Rotation);
}

void SoftwareSkinningScene_1::OnGUI()
{
	ImGui::Checkbox("Rotation Bones", &m_RotateBones);
	ImGui::DragFloat3("Bone 0", reinterpret_cast<float*>(&m_Bone0Rotation), 0.5f, -90, 90);
	ImGui::DragFloat3("Bone 1", reinterpret_cast<float*>(&m_Bone1Rotation), 0.5f, -90, 90);
}
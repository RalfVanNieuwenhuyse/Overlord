#include "stdafx.h"
#include "BoneObject.h"

BoneObject::BoneObject(BaseMaterial* pMaterial, float length)
	:GameObject(),m_pMaterial{pMaterial},m_Length{length}
{
}

void BoneObject::Initialize(const SceneContext&)
{
	GameObject* pEmpty = new GameObject();
	AddChild(pEmpty);

	auto pModel = pEmpty->AddComponent(new ModelComponent(L"Meshes/Bone.ovm"));
	pModel->SetMaterial(m_pMaterial);

	pEmpty->GetTransform()->Rotate(0, -90, 0);
	pEmpty->GetTransform()->Scale(m_Length);
}

void BoneObject::AddBone(BoneObject* pBone)
{
	pBone->GetTransform()->Translate(m_Length, 0, 0);
	AddChild(pBone);
}

void BoneObject::CalculateBindPose()
{
	XMFLOAT4X4 inverse{};
	XMStoreFloat4x4(&inverse, XMMatrixInverse(nullptr, XMLoadFloat4x4(&this->GetTransform()->GetWorld())));

	m_BindPose = inverse;

	for (BoneObject* pChild : GetChildren<BoneObject>())
	{
		pChild->CalculateBindPose();
	}	
}
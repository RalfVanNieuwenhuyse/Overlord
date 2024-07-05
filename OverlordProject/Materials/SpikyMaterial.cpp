#include "stdafx.h"
#include "SpikyMaterial.h"

SpikyMaterial::SpikyMaterial()
	:Material<SpikyMaterial>(L"Effects/SpikyShader.fx")
{
}

void SpikyMaterial::InitializeEffectVariables()
{
	SetVariable_Scalar(L"gSpikeLength", m_SpikeLength);
	SetVariable_Vector(L"m_LightDirection", XMFLOAT3{ 0.577f, 3.577f, 0.577f });
}

#include "stdafx.h"
#include "ParticleEmitterComponent.h"
#include "Misc/ParticleMaterial.h"

ParticleMaterial* ParticleEmitterComponent::m_pParticleMaterial{};

ParticleEmitterComponent::ParticleEmitterComponent(const std::wstring& assetFile, const ParticleEmitterSettings& emitterSettings, UINT particleCount):
	m_ParticlesArray(new Particle[particleCount]),
	m_ParticleCount(particleCount), //How big is our particle buffer?
	m_MaxParticles(particleCount), //How many particles to draw (max == particleCount)
	m_AssetFile(assetFile),
	m_EmitterSettings(emitterSettings)
{
	m_enablePostDraw = true; //This enables the PostDraw function for the component
}

ParticleEmitterComponent::~ParticleEmitterComponent()
{
	delete[] m_ParticlesArray;
	m_pVertexBuffer->Release();
	//TODO_W9(L"Implement Destructor")

}

void ParticleEmitterComponent::Initialize(const SceneContext& sceneContext)
{
	//TODO_W9(L"Implement Initialize")
	if (!m_pParticleMaterial)
	{
		m_pParticleMaterial = MaterialManager::Get()->CreateMaterial<ParticleMaterial>();
	}

	CreateVertexBuffer(sceneContext);
	m_pParticleTexture = ContentManager::Load<TextureData>(m_AssetFile);
}
	

void ParticleEmitterComponent::CreateVertexBuffer(const SceneContext& sceneContext)
{
	//TODO_W9(L"Implement CreateVertexBuffer")
	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
	}
	
	// 2. Create a dynamic vertex buffer
	D3D11_BUFFER_DESC desc{};
	desc.Usage = D3D11_USAGE_DYNAMIC; // Set usage to Dynamic
	desc.ByteWidth = sizeof(VertexParticle) * m_ParticleCount; // ByteWidth based on ParticleCount and size of VertexParticle
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Select appropriate BindFlag for VertexBuffer
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // Enable CPU write access
	desc.MiscFlags = 0; // No MiscFlags

	
	// Create the VertexBuffer
	HRESULT result = sceneContext.d3dContext.pDevice->CreateBuffer(&desc, nullptr, &m_pVertexBuffer);
	if (FAILED(result))
	{
		Logger::LogError(L"Vertex buffer is not created for particle emitter component");
	}
}

void ParticleEmitterComponent::Update(const SceneContext& sceneContext)
{
	//TODO_W9(L"Implement Update")
	float particleInterval = (m_EmitterSettings.maxEnergy - m_EmitterSettings.minEnergy) / static_cast<float>(m_ParticleCount);

	const float elapsedSec = sceneContext.pGameTime->GetElapsed();

	m_LastParticleSpawn += elapsedSec;

	m_ActiveParticles = 0;

	D3D11_MAPPED_SUBRESOURCE subResource{};

	sceneContext.d3dContext.pDeviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);

	VertexParticle* pBuffer = reinterpret_cast<VertexParticle*>(subResource.pData);

	for (unsigned i{}; i < m_ParticleCount; ++i)
	{
		Particle& curParticle = m_ParticlesArray[i];
		if (curParticle.isActive)
			UpdateParticle(curParticle, elapsedSec);


		if (!curParticle.isActive && m_LastParticleSpawn >= particleInterval)
		{
			SpawnParticle(curParticle);
			m_LastParticleSpawn -= particleInterval;
		}

		if (curParticle.isActive)
		{
			pBuffer[m_ActiveParticles] = curParticle.vertexInfo;
			++m_ActiveParticles;
		}
	}

	sceneContext.d3dContext.pDeviceContext->Unmap(m_pVertexBuffer, 0);
}

void ParticleEmitterComponent::UpdateParticle(Particle& p, float elapsedTime) const
{
	//TODO_W9(L"Implement UpdateParticle")
	if (!p.isActive)
		return;

	p.currentEnergy -= elapsedTime;

	if (p.currentEnergy < 0)
	{
		p.isActive = false;
		return;
	}

	XMStoreFloat3(&p.vertexInfo.Position, XMLoadFloat3(&p.vertexInfo.Position) + XMLoadFloat3(&m_EmitterSettings.velocity) * elapsedTime);

	const float lifePercent{ p.currentEnergy / p.totalEnergy };

	p.vertexInfo.Color = m_EmitterSettings.color;

	constexpr float fadeOutDelay{ 2 };

	p.vertexInfo.Color.w *= lifePercent * fadeOutDelay;


	constexpr float unitSize{ 1.0f };
	p.vertexInfo.Size = p.initialSize * (unitSize + (p.sizeChange - unitSize) * (1.0f - lifePercent));
}

void ParticleEmitterComponent::SpawnParticle(Particle& p)
{
	//TODO_W9(L"Implement SpawnParticle")
	p.isActive = true;

	p.totalEnergy = MathHelper::randF(m_EmitterSettings.minEnergy, m_EmitterSettings.maxEnergy);
	p.currentEnergy = p.totalEnergy;

	const XMVECTOR randomDir = XMVector3Normalize({
			MathHelper::randF(-1.0f, 1.0f),
			MathHelper::randF(-1.0f, 1.0f),
			MathHelper::randF(-1.0f, 1.0f)
	});

	const float particalDistance{ MathHelper::randF(m_EmitterSettings.minEmitterRadius, m_EmitterSettings.maxEmitterRadius) };

	XMStoreFloat3(&p.vertexInfo.Position, XMLoadFloat3(&GetTransform()->GetWorldPosition()) + randomDir * particalDistance);

	p.initialSize = MathHelper::randF(m_EmitterSettings.minSize, m_EmitterSettings.maxSize);
	p.vertexInfo.Size = p.initialSize;
	p.sizeChange = MathHelper::randF(m_EmitterSettings.minScale, m_EmitterSettings.maxScale);

	p.vertexInfo.Rotation = MathHelper::randF(-XM_PI, XM_PI);

	p.vertexInfo.Color = m_EmitterSettings.color;
}

void ParticleEmitterComponent::PostDraw(const SceneContext& sceneContext)
{
	//TODO_W9(L"Implement PostDraw")
	m_pParticleMaterial->SetVariable_Matrix(L"gWorldViewProj", sceneContext.pCamera->GetViewProjection());
	m_pParticleMaterial->SetVariable_Matrix(L"gViewInverse", sceneContext.pCamera->GetViewInverse());
	m_pParticleMaterial->SetVariable_Texture(L"gParticleTexture", m_pParticleTexture->GetShaderResourceView());

	const auto& techniqueContext = m_pParticleMaterial->GetTechniqueContext();

	auto pDeviceContext = sceneContext.d3dContext.pDeviceContext;

	pDeviceContext->IASetInputLayout(techniqueContext.pInputLayout);

	pDeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	constexpr UINT offset{};
	constexpr UINT stride{ sizeof(VertexParticle) };

	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	D3DX11_TECHNIQUE_DESC techniqueDesc{};
	techniqueContext.pTechnique->GetDesc(&techniqueDesc);
	for (unsigned i{ 0 }; i < techniqueDesc.Passes; ++i)
	{
		techniqueContext.pTechnique->GetPassByIndex(i)->Apply(0, pDeviceContext);
		pDeviceContext->Draw(m_ActiveParticles, 0);
	}
}

void ParticleEmitterComponent::DrawImGui()
{
	if(ImGui::CollapsingHeader("Particle System"))
	{
		ImGui::SliderUInt("Count", &m_ParticleCount, 0, m_MaxParticles);
		ImGui::InputFloatRange("Energy Bounds", &m_EmitterSettings.minEnergy, &m_EmitterSettings.maxEnergy);
		ImGui::InputFloatRange("Size Bounds", &m_EmitterSettings.minSize, &m_EmitterSettings.maxSize);
		ImGui::InputFloatRange("Scale Bounds", &m_EmitterSettings.minScale, &m_EmitterSettings.maxScale);
		ImGui::InputFloatRange("Radius Bounds", &m_EmitterSettings.minEmitterRadius, &m_EmitterSettings.maxEmitterRadius);
		ImGui::InputFloat3("Velocity", &m_EmitterSettings.velocity.x);
		ImGui::ColorEdit4("Color", &m_EmitterSettings.color.x, ImGuiColorEditFlags_NoInputs);
	}
}
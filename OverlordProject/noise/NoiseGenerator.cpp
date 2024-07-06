#include "stdafx.h"
#include "NoiseGenerator.h"

#include "FastNoise/FastNoise.h"
#include "OverlordAPI.h"

void NoiseGenerator::DrawImGui()
{
	if (ImGui::CollapsingHeader("NoiseMap"))
	{
		ImGui::DragInt2("Map size", reinterpret_cast<int*>(&m_MapSize), 256, 256);
	
		if (ImGui::Button("generate noise map"))
		{			
			auto noiseMap = GenerateNoiseMap(m_MapSize.x, m_MapSize.y);
			auto divice = SceneManager::Get()->GetActiveSceneContext().d3dContext.pDevice;
			m_TextureVieuw = CreateTextureFromImage(divice);
		}
		if (m_TextureVieuw)
		{
			ImGui::Image((void*)m_TextureVieuw.Get(), ImVec2(static_cast<float>(256), static_cast<float>(256)));
		}
	}
}

std::vector<float> NoiseGenerator::GenerateNoiseMap(int width, int height)
{
	m_NoiseMap.clear();

	auto simplex = FastNoise::New<FastNoise::Perlin>();
	auto fractal = FastNoise::New<FastNoise::FractalFBm>();

	fractal->SetSource(simplex);
	fractal->SetOctaveCount(5);

	std::vector<float> noise(width * height);

	fractal->GenUniformGrid2D(noise.data(), 0, 0, width, height, 0.02f, 1337);

	m_NoiseMap = noise;
	m_width = width;
	m_height = height;

	return noise;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> NoiseGenerator::CreateTextureFromImage(ID3D11Device* device)
{
	auto image = ConvertNoiseMapToImage(m_NoiseMap);

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = m_MapSize.x;
	desc.Height = m_MapSize.y;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8_UNORM; // 8-bit single-channel format for grayscale
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = image.data();
	initData.SysMemPitch = m_MapSize.x * sizeof(uint8_t);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	HRESULT hr = device->CreateTexture2D(&desc, &initData, &texture);
	if (FAILED(hr))
	{
		// Handle error
		return nullptr;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> maptextureView;
	hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, &maptextureView);
	if (FAILED(hr))
	{
		// Handle error
		return nullptr;
	}

	m_TextureVieuw = maptextureView;
	return maptextureView;
}

std::vector<uint8_t> NoiseGenerator::ConvertNoiseMapToImage(const std::vector<float>& noiseMap)
{
	std::vector<uint8_t> mapimage(noiseMap.size());
	for (size_t i = 0; i < noiseMap.size(); ++i) {
		mapimage[i] = static_cast<uint8_t>(noiseMap[i] * 255);
	}
	return mapimage;	
}

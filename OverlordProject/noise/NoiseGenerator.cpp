#include "stdafx.h"
#include "NoiseGenerator.h"


#include "OverlordAPI.h"

const char* noiseTypeNames[] = {
	"Perlin",
	"Simplex",
	"OpenSimplex2",
	"OpenSimplex2S",
	"CellularDistance",
	"CellularValue",
	"Value"
};

void NoiseGenerator::DrawImGui()
{
	if (ImGui::CollapsingHeader("NoiseMap"), ImGuiTreeNodeFlags_DefaultOpen)
	{
		bool valueChanged = false;

		valueChanged |= ImGui::DragInt2("Map size", reinterpret_cast<int*>(&m_MapSize),1.f,64,INT_MAX);
		if (valueChanged)
		{
			if (m_MapSize.x < 64) m_MapSize.x = 64;
			if (m_MapSize.y < 64) m_MapSize.y = 64;
		}
	
		valueChanged |= ImGui::DragFloat("Scale", reinterpret_cast<float*>(&m_Scale));
		valueChanged |= ImGui::DragInt2("Offset", reinterpret_cast<int*>(&m_Offset));

		

		int currentNoiseTypeIndex = static_cast<int>(M_NoiseType);

		if (ImGui::Combo("Noise Type", &currentNoiseTypeIndex, noiseTypeNames, IM_ARRAYSIZE(noiseTypeNames)))
		{
			valueChanged |= true;
			M_NoiseType = static_cast<NoiseType>(currentNoiseTypeIndex);
		}

		valueChanged |= ImGui::DragInt("Seed", reinterpret_cast<int*>(&m_Seed));

		valueChanged |= ImGui::DragInt("Octaves", reinterpret_cast<int*>(&m_Octaves),1.f,0, INT_MAX);
		if (valueChanged)
		{
			if (m_Octaves < 0) m_Octaves = 0;
			
		}

		valueChanged |= ImGui::DragFloat("Gain", reinterpret_cast<float*>(&m_Gain),0.01f);
		valueChanged |= ImGui::DragFloat("Lacunarity", reinterpret_cast<float*>(&m_Lacunarity), 0.01f);
			

		valueChanged |= ImGui::Checkbox("Auto generate on change", &m_AutoGen);
		if (ImGui::Button("generate noise map")|| (valueChanged&& m_AutoGen))
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

	FastNoise::SmartNode<FastNoise::Generator> noiseType = FastNoise::New<FastNoise::Perlin>();
	switch (M_NoiseType)
	{
	case NoiseGenerator::NoiseType::Perlin:
		noiseType = FastNoise::New<FastNoise::Perlin>();
		break;
	case NoiseGenerator::NoiseType::Simplex:
		noiseType = FastNoise::New<FastNoise::Simplex>();
		break;
	case NoiseGenerator::NoiseType::OpenSimplex2:
		noiseType = FastNoise::New<FastNoise::OpenSimplex2>();
		break;
	case NoiseGenerator::NoiseType::OpenSimplex2S:
		noiseType = FastNoise::New<FastNoise::OpenSimplex2S>();
		break;	
	case NoiseGenerator::NoiseType::CellularDistance:
		noiseType = FastNoise::New<FastNoise::CellularDistance>();
		break;	
	case NoiseGenerator::NoiseType::CellularValue:
		noiseType = FastNoise::New<FastNoise::CellularValue>();
		break;
	case NoiseGenerator::NoiseType::Value:
		noiseType = FastNoise::New<FastNoise::Value>();
		break;
	default:
		break;
	}

	auto fractal = FastNoise::New<FastNoise::FractalFBm>();

	fractal->SetSource(noiseType);
	fractal->SetOctaveCount(m_Octaves);
	fractal->SetGain(m_Gain);
	fractal->SetLacunarity(m_Lacunarity);
	
	std::vector<float> noise(width * height);

	fractal->GenUniformGrid2D(noise.data(), m_Offset.x, m_Offset.y, width, height, 1.f/ m_Scale, m_Seed);

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

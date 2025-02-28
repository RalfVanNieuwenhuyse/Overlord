#include "stdafx.h"
#include "NoiseGenerator.h"


#include "OverlordAPI.h"
#include <fstream>

//#include "ImGui_Curve.h"


const char* noiseTypeNames[] = {
	"Perlin",
	"Simplex",
	"OpenSimplex2",
	"OpenSimplex2S",
	"CellularDistance",
	"CellularValue",
	"Value"
};

NoiseGenerator::NoiseGenerator()
{
	LoadSettings(m_SettingsFileName);
}

NoiseGenerator::~NoiseGenerator()
{
	SaveSettings(m_SettingsFileName);
	m_TextureVieuw.ReleaseAndGetAddressOf();
}

void NoiseGenerator::DrawImGui()
{
	m_ValueChanged = false;
	if (ImGui::CollapsingHeader("NoiseMap"), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed)
	{		

		m_ValueChanged |= ImGui::DragInt2("Map size", reinterpret_cast<int*>(&m_MapSize),1.f,64,INT_MAX);
		if (m_ValueChanged)
		{
			if (m_MapSize.x < 64) m_MapSize.x = 64;
			if (m_MapSize.y < 64) m_MapSize.y = 64;
		}
	
		m_ValueChanged |= ImGui::DragFloat("Scale", reinterpret_cast<float*>(&m_Scale));
		m_ValueChanged |= ImGui::DragInt2("Offset", reinterpret_cast<int*>(&m_Offset));

		int currentNoiseTypeIndex = static_cast<int>(M_NoiseType);

		if (ImGui::Combo("Noise Type", &currentNoiseTypeIndex, noiseTypeNames, IM_ARRAYSIZE(noiseTypeNames)))
		{
			m_ValueChanged |= true;
			M_NoiseType = static_cast<NoiseType>(currentNoiseTypeIndex);
		}

		m_ValueChanged |= ImGui::DragInt("Seed", reinterpret_cast<int*>(&m_Seed));

		m_ValueChanged |= ImGui::DragInt("Octaves", reinterpret_cast<int*>(&m_Octaves),1.f,0, INT_MAX);
		if (m_ValueChanged)
		{
			if (m_Octaves < 0) m_Octaves = 0;
			
		}

		m_ValueChanged |= ImGui::DragFloat("Gain", reinterpret_cast<float*>(&m_Gain),0.01f);
		m_ValueChanged |= ImGui::DragFloat("Lacunarity", reinterpret_cast<float*>(&m_Lacunarity), 0.01f);
			

		m_ValueChanged |= ImGui::Checkbox("Auto generate noise map on change", &m_AutoGen);
		if (ImGui::Button("generate noise map")|| (m_ValueChanged && m_AutoGen))
		{	
			m_ValueChanged |= true;
			Generate();
		}
		if (m_TextureVieuw)
		{
			ImGui::Image((void*)m_TextureVieuw.Get(), ImVec2(static_cast<float>(256), static_cast<float>(256)));
		}
	}
}

void NoiseGenerator::Generate()
{
	auto noiseMap = GenerateNoiseMap(m_MapSize.x, m_MapSize.y);
	auto divice = SceneManager::Get()->GetActiveSceneContext().d3dContext.pDevice;
	m_TextureVieuw = CreateTextureFromImage(divice);
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

	// Scale the values from [-1, 1] to [0, 1]
	for (auto& value : noise)
	{
		value = (value + 1.0f) / 2.0f;
	}

	m_NoiseMap = noise;
	m_width = width;
	m_height = height;

	return noise;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> NoiseGenerator::CreateTextureFromImage(ID3D11Device* device)
{
	auto image = ConvertNoiseMapToImageRGBA(m_NoiseMap);//rgba for greyscale
	//auto image = ConvertNoiseMapToImage(m_NoiseMap);//redscale

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = m_MapSize.x;
	desc.Height = m_MapSize.y;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //rgba for greyscale
	//desc.Format = DXGI_FORMAT_R8_UNORM; // 8-bit single-channel format for redScale
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = image.data();
	initData.SysMemPitch = m_MapSize.x * 4 * sizeof(uint8_t);//rgba for greyscale
	//initData.SysMemPitch = m_MapSize.x * sizeof(uint8_t); //redscale


	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	HRESULT hr = device->CreateTexture2D(&desc, &initData, &texture);
	if (FAILED(hr))
	{
		// Add logging or error handling here
		OutputDebugStringA("Failed to create texture\n");
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
		// Add logging or error handling here
		OutputDebugStringA("Failed to create shader resource view\n");
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

std::vector<uint8_t> NoiseGenerator::ConvertNoiseMapToImageRGBA(const std::vector<float>& noiseMap)
{
	std::vector<uint8_t> mapimage(noiseMap.size() * 4);
	for (size_t i = 0; i < noiseMap.size(); ++i) {
		uint8_t value = static_cast<uint8_t>(noiseMap[i] * 255);
		mapimage[i * 4] = value;
		mapimage[i * 4 + 1] = value;
		mapimage[i * 4 + 2] = value;
		mapimage[i * 4 + 3] = 255;
	}
	return mapimage;
}

void NoiseGenerator::LoadSettings(const std::string& filename)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open())
	{
		Logger::LogError(L"Settings file not open");
		return;
	}

	file.read(reinterpret_cast<char*>(&m_MapSize), sizeof(m_MapSize));
	file.read(reinterpret_cast<char*>(&m_Scale), sizeof(m_Scale));
	file.read(reinterpret_cast<char*>(&m_Offset), sizeof(m_Offset));
	file.read(reinterpret_cast<char*>(&M_NoiseType), sizeof(M_NoiseType));
	file.read(reinterpret_cast<char*>(&m_Seed), sizeof(m_Seed));
	file.read(reinterpret_cast<char*>(&m_Octaves), sizeof(m_Octaves));
	file.read(reinterpret_cast<char*>(&m_Gain), sizeof(m_Gain));
	file.read(reinterpret_cast<char*>(&m_Lacunarity), sizeof(m_Lacunarity));
	file.read(reinterpret_cast<char*>(&m_AutoGen), sizeof(m_AutoGen));
}

void NoiseGenerator::SaveSettings(const std::string& filename)
{
	std::ofstream file(filename, std::ios::binary);
	if (!file.is_open())
    {
        Logger::LogError(L"Settings file not open");
        return;
    } 

	file.write(reinterpret_cast<const char*>(&m_MapSize), sizeof(m_MapSize));
	file.write(reinterpret_cast<const char*>(&m_Scale), sizeof(m_Scale));
	file.write(reinterpret_cast<const char*>(&m_Offset), sizeof(m_Offset));
	file.write(reinterpret_cast<const char*>(&M_NoiseType), sizeof(M_NoiseType));
	file.write(reinterpret_cast<const char*>(&m_Seed), sizeof(m_Seed));
	file.write(reinterpret_cast<const char*>(&m_Octaves), sizeof(m_Octaves));
	file.write(reinterpret_cast<const char*>(&m_Gain), sizeof(m_Gain));
	file.write(reinterpret_cast<const char*>(&m_Lacunarity), sizeof(m_Lacunarity));
	file.write(reinterpret_cast<const char*>(&m_AutoGen), sizeof(m_AutoGen));
}

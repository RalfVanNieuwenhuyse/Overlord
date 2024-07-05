#include "stdafx.h"
#include "SummonTestScene.h"
#include "Prefabs/CubePrefab.h"
#include "FastNoise/FastNoise.h"

void SummonTestScene::Initialize()
{
	m_SceneContext.settings.enableOnGUI = true;

	auto simplex = FastNoise::New<FastNoise::Simplex>();
	auto fractal = FastNoise::New<FastNoise::FractalFBm>();

	fractal->SetSource(simplex);
	fractal->SetOctaveCount(5);

	std::vector<float> noise(16 * 16);

	fractal->GenUniformGrid2D(noise.data(), 0, 0, 16, 16, 0.02f, 1337);

	int index = 0;

	for (int y = 0; y < 16; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			std::cout << "x " << x << "\ty " << y << "\t: " << noise[index++] << std::endl;
		}
	}
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

    if (ImGui::Button("gen")) {
        auto noiseMap = GenerateNoiseMap(width, height);
        image = ConvertNoiseMapToImage(noiseMap, width, height);

        textureView = CreateTextureFromImage(GetSceneContext().d3dContext.pDevice, image, width, height);
    }

    if (textureView) {
       
        ImGui::Image((void*)textureView.Get(), ImVec2(static_cast<float>(width), static_cast<float>(height)));
        
    }
}


std::vector<float> SummonTestScene::GenerateNoiseMap(int mapwidth, int mapheight)
{    
	auto simplex = FastNoise::New<FastNoise::Simplex>();
	auto fractal = FastNoise::New<FastNoise::FractalFBm>();

	fractal->SetSource(simplex);
	fractal->SetOctaveCount(5);

	std::vector<float> noise(mapwidth * mapheight);

	fractal->GenUniformGrid2D(noise.data(), 0, 0, mapwidth, mapheight, 0.02f, 1337);
    return noise;
}

std::vector<uint8_t> SummonTestScene::ConvertNoiseMapToImage(const std::vector<float>& noiseMap, int mapwidth, int mapheight) {
    std::vector<uint8_t> mapimage(mapwidth * mapheight);
    for (size_t i = 0; i < noiseMap.size(); ++i) {
        mapimage[i] = static_cast<uint8_t>(noiseMap[i] * 255);
    }
    return mapimage;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SummonTestScene::CreateTextureFromImage(ID3D11Device* device, const std::vector<uint8_t>& mapimage, int mapwidth, int mapheight)
{
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = mapwidth;
    desc.Height = mapheight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = mapimage.data();
    initData.SysMemPitch = width * sizeof(uint8_t);

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    HRESULT hr = device->CreateTexture2D(&desc, &initData, &texture);
    if (FAILED(hr)) 
    {
        Logger::LogInfo(L"hello there 1");
        // Handle error
        return nullptr;        
    }

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> maptextureView;
    hr = device->CreateShaderResourceView(texture.Get(), nullptr, &maptextureView);
    if (FAILED(hr)) 
    {
        Logger::LogInfo(L"hello there02");
        // Handle error
        return nullptr;
        
    }

    Logger::LogInfo(L"hello there03");

    return maptextureView;
}

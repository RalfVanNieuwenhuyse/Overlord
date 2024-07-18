#pragma once

#include <vector>
#include <cstdint>
#include <d3d11.h>
#include <wrl/client.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include "FastNoise/FastNoise.h"




class NoiseGenerator
{	
public:
	NoiseGenerator() = default;
	~NoiseGenerator();

	NoiseGenerator(const NoiseGenerator& other) = delete;
	NoiseGenerator(NoiseGenerator&& other) noexcept = delete;
	NoiseGenerator& operator=(const NoiseGenerator& other) = delete;
	NoiseGenerator& operator=(NoiseGenerator&& other) noexcept = delete;

	void DrawImGui();

	void Generate();

	std::vector<float> GetNoiseMap() const { return m_NoiseMap; };
	XMINT2 GetMapSize() const { return m_MapSize; };
	bool GetValueChanged() const { return m_ValueChanged; }
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetTexture() const { return m_TextureVieuw; };

	std::vector<float> GenerateNoiseMap(int width, int height);
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateTextureFromImage(ID3D11Device* device);

	enum class NoiseType
	{
		Perlin,
		Simplex,
		OpenSimplex2,
		OpenSimplex2S,
		CellularDistance,
		CellularValue,
		Value
	};

private:
	bool m_AutoGen{false};
	std::vector<float> m_NoiseMap;
	int m_width = 256;
	int m_height = 256;
	float m_Scale{100.f};
	XMINT2 m_MapSize{ 1024, 1024 };
	XMINT2 m_Offset{ 0, 0 };
	NoiseType M_NoiseType{NoiseType::Perlin};
	int m_Seed{ 20 };

	int m_Octaves{ 5 };
	float m_Gain{.5f};
	float m_Lacunarity{2.f};

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_TextureVieuw;

	bool m_ValueChanged{false};

	std::vector<uint8_t> ConvertNoiseMapToImage(const std::vector<float>& noiseMap);
};


#pragma once

#include <vector>
#include <cstdint>
#include <d3d11.h>
#include <wrl/client.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

class NoiseGenerator
{
public:
	NoiseGenerator() = default;
	~NoiseGenerator() = default;

	NoiseGenerator(const NoiseGenerator& other) = delete;
	NoiseGenerator(NoiseGenerator&& other) noexcept = delete;
	NoiseGenerator& operator=(const NoiseGenerator& other) = delete;
	NoiseGenerator& operator=(NoiseGenerator&& other) noexcept = delete;

	void DrawImGui();

	std::vector<float> GetNoiseMap() const { return m_NoiseMap; };
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetTexture() const { return m_TextureVieuw; };

	std::vector<float> GenerateNoiseMap(int width, int height);
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateTextureFromImage(ID3D11Device* device);


private:
	std::vector<float> m_NoiseMap;
	int m_width = 256;
	int m_height = 256;
	XMINT2 m_MapSize{ 256, 256 };

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_TextureVieuw;

	std::vector<uint8_t> ConvertNoiseMapToImage(const std::vector<float>& noiseMap);
};

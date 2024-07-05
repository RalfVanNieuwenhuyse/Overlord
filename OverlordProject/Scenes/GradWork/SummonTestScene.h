#pragma once
#include <vector>
#include <cstdint>
#include <d3d11.h>
#include <wrl/client.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

class CubePrefab;
class SummonTestScene : public GameScene
{
public:
	SummonTestScene() : GameScene(L"SummonTestScene") {};
	~SummonTestScene() override = default;

	SummonTestScene(const SummonTestScene& other) = delete;
	SummonTestScene(SummonTestScene&& other) noexcept = delete;
	SummonTestScene& operator=(const SummonTestScene& other) = delete;
	SummonTestScene& operator=(SummonTestScene&& other) noexcept = delete;

protected:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void OnGUI() override;

private:
	CubePrefab* m_pCube{ nullptr };


	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;
	std::vector<uint8_t> image;
	int width = 256;
	int height = 256;

	std::vector<float> GenerateNoiseMap(int width, int height);
	std::vector<uint8_t> ConvertNoiseMapToImage(const std::vector<float>& noiseMap, int width, int height);
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateTextureFromImage(ID3D11Device* device, const std::vector<uint8_t>& image, int width, int height);

};

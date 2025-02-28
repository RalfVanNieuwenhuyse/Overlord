#pragma once
#include <vector>
#include <cstdint>
#include <d3d11.h>
#include <wrl/client.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <memory>

#include "noise/TerrainGenrerator.h"

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
	std::unique_ptr<TerrainGenrerator> m_TerrainGen;
};

#pragma once

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

};

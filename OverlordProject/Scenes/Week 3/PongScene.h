#pragma once

class SpherePrefab;
class CubePrefab;

class PongScene : public GameScene
{
public:
	PongScene() : GameScene(L"PongScene") {};;
	~PongScene() override = default;

	PongScene(const PongScene& other) = delete;
	PongScene(PongScene&& other) noexcept = delete;
	PongScene& operator=(const PongScene& other) = delete;
	PongScene& operator=(PongScene&& other) noexcept = delete;

protected:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void OnGUI() override;

private:
	FixedCamera* m_pCamera{ nullptr };
	
	SpherePrefab* m_pBall{};

	bool m_IsGameActive{ false };

	CubePrefab* m_pPeddleLeft{ nullptr };
	CubePrefab* m_pPeddleRight{ nullptr };

	GameObject* m_pTriggerLeft{nullptr};
	GameObject* m_pTriggerRight{ nullptr };

	float m_PeddleminZPos{};
	float m_PeddlemaxZPos{};

	//helper function
	void ResetGame();

};


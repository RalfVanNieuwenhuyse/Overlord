#pragma once

class UberMaterial;
class UberMaterialScene : public GameScene
{
public:

	UberMaterialScene() :GameScene(L"UberMaterialScene") {};
	~UberMaterialScene() override = default;

	UberMaterialScene(const  UberMaterialScene& other) = delete;
	UberMaterialScene(UberMaterialScene&& other) noexcept = delete;
	UberMaterialScene& operator=(const  UberMaterialScene& other) = delete;
	UberMaterialScene& operator=(UberMaterialScene&& other) noexcept = delete;

protected:
	void Initialize() override;

	void Update() override;

	void OnGUI() override;

private:
	UberMaterial* m_pUberMaterial{ nullptr };
	bool m_RotateObject{ true };
	GameObject* m_pSphere;
};


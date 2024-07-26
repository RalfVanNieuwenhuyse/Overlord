#pragma once
#include <memory>

class MeshGenerator
{
public:
	MeshGenerator();
	~MeshGenerator();

	MeshGenerator(const MeshGenerator& other) = delete;
	MeshGenerator(MeshGenerator&& other) noexcept = delete;
	MeshGenerator& operator=(const MeshGenerator& other) = delete;
	MeshGenerator& operator=(MeshGenerator&& other) noexcept = delete;

	bool DrawImGui();
	MeshIndexedDrawComponent* GenerateMesh(int width, int height, const std::vector<float>& heightmap);

	void SetMapSize(const XMINT2& mapSize) { m_MapSize = mapSize; };
	void SetHeightMap(const std::vector<float>& heightmap) { m_HeightMap = heightmap; };

	bool GetValueChanged() const { return m_ValueChanged; }

	void Generate();

	void LoadSettings(const std::string& filename);
	void SaveSettings(const std::string& filename);

private:
	float m_HeightModifier{10.f};
	float m_HeightCurve[5] = { 0.950f, 0.050f, 0.795f, 0.035f };
	XMFLOAT4 m_Color{ 0.501960814f, 0.501960814f, 0.501960814f, 1.f };

	bool m_AutoGenMesh{ false };

	XMINT2 m_MapSize{ 0, 0 };
	std::vector<float> m_HeightMap;

	GameObject* m_Terrain;
	MeshIndexedDrawComponent* m_Mesh;
	XMFLOAT3 m_Scale{ 1, 1, 1 };


	bool m_ValueChanged{ false };

	bool m_HasGeneratedOnec{ false };
	std::string m_FileName{"Terrain"};

	std::string m_SettingsFileName{ "MeshSettings.bin" };

};
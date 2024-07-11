#pragma once
//#include <memory>
#include "noise/NoiseGenerator.h"
#include "noise/MeshGenerator.h"

//class MeshGenerator;
//class NoiseGenerator;

class TerrainGenrerator
{
public:
	TerrainGenrerator();
	~TerrainGenrerator() = default;

	TerrainGenrerator(const TerrainGenrerator& other) = delete;
	TerrainGenrerator(TerrainGenrerator&& other) noexcept = delete;
	TerrainGenrerator& operator=(const TerrainGenrerator& other) = delete;
	TerrainGenrerator& operator=(TerrainGenrerator&& other) noexcept = delete;

	void DrawImGui();
private:
	NoiseGenerator m_NoiseGen;
	MeshGenerator m_MeshGen;

	//GameObject* m_Terrain;
	bool m_AutoGenTerain{ false };

	void GenerateTerrain();

};


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
    int m_TimesToGenerate{10};

    float m_AverageTime = 0.0f;

	void GenerateTerrain();

};


#include <numeric>
class Timer
{
public:
    Timer(){}        
    ~Timer(){}
    void Start()
    {
        m_MeasureStart = std::chrono::high_resolution_clock::now();
    }

    float Endms()//gives elapsed time back
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = end - m_MeasureStart;
        auto elapsedms = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());

        m_Timings.push_back(elapsedms);
        return elapsedms;
    }

    void EndPrintms()//prits elapsed time back
    {
        std::cout << "Time: " << Endms() << "ms\n";
    }

    float AverageTimings()
    {
        auto minmax = std::minmax_element(m_Timings.begin(), m_Timings.end());
        auto sum = std::accumulate(m_Timings.begin(), m_Timings.end(), 0.0f) - (*minmax.first) - (*minmax.second);
        auto averageDuration = sum / (m_Timings.size() - 2);

        return averageDuration;
    }

    void PrintAverage()
    {
        std::cout << "Average time: " << AverageTimings() << "ms\n\n";
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_MeasureStart{};
    std::vector<float> m_Timings{};    
};
#include "stdafx.h"
#include "TerrainGenrerator.h"

#include <thread>
#include <atomic>

#include <fstream>
#include <iomanip> // For std::put_time
#include <chrono> // For std::chrono
#include <iostream> // For std::cerr

TerrainGenrerator::TerrainGenrerator()
{
	m_NoiseGen = std::make_unique<NoiseGenerator>();
	m_MeshGen = std::make_unique<MeshGenerator>();
}

void TerrainGenrerator::DrawImGui()
{
	bool valueChanged = false;

	if (ImGui::CollapsingHeader("Terrain generator"), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed)
	{
		valueChanged |= m_NoiseGen->GetValueChanged();

		if (valueChanged && !m_NoiseGen->GetNoiseMap().empty())
		{
			m_MeshGen->SetMapSize(m_NoiseGen->GetMapSize());
			m_MeshGen->SetHeightMap(m_NoiseGen->GetNoiseMap());
		}

		valueChanged |= m_MeshGen->GetValueChanged();

		m_NoiseGen->DrawImGui();
		m_MeshGen->DrawImGui();


		if (ImGui::CollapsingHeader("Generator"), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed)
		{
			valueChanged |= ImGui::Checkbox("Auto generate terrain on change", &m_AutoGenTerain);

			if (ImGui::Button("generate Terrain") || (valueChanged && m_AutoGenTerain))
			{
				Timer time;
				time.Start();
				GenerateTerrain();
				m_AverageTime = time.Endms();
				SaveTiming(m_AverageTime);
			}

			ImGui::DragInt("Times to generate", &m_TimesToGenerate, 1.f, 2, INT_MAX);
			if (m_TimesToGenerate < 2)
			{
				m_TimesToGenerate = 2;
			}

			std::string textbutton;
			textbutton += "generate Terrain ";
			textbutton += std::to_string(m_TimesToGenerate);
			textbutton += " times";



			if (ImGui::Button(textbutton.c_str()))
			{
				Timer time;
				for (int i = 0; i < m_TimesToGenerate; i++)
				{
					time.Start();
					GenerateTerrain();
					time.Endms();
				}
				m_AverageTime = time.AverageTimings();
				SaveTiming(m_AverageTime);
			}

			ImGui::Text("Average Time: %.3f ms", m_AverageTime);
		}
	}
}

void TerrainGenrerator::GenerateTerrain()
{
	m_NoiseGen->Generate();

	m_MeshGen->SetMapSize(m_NoiseGen->GetMapSize());
	m_MeshGen->SetHeightMap(m_NoiseGen->GetNoiseMap());

	m_MeshGen->Generate();
}

void TerrainGenrerator::SaveTiming(float averageTime)
{
	std::ofstream outFile("timings.txt", std::ios_base::app);
	if (outFile.is_open())
	{
		auto now = std::chrono::system_clock::now();
		auto now_c = std::chrono::system_clock::to_time_t(now);

		struct tm buf;
		localtime_s(&buf, &now_c);

		outFile << "Generated at " << std::put_time(&buf, "%F %T")
			<< " - Noise Type: " << NoiseTypeToString(m_NoiseGen->GetNoiseType())
			<< " - Time: " << averageTime << " ms\n";
		outFile.close();
	}
	else
	{
		std::cerr << "Unable to open file for writing timings\n";
	}
}

std::string TerrainGenrerator::NoiseTypeToString(NoiseGenerator::NoiseType type)
{
	switch (type)
	{
	case NoiseGenerator::NoiseType::Perlin: return "Perlin";
	case NoiseGenerator::NoiseType::Simplex: return "Simplex";
	case NoiseGenerator::NoiseType::OpenSimplex2: return "OpenSimplex2";
	case NoiseGenerator::NoiseType::OpenSimplex2S: return "OpenSimplex2S";
	case NoiseGenerator::NoiseType::CellularDistance: return "CellularDistance";
	case NoiseGenerator::NoiseType::CellularValue: return "CellularValue";
	case NoiseGenerator::NoiseType::Value: return "Value";
	default: return "Unknown";
	}
}

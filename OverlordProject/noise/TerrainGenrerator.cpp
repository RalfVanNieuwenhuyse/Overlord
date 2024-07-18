#include "stdafx.h"
#include "TerrainGenrerator.h"

#include <thread>
#include <atomic>

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
		valueChanged |= m_MeshGen->GetValueChanged();

		m_NoiseGen->DrawImGui();
		m_MeshGen->DrawImGui();


		if (ImGui::CollapsingHeader("Generator"), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed)
		{
			valueChanged |= ImGui::Checkbox("Auto generate terrain on change", &m_AutoGenTerain);

			if (ImGui::Button("generate Terrain") || (valueChanged && m_AutoGenTerain))
			{
				/*m_GenerationInProgress = true;
				std::thread(&TerrainGenrerator::GenerateTerrain, this).detach();*/
				Timer time;
				time.Start();
				GenerateTerrain();
				m_AverageTime = time.Endms();
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

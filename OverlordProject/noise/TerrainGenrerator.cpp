#include "stdafx.h"
#include "TerrainGenrerator.h"


TerrainGenrerator::TerrainGenrerator()
{
	//m_NoiseGen;
	//m_MeshGen;
}

void TerrainGenrerator::DrawImGui()
{
	bool valueChanged = false;

	if (ImGui::CollapsingHeader("Terrain generator"), ImGuiTreeNodeFlags_DefaultOpen)
	{
		valueChanged |= m_NoiseGen.GetValueChanged();
		valueChanged |= m_MeshGen.GetValueChanged();

		m_NoiseGen.DrawImGui();
		m_MeshGen.DrawImGui();

		valueChanged |= ImGui::Checkbox("Auto generate terrain on change", &m_AutoGenTerain);

		if (ImGui::Button("generate Terrain") || (valueChanged && m_AutoGenTerain))
		{
			GenerateTerrain();
		}
	}
}

void TerrainGenrerator::GenerateTerrain()
{
	m_NoiseGen.Generate();

	m_MeshGen.SetMapSize(m_NoiseGen.GetMapSize());
	m_MeshGen.SetHeightMap(m_NoiseGen.GetNoiseMap());

	m_MeshGen.Generate();
}

#include "stdafx.h"
#include "SummonTestScene.h"
#include "Prefabs/CubePrefab.h"
#include "FastNoise/FastNoise.h"


void SummonTestScene::Initialize()
{
	m_SceneContext.settings.enableOnGUI = true;	
	m_TerrainGen = std::make_unique<TerrainGenrerator>();
}

void SummonTestScene::Update()
{
}

void SummonTestScene::Draw()
{
	
}

void SummonTestScene::OnGUI()
{	

	m_TerrainGen->DrawImGui();
}
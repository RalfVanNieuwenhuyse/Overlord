#include "stdafx.h"
#include "MeshGenerator.h"
#include <vector>

#include <execution>
#include <vector>
#include <algorithm>

#include "ImGui_Curve.h"

MeshGenerator::MeshGenerator()
{
    m_Terrain = new GameObject();
    m_Mesh = new MeshIndexedDrawComponent(1024*1024, 1024*1024 * 6);
    m_Terrain->AddComponent(m_Mesh);
    /*auto scene = SceneManager::Get()->GetActiveScene();
    scene->AddChild(m_Terrain);*/
}

bool MeshGenerator::DrawImGui()
{
    m_ValueChanged = false;
    if (ImGui::CollapsingHeader("Mesh"), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed)
    {        

        m_ValueChanged |= ImGui::DragFloat("Height modifier", reinterpret_cast<float*>(&m_HeightModifier));
        m_ValueChanged |= static_cast<bool>(ImGui::Bezier("easeInExpo", m_HeightCurve));

        m_ValueChanged |= ImGui::Checkbox("Auto generate mesh on change", &m_AutoGenMesh);

        ImGui::DragFloat3("Scale mesh", reinterpret_cast<float*>(&m_Scale), 0.5f);
        m_Terrain->GetTransform()->Scale(m_Scale);

        if (ImGui::Button("generate mesh") || (m_ValueChanged && m_AutoGenMesh))
        {
            Generate();
        }
    }
    return m_ValueChanged;
}

MeshIndexedDrawComponent* MeshGenerator::GenerateMesh(int width, int height, const std::vector<float>& heightmap)
{
    if (width <= 0)
    {
        Logger::LogError(L"Width is 0");
        return nullptr;
    }
    if (height <= 0)
    {
        Logger::LogError(L"Height is 0");
        return nullptr;
    }
    if (heightmap.empty())
    {
        Logger::LogError(L"Heightmap is empty");
        return nullptr;
    }
    
    m_Mesh->ClearVertexList();
    m_Mesh->ClearIndexList();
    const int vertCount = width * height;
    const int quadCount = (width - 1) * (height - 1);
    
    m_Mesh = new MeshIndexedDrawComponent(vertCount, quadCount * 6);

    XMFLOAT3 normal(0, 1, 0); // Placeholder normal
    // Vertices
    for (int z = 0; z < height; ++z)
    {
        for (int x = 0; x < width; ++x)
        {
            const int index = z * width + x;
            const float heightValue = heightmap[index];
            const float curveHeight = ImGui::BezierValue(heightValue, m_HeightCurve);
            const float finalHeight = curveHeight * m_HeightModifier;

            XMFLOAT3 pos(static_cast<float>(x), finalHeight, static_cast<float>(z));            

            m_Mesh->AddVertex(VertexPosNormCol(pos, normal, m_Color), false);
        }
    }

    // Indices
    for (int z = 0; z < height - 1; ++z)
    {
        for (int x = 0; x < width - 1; ++x)
        {
            const int v0 = z * width + x;
            const int v1 = v0 + 1;
            const int v2 = v0 + width;
            const int v3 = v2 + 1;

            // First triangle
            m_Mesh->AddIndex(v0, false);
            m_Mesh->AddIndex(v1, false);
            m_Mesh->AddIndex(v2, false);

            // Second triangle
            m_Mesh->AddIndex(v2, false);
            m_Mesh->AddIndex(v1, false);
            m_Mesh->AddIndex(v3, false);
        }
    }

    // Update buffers and generate normals
    //pMesh->UpdateVertexBuffer();
    //pMesh->UpdateIndexBuffer();
    m_Mesh->GenerateNormals();

    return m_Mesh;
}

void MeshGenerator::Generate()
{
    auto scene = SceneManager::Get()->GetActiveScene();
    
    if (m_Terrain->GetScene() != nullptr)
    {
        scene->RemoveChild(m_Terrain, true);
    }

    GenerateMesh(m_MapSize.x, m_MapSize.y, m_HeightMap);
    m_Terrain = new GameObject();
    m_Terrain->AddComponent(m_Mesh);
    scene->AddChild(m_Terrain);
}

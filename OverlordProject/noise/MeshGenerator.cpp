#include "stdafx.h"
#include "MeshGenerator.h"
#include <vector>

#include <execution>
#include <vector>
#include <algorithm>
#include <future>
#include <atomic>

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
    std::vector<VertexPosNormCol> tempVertices(vertCount);
    std::vector<uint32_t> tempIndices(quadCount * 6);

    // Populate vertices in parallel
    auto vertexWorker = [&](int start, int end) {
        for (int z = start; z < end; ++z)
        {
            for (int x = 0; x < width; ++x)
            {
                const int index = z * width + x;
                const float heightValue = heightmap[index];
                const float curveHeight = ImGui::BezierValue(heightValue, m_HeightCurve);
                const float finalHeight = curveHeight * m_HeightModifier;

                XMFLOAT3 pos(static_cast<float>(x), finalHeight, static_cast<float>(z));
                tempVertices[index] = VertexPosNormCol(pos, normal, m_Color);
            }
        }
        };

    // Populate indices in parallel
    auto indexWorker = [&](int start, int end) {
        for (int z = start; z < end; ++z)
        {
            for (int x = 0; x < width - 1; ++x)
            {
                const int v0 = z * width + x;
                const int v1 = v0 + 1;
                const int v2 = v0 + width;
                const int v3 = v2 + 1;
                const int index = (z * (width - 1) + x) * 6;

                // First triangle
                tempIndices[index] = v0;
                tempIndices[index + 1] = v1;
                tempIndices[index + 2] = v2;

                // Second triangle
                tempIndices[index + 3] = v2;
                tempIndices[index + 4] = v1;
                tempIndices[index + 5] = v3;
            }
        }
        };

    int numThreads = std::thread::hardware_concurrency();
    int rowsPerThread = height / numThreads;
    int startRow = 0;
    std::vector<std::future<void>> futures;

    // Launch vertex workers
    for (int i = 0; i < numThreads; ++i)
    {
        int endRow = (i == numThreads - 1) ? height : startRow + rowsPerThread;
        futures.emplace_back(std::async(std::launch::async, vertexWorker, startRow, endRow));
        startRow = endRow;
    }

    // Launch index workers
    startRow = 0;
    for (int i = 0; i < numThreads; ++i)
    {
        int endRow = (i == numThreads - 1) ? height - 1 : startRow + rowsPerThread;
        futures.emplace_back(std::async(std::launch::async, indexWorker, startRow, endRow));
        startRow = endRow;
    }

    // Wait for all tasks to complete
    for (auto& future : futures)
    {
        future.get();
    }

    // Add vertices and indices to the mesh
    for (const auto& vertex : tempVertices)
    {
        m_Mesh->AddVertex(vertex, false);
    }

    for (const auto& index : tempIndices)
    {
        m_Mesh->AddIndex(index, false);
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

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
    
    LoadSettings(m_SettingsFileName);
}

MeshGenerator::~MeshGenerator()
{
    SaveSettings(m_SettingsFileName);

    auto scene = SceneManager::Get()->GetActiveScene();
    m_Terrain->RemoveComponent(m_Mesh);

    if(m_Terrain->GetScene() != nullptr) scene->RemoveChild(m_Terrain);
   
    delete m_Mesh;  
    delete m_Terrain;    
}

bool MeshGenerator::DrawImGui()
{
    m_ValueChanged = false;
    if (ImGui::CollapsingHeader("Mesh"), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed)
    {        

        m_ValueChanged |= ImGui::DragFloat("Height modifier", static_cast<float*>(&m_HeightModifier));
        m_ValueChanged |= static_cast<bool>(ImGui::Bezier("easeInExpo", m_HeightCurve));

        m_ValueChanged |= ImGui::Checkbox("Auto generate mesh on change", &m_AutoGenMesh);

        ImGui::DragFloat3("Scale mesh", reinterpret_cast<float*>(&m_Scale), 0.5f);
        m_Terrain->GetTransform()->Scale(m_Scale);

        if (!m_HeightMap.empty())
        {
            if (ImGui::Button("generate mesh") || (m_ValueChanged && m_AutoGenMesh))
            {
                Generate();
            }
        }        

        if(m_HasGeneratedOnec)
        {
            const size_t bufferSize = 256;
            std::vector<char> buffer(bufferSize);
            strncpy_s(buffer.data(), buffer.size(), m_FileName.c_str(), _TRUNCATE);
            bool edited = ImGui::InputText("File name", buffer.data(), buffer.size());
            if (edited)
            {
                m_FileName = std::string(buffer.data());
            }

            if (ImGui::Button("Save to file"))
            {
                m_Mesh->SaveToObj(m_FileName);
            }

            ImGui::Text("Surface Roughness: %.10g", m_SurfaceRoughness);
        }
    }
    return m_ValueChanged;
}

MeshIndexedDrawComponent* MeshGenerator::GenerateMesh(int width, int height, const std::vector<float>& heightmap)
{
    m_FinalHeightMap.clear();

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
    
    const int vertCount = width * height;
    const int quadCount = (width - 1) * (height - 1);

    m_Mesh = new MeshIndexedDrawComponent(vertCount, quadCount * 6);

    XMFLOAT3 normal(0, 1, 0); // Placeholder normal
    std::vector<VertexPosNormCol> tempVertices(vertCount);
    std::vector<uint32_t> tempIndices(quadCount * 6);

    std::mutex mtx;

    // Populate vertices in parallel
    auto vertexWorker = [&](int start, int end) {
        std::for_each(std::execution::par_unseq, tempVertices.begin() + start * width, tempVertices.begin() + end * width,
            [&](VertexPosNormCol& vertex) {
                int index = static_cast<int>( & vertex - tempVertices.data()); // Calculate index based on pointer arithmetic
                int z = index / width;
                int x = index % width;
                const float heightValue = heightmap[index];
                const float curveHeight = ImGui::BezierValue(heightValue, m_HeightCurve);
                const float finalHeight = curveHeight * m_HeightModifier;

                XMFLOAT3 pos(static_cast<float>(x), finalHeight, static_cast<float>(z));
                vertex = VertexPosNormCol(pos, normal, m_Color);

                {
                    std::lock_guard<std::mutex> lock(mtx);
                    m_FinalHeightMap.push_back(finalHeight); // Thread-safe access
                }
            });
        };

    // Populate indices in parallel
    auto indexWorker = [&](int start, int end) {
        std::for_each(std::execution::par_unseq, tempIndices.begin() + start * (width - 1) * 6, tempIndices.begin() + end * (width - 1) * 6,
            [&](uint32_t& index) {
                int idx = static_cast<int>(&index - tempIndices.data()); // Calculate index based on pointer arithmetic
                int quadIdx = idx / 6;
                int x = quadIdx % (width - 1);
                int z = quadIdx / (width - 1);

                const int v0 = z * width + x;
                const int v1 = v0 + 1;
                const int v2 = v0 + width;
                const int v3 = v2 + 1;
                const int indexBase = quadIdx * 6;

                // First triangle
                tempIndices[indexBase] = v0;
                tempIndices[indexBase + 1] = v1;
                tempIndices[indexBase + 2] = v2;

                // Second triangle
                tempIndices[indexBase + 3] = v2;
                tempIndices[indexBase + 4] = v1;
                tempIndices[indexBase + 5] = v3;
            });
        };

    int numThreads = std::thread::hardware_concurrency();
    int rowsPerThread = height / numThreads;
    int startRow = 0;
    std::vector<std::future<void>> futures;
    futures.reserve(numThreads);

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

    //generate normals
    m_Mesh->GenerateNormals();
    
    //SR
    float mean = std::accumulate(m_FinalHeightMap.begin(), m_FinalHeightMap.end(), 0.0f) / m_FinalHeightMap.size();
    float sumOfSquaredDifferences = 0.0f;
    for (float val : m_FinalHeightMap)
    {
        sumOfSquaredDifferences += std::powf(val - mean, 2);
    }    
    float variance = sumOfSquaredDifferences / m_FinalHeightMap.size();
    m_SurfaceRoughness = std::sqrt(variance);

    return m_Mesh;
}

void MeshGenerator::Generate()
{
    m_HasGeneratedOnec = true;
    auto scene = SceneManager::Get()->GetActiveScene();
    
    if (m_Terrain->GetScene() != nullptr)
    {
        scene->RemoveChild(m_Terrain);
    }    

    m_Terrain->RemoveComponent(m_Mesh);
    delete m_Mesh;
    delete m_Terrain;
    
    GenerateMesh(m_MapSize.x, m_MapSize.y, m_HeightMap);
    m_Terrain = new GameObject();
    m_Terrain->AddComponent(m_Mesh);
    scene->AddChild(m_Terrain);
}

void MeshGenerator::LoadSettings(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        Logger::LogError(L"Settings file not open");
        return;
    }        

    file.read(reinterpret_cast<char*>(&m_HeightModifier), sizeof(m_HeightModifier));
    file.read(reinterpret_cast<char*>(&m_HeightCurve), sizeof(m_HeightCurve));
    file.read(reinterpret_cast<char*>(&m_AutoGenMesh), sizeof(m_AutoGenMesh));
    file.read(reinterpret_cast<char*>(&m_Scale), sizeof(m_Scale));

    std::vector<char> buffer(256); // Adjust buffer size if necessary
    file.read(buffer.data(), buffer.size());
    m_FileName = std::string(buffer.data());
}

void MeshGenerator::SaveSettings(const std::string& filename)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        Logger::LogError(L"Settings file not open");
        return;
    }
    file.write(reinterpret_cast<const char*>(&m_HeightModifier), sizeof(m_HeightModifier));
    file.write(reinterpret_cast<const char*>(&m_HeightCurve), sizeof(m_HeightCurve));
    file.write(reinterpret_cast<const char*>(&m_AutoGenMesh), sizeof(m_AutoGenMesh));
    file.write(reinterpret_cast<const char*>(&m_Scale), sizeof(m_Scale));
    file.write(reinterpret_cast<const char*>(&m_FileName), m_FileName.size());
}
